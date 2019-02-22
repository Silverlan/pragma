#include "stdafx_shared.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/math/surfacematerial.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/lua/classes/lphysics.h"
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.h>

DEFINE_BASE_HANDLE(DLLNETWORK,PhysShape,PhysShape);
DEFINE_DERIVED_HANDLE(DLLNETWORK,PhysShape,PhysShape,PhysConvexHullShape,PhysConvexHullShape);
DEFINE_DERIVED_HANDLE(DLLNETWORK,PhysShape,PhysShape,PhysHeightfield,PhysHeightfield);

PhysShape::PhysShape(btCollisionShape *shape,bool bOwns)
	: std::enable_shared_from_this<PhysShape>(),m_shape(nullptr),m_externalShape(nullptr),m_handle(this)
{
	if(bOwns == true)
		m_shape = std::shared_ptr<btCollisionShape>(shape);
	else
		m_externalShape = shape;
}

PhysShape::~PhysShape()
{
	m_handle.Invalidate();
}

luabind::object PhysShape::GetLuaObject(lua_State *l) {return luabind::object(l,LPhysShape(shared_from_this()));}
bool PhysShape::IsConvex() const {return false;}
bool PhysShape::IsConvexHull() const {return false;}
bool PhysShape::IsCompoundShape() const {return false;}
bool PhysShape::IsHeightfield() const {return false;}
bool PhysShape::IsTriangleShape() const {return false;}

void PhysShape::CalculateLocalInertia(float mass,Vector3 *localInertia)
{
	if(m_shape == nullptr)
		return;
	btVector3 btInertia;
	m_shape->calculateLocalInertia(mass,btInertia);
	localInertia->x = static_cast<float>(btInertia.x() /PhysEnv::WORLD_SCALE);
	localInertia->y = static_cast<float>(btInertia.y() /PhysEnv::WORLD_SCALE);
	localInertia->z = static_cast<float>(btInertia.z() /PhysEnv::WORLD_SCALE);
}

void PhysShape::Remove()
{
	delete this;
}

void PhysShape::GetAABB(Vector3 &min,Vector3 &max) const
{
	if(m_shape == nullptr)
	{
		min = {};
		max = {};
		return;
	}
	btTransform t {};
	t.setIdentity();
	btVector3 btMin,btMax;
	m_shape->getAabb(t,btMin,btMax);
	min = uvec::create(btMin /PhysEnv::WORLD_SCALE);
	max = uvec::create(btMax /PhysEnv::WORLD_SCALE);
}

PhysShapeHandle PhysShape::GetHandle() const {return m_handle;}

btCollisionShape *PhysShape::GetShape() {return (m_externalShape != nullptr) ? m_externalShape : m_shape.get();}

//////////////////////////////////

PhysConvexShape::PhysConvexShape(btConvexShape *shape)
	: PhysShape(shape),m_convexShape(shape)
{
}
bool PhysConvexShape::IsConvex() const {return true;}
btConvexShape *PhysConvexShape::GetConvexShape() {return m_convexShape;}
luabind::object PhysConvexShape::GetLuaObject(lua_State *l) {return luabind::object(l,LPhysConvexShape(std::static_pointer_cast<::PhysConvexShape>(shared_from_this())));}

void PhysConvexShape::SetCollisionMesh(CollisionMesh &collisionMesh) {m_collisionMesh = collisionMesh.shared_from_this();}
void PhysConvexShape::SetCollisionMesh() {m_collisionMesh = nullptr;}
const CollisionMesh *PhysConvexShape::GetCollisionMesh() const {return const_cast<PhysConvexShape*>(this)->GetCollisionMesh();}
CollisionMesh *PhysConvexShape::GetCollisionMesh() {return m_collisionMesh.get();}

void PhysConvexShape::SetLocalScaling(const Vector3 &scale)
{
	m_convexShape->setLocalScaling(btVector3(scale.x,scale.y,scale.z));
}

//////////////////////////////////

PhysConvexHullShape::PhysConvexHullShape(btConvexHullShape *shape)
	: PhysConvexShape(shape),m_convexHullShape(shape),m_surfaceMaterial(0)
{}

