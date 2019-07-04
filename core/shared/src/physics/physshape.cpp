#include "stdafx_shared.h"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/math/surfacematerial.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/networkstate/networkstate.h"
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.h>

pragma::physics::IShape::IShape(IEnvironment &env)
	: IBase{env}
{}
pragma::physics::IShape::~IShape()
{}

bool pragma::physics::IShape::IsConvex() const {return false;}
bool pragma::physics::IShape::IsConvexHull() const {return false;}
bool pragma::physics::IShape::IsCompoundShape() const {return false;}
bool pragma::physics::IShape::IsHeightfield() const {return false;}
bool pragma::physics::IShape::IsTriangleShape() const {return false;}

pragma::physics::IConvexShape *pragma::physics::IShape::GetConvexShape() {return nullptr;}
const pragma::physics::IConvexShape *pragma::physics::IShape::GetConvexShape() const {return const_cast<IShape*>(this)->GetConvexShape();}
pragma::physics::IConvexHullShape *pragma::physics::IShape::GetConvexHullShape() {return nullptr;}
const pragma::physics::IConvexHullShape *pragma::physics::IShape::GetConvexHullShape() const {return const_cast<IShape*>(this)->GetConvexHullShape();}
pragma::physics::ICompoundShape *pragma::physics::IShape::GetCompoundShape() {return nullptr;}
const pragma::physics::ICompoundShape *pragma::physics::IShape::GetCompoundShape() const {return const_cast<IShape*>(this)->GetCompoundShape();}
pragma::physics::IHeightfield *pragma::physics::IShape::GetHeightfield() {return nullptr;}
const pragma::physics::IHeightfield *pragma::physics::IShape::GetHeightfield() const {return const_cast<IShape*>(this)->GetHeightfield();}
pragma::physics::ITriangleShape *pragma::physics::IShape::GetTriangleShape() {return nullptr;}
const pragma::physics::ITriangleShape *pragma::physics::IShape::GetTriangleShape() const {return const_cast<IShape*>(this)->GetTriangleShape();}

void pragma::physics::IShape::SetSurfaceMaterial(int32_t surfMatIdx) {m_surfaceMaterialIdx = surfMatIdx;}
int32_t pragma::physics::IShape::GetSurfaceMaterialIndex() const {return m_surfaceMaterialIdx;}
SurfaceMaterial *pragma::physics::IShape::GetSurfaceMaterial()
{
	if(m_surfaceMaterialIdx < 0)
		return nullptr;
	return m_physEnv.GetNetworkState().GetGameState()->GetSurfaceMaterial(m_surfaceMaterialIdx);
}

void pragma::physics::IShape::SetDensity(float density) {m_density = density;}
float pragma::physics::IShape::GetDensity() const {return m_density;}

/////////////

pragma::physics::IConvexShape::IConvexShape(IEnvironment &env)
	: IShape{env}
{}
pragma::physics::IConvexShape::~IConvexShape()
{}

bool pragma::physics::IConvexShape::IsConvex() const {return true;}
pragma::physics::IConvexShape *pragma::physics::IConvexShape::GetConvexShape() {return this;}

void pragma::physics::IConvexShape::SetCollisionMesh(CollisionMesh &collisionMesh) {m_collisionMesh = collisionMesh.shared_from_this();}
void pragma::physics::IConvexShape::SetCollisionMesh() {m_collisionMesh = nullptr;}
const CollisionMesh *pragma::physics::IConvexShape::GetCollisionMesh() const {return const_cast<IConvexShape*>(this)->GetCollisionMesh();}
CollisionMesh *pragma::physics::IConvexShape::GetCollisionMesh() {return m_collisionMesh.get();}

/////////////

pragma::physics::ICapsuleShape::ICapsuleShape(IEnvironment &env)
	: IConvexShape{env}
{}
pragma::physics::ICapsuleShape::~ICapsuleShape()
{}

/////////////

pragma::physics::IBoxShape::IBoxShape(IEnvironment &env)
	: IConvexShape{env}
{}
pragma::physics::IBoxShape::~IBoxShape()
{}

/////////////

pragma::physics::IConvexHullShape::IConvexHullShape(IEnvironment &env)
	: IConvexShape{env}
{}
pragma::physics::IConvexHullShape *pragma::physics::IConvexHullShape::GetConvexHullShape() {return this;}
bool pragma::physics::IConvexHullShape::IsConvexHull() const {return true;}