PhysConvexHullShape::PhysConvexHullShape()
	: PhysConvexHullShape(new btConvexHullShape)
{}
luabind::object PhysConvexHullShape::GetLuaObject(lua_State *l) {return luabind::object(l,LPhysConvexHullShape(std::static_pointer_cast<PhysConvexHullShape>(shared_from_this())));}
bool PhysConvexHullShape::IsConvexHull() const {return true;}
void PhysConvexHullShape::SetSurfaceMaterial(int id) {m_surfaceMaterial = id;}
int PhysConvexHullShape::GetSurfaceMaterial() const {return m_surfaceMaterial;}

void PhysConvexHullShape::AddPoint(const Vector3 &point)
{
	m_convexHullShape->addPoint(btVector3(point.x,point.y,point.z) *PhysEnv::WORLD_SCALE);
}

//////////////////////////////////

PhysHeightfield::PhysHeightfield(uint32_t width,uint32_t length,btScalar maxHeight,uint8_t upAxis)
	: PhysShape(nullptr),m_width(width),m_length(length),m_maxHeight(maxHeight),m_upAxis(upAxis)
{
	const auto bFlipQuadEdges = false;
	static auto height = 50.f; // TODO
	const auto heightScale = 1.f;
	const auto minHeight = -maxHeight;//-50.f;
	const auto gridSpacing = 10.f *PhysEnv::WORLD_SCALE;

	m_heightFieldData.resize(width *length,height *PhysEnv::WORLD_SCALE);
	auto shape = std::shared_ptr<btHeightfieldTerrainShape>(new btHeightfieldTerrainShape(width,length,m_heightFieldData.data(),heightScale,minHeight *PhysEnv::WORLD_SCALE,maxHeight *PhysEnv::WORLD_SCALE,upAxis,PHY_ScalarType::PHY_FLOAT,bFlipQuadEdges));

	shape->setLocalScaling({gridSpacing,1.0,gridSpacing});
	m_shape = shape;
	m_heightField = shape;
}
luabind::object PhysHeightfield::GetLuaObject(lua_State *l) {return luabind::object(l,LPhysHeightfield(std::static_pointer_cast<PhysHeightfield>(shared_from_this())));}
uint32_t PhysHeightfield::GetWidth() const {return m_width;}
uint32_t PhysHeightfield::GetLength() const {return m_length;}
btScalar PhysHeightfield::GetMaxHeight() const {return m_maxHeight;}
uint8_t PhysHeightfield::GetUpAxis() const {return m_upAxis;}
float PhysHeightfield::GetHeight(uint32_t x,uint32_t y) const
{
	auto idx = y *GetWidth() +x;
	if(idx >= m_heightFieldData.size())
		return 0.f;
	return m_heightFieldData.at(idx);
}
void PhysHeightfield::SetHeight(uint32_t x,uint32_t y,float height)
{
	auto idx = y *GetWidth() +x;
	if(idx >= m_heightFieldData.size())
		return;
	m_heightFieldData.at(idx) = height *PhysEnv::WORLD_SCALE;
}
bool PhysHeightfield::IsHeightfield() const {return true;}

//////////////////////////////////

PhysTriangleShape::PhysTriangleShape()
	: PhysShape(nullptr),m_infoMap(nullptr),//m_triangleMesh(new btTriangleMesh),
	m_bBuilt(false)
{}

PhysTriangleShape::~PhysTriangleShape()
{
	if(m_infoMap != nullptr)
		delete m_infoMap;
}

luabind::object PhysTriangleShape::GetLuaObject(lua_State *l) {return luabind::object(l,LPhysTriangleShape(std::static_pointer_cast<::PhysTriangleShape>(shared_from_this())));}