/////////////

pragma::physics::ICompoundShape::ICompoundShape(IEnvironment &env)
	: IShape{env}
{}
pragma::physics::ICompoundShape::ICompoundShape(IEnvironment &env,pragma::physics::IShape &shape)
	: IShape{env}
{
	m_shapes.push_back(std::static_pointer_cast<IShape>(shape.shared_from_this()));
}
pragma::physics::ICompoundShape::ICompoundShape(IEnvironment &env,const std::vector<pragma::physics::IShape*> &shapes)
	: IShape{env}
{
	m_shapes.reserve(shapes.size());
	for(auto *pShape : shapes)
		m_shapes.push_back(std::static_pointer_cast<IShape>(pShape->shared_from_this()));
}
bool pragma::physics::ICompoundShape::IsCompoundShape() const {return true;}
pragma::physics::ICompoundShape *pragma::physics::ICompoundShape::GetCompoundShape() {return this;}

/////////////

pragma::physics::IHeightfield::IHeightfield(IEnvironment &env,uint32_t width,uint32_t length,float maxHeight,uint8_t upAxis)
	: IShape{env},m_width{width},m_length{length},m_maxHeight{maxHeight},m_upAxis{upAxis}
{}
bool pragma::physics::IHeightfield::IsHeightfield() const {return true;}
pragma::physics::IHeightfield *pragma::physics::IHeightfield::GetHeightfield() {return this;}

/////////////

pragma::physics::ITriangleShape::ITriangleShape(IEnvironment &env)
	: IShape{env}
{}
bool pragma::physics::ITriangleShape::IsTriangleShape() const {return true;}
pragma::physics::ITriangleShape *pragma::physics::ITriangleShape::GetTriangleShape() {return this;}
size_t pragma::physics::ITriangleShape::GetVertexCount() const {return m_vertices.size();}
Vector3 *pragma::physics::ITriangleShape::GetVertex(size_t idx) {return idx < m_vertices.size() ? &m_vertices.at(idx) : nullptr;}
const Vector3 *pragma::physics::ITriangleShape::GetVertex(size_t idx) const {return const_cast<ITriangleShape*>(this)->GetVertex(idx);}
std::vector<Vector3> &pragma::physics::ITriangleShape::GetVertices() {return m_vertices;}
const std::vector<Vector3> &pragma::physics::ITriangleShape::GetVertices() const {return const_cast<ITriangleShape*>(this)->GetVertices();}

std::vector<int32_t> &pragma::physics::ITriangleShape::GetTriangles() {return m_triangles;}
const std::vector<int32_t> &pragma::physics::ITriangleShape::GetTriangles() const {return const_cast<ITriangleShape*>(this)->GetTriangles();}
std::vector<int32_t> &pragma::physics::ITriangleShape::GetSurfaceMaterials() {return m_faceMaterials;}
const std::vector<int32_t> &pragma::physics::ITriangleShape::GetSurfaceMaterials() const {return const_cast<ITriangleShape*>(this)->GetSurfaceMaterials();}
void pragma::physics::ITriangleShape::AddTriangle(const Vector3 &a,const Vector3 &b,const Vector3 &c,const SurfaceMaterial *mat)
{
	assert(!m_bBuilt); // If already built, we'd have to remove our btTriangleIndexVertexMaterialArray, due to its internal structure (It points directly to our vector data). 
	m_vertices.push_back(a);
	m_triangles.push_back(static_cast<int>(m_vertices.size() -1));

	m_vertices.push_back(b);
	m_triangles.push_back(static_cast<int>(m_vertices.size() -1));

	m_vertices.push_back(c);
	m_triangles.push_back(static_cast<int>(m_vertices.size() -1));

	if(mat != nullptr)
		m_faceMaterials.push_back(static_cast<int>(mat->GetIndex()));
	else
		m_faceMaterials.push_back(0);
	/*m_triangleMesh->addTriangle(
	btVector3(a.x,a.y,a.z),
	btVector3(b.x,b.y,b.z),
	btVector3(c.x,c.y,c.z),
	false
	);*/
}
void pragma::physics::ITriangleShape::CalculateLocalInertia(float,Vector3 *localInertia) const
{
	*localInertia = Vector3(0.f,0.f,0.f);
}