void PhysTriangleShape::AddTriangle(const Vector3 &a,const Vector3 &b,const Vector3 &c,const SurfaceMaterial *mat)
{
	assert(!m_bBuilt); // If already built, we'd have to remove our btTriangleIndexVertexMaterialArray, due to its internal structure (It points directly to our vector data). 
	m_vertices.push_back(btVector3(a.x,a.y,a.z) *PhysEnv::WORLD_SCALE);
	m_triangles.push_back(static_cast<int>(m_vertices.size() -1));

	m_vertices.push_back(btVector3(b.x,b.y,b.z) *PhysEnv::WORLD_SCALE);
	m_triangles.push_back(static_cast<int>(m_vertices.size() -1));

	m_vertices.push_back(btVector3(c.x,c.y,c.z) *PhysEnv::WORLD_SCALE);
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

void PhysTriangleShape::ReserveTriangles(std::size_t count)
{
	m_vertices.reserve(count *3);
	m_triangles.reserve(count *3);
	m_faceMaterials.reserve(count);
}
btTriangleIndexVertexArray *PhysTriangleShape::GetBtIndexVertexArray() {return m_triangleArray.get();}

void PhysTriangleShape::Build(const std::vector<SurfaceMaterial> *materials)
{
	m_shape = nullptr;
	if(m_triangles.empty() || m_vertices.empty())
		return;
	btBvhTriangleMeshShape *shape = nullptr;
	if(materials == nullptr)
	{
		m_triangleArray = std::make_unique<btTriangleIndexVertexArray>(
			static_cast<int>(m_triangles.size() /3),&m_triangles[0],static_cast<int>(sizeof(int) *3),
			static_cast<int>(m_vertices.size()),&m_vertices[0][0],static_cast<int>(sizeof(btVector3))
		);
		shape = new btBvhTriangleMeshShape(m_triangleArray.get(),false);
	}
	else
	{
		m_triangleArray = std::make_unique<btTriangleIndexVertexMaterialArray>(
			static_cast<int>(m_triangles.size() /3),&m_triangles[0],static_cast<int>(sizeof(int) *3),
			static_cast<int>(m_vertices.size()),&m_vertices[0][0],static_cast<int>(sizeof(btVector3)),
			static_cast<int>(materials->size()),(unsigned char*)(&(*materials)[0]),static_cast<int>(sizeof(SurfaceMaterial)),
			&m_faceMaterials[0],static_cast<int>(sizeof(int))
		);
		shape = new btMultimaterialTriangleMeshShape(m_triangleArray.get(),false);
	}
	m_bBuilt = true;
	m_shape = std::shared_ptr<btCollisionShape>(shape);
	m_infoMap = new btTriangleInfoMap();
	GenerateInternalEdgeInfo();
}

void PhysTriangleShape::GenerateInternalEdgeInfo()
{
	static_cast<btBvhTriangleMeshShape*>(m_shape.get())->setTriangleInfoMap(nullptr);
	btGenerateInternalEdgeInfo(static_cast<btBvhTriangleMeshShape*>(m_shape.get()),m_infoMap);
}
std::vector<btVector3> &PhysTriangleShape::GetVertices() {return m_vertices;}
const std::vector<btVector3> &PhysTriangleShape::GetVertices() const {return m_vertices;}
std::vector<int32_t> &PhysTriangleShape::GetTriangles() {return m_triangles;}
const std::vector<int32_t> &PhysTriangleShape::GetTriangles() const {return const_cast<PhysTriangleShape*>(this)->GetTriangles();}
std::vector<int32_t> &PhysTriangleShape::GetSurfaceMaterials() {return m_faceMaterials;}
const std::vector<int32_t> &PhysTriangleShape::GetSurfaceMaterials() const {return const_cast<PhysTriangleShape*>(this)->GetSurfaceMaterials();}

void PhysTriangleShape::CalculateLocalInertia(float,Vector3 *localInertia)
{
	*localInertia = Vector3(0.f,0.f,0.f);
}
bool PhysTriangleShape::IsTriangleShape() const {return true;}

//////////////////////////////////

PhysCompoundShape::PhysCompoundShape(btCompoundShape *sh)
	: PhysShape(sh)
{}
PhysCompoundShape::PhysCompoundShape()
	: PhysShape(new btCompoundShape)
{}
PhysCompoundShape::PhysCompoundShape(std::shared_ptr<PhysShape> &shape)
	: PhysShape(new btCompoundShape)
{
	AddShape(shape);
}
PhysCompoundShape::PhysCompoundShape(std::vector<std::shared_ptr<PhysShape>> &shapes)
	: PhysShape(new btCompoundShape)
{
	for(auto it=shapes.begin();it!=shapes.end();++it)
		AddShape(*it);
}
void PhysCompoundShape::AddShape(std::shared_ptr<PhysShape> &shape,const Vector3 &origin,const Quat &rot)
{
	auto *btShape = shape->GetShape();
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(origin.x,origin.y,origin.z) *PhysEnv::WORLD_SCALE);
	t.setRotation(uquat::create_bt(rot));
	static_cast<btCompoundShape*>(m_shape.get())->addChildShape(t,btShape);
	m_shapes.push_back(shape);
}
bool PhysCompoundShape::IsCompoundShape() const {return true;}
