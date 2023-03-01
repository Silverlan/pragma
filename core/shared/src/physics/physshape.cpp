/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/math/surfacematerial.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/physics/base_t.hpp"

pragma::physics::IShape::IShape(IEnvironment &env) : IBase {env} {}
pragma::physics::IShape::~IShape() {}
void pragma::physics::IShape::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IShape>(lua); }

bool pragma::physics::IShape::IsValid() const { return true; }
bool pragma::physics::IShape::IsConvex() const { return false; }
bool pragma::physics::IShape::IsConvexHull() const { return false; }
bool pragma::physics::IShape::IsCompoundShape() const { return false; }
bool pragma::physics::IShape::IsHeightfield() const { return false; }
bool pragma::physics::IShape::IsTriangleShape() const { return false; }

pragma::physics::IConvexShape *pragma::physics::IShape::GetConvexShape() { return nullptr; }
const pragma::physics::IConvexShape *pragma::physics::IShape::GetConvexShape() const { return const_cast<IShape *>(this)->GetConvexShape(); }
pragma::physics::IConvexHullShape *pragma::physics::IShape::GetConvexHullShape() { return nullptr; }
const pragma::physics::IConvexHullShape *pragma::physics::IShape::GetConvexHullShape() const { return const_cast<IShape *>(this)->GetConvexHullShape(); }
pragma::physics::ICompoundShape *pragma::physics::IShape::GetCompoundShape() { return nullptr; }
const pragma::physics::ICompoundShape *pragma::physics::IShape::GetCompoundShape() const { return const_cast<IShape *>(this)->GetCompoundShape(); }
pragma::physics::IHeightfield *pragma::physics::IShape::GetHeightfield() { return nullptr; }
const pragma::physics::IHeightfield *pragma::physics::IShape::GetHeightfield() const { return const_cast<IShape *>(this)->GetHeightfield(); }
pragma::physics::ITriangleShape *pragma::physics::IShape::GetTriangleShape() { return nullptr; }
const pragma::physics::ITriangleShape *pragma::physics::IShape::GetTriangleShape() const { return const_cast<IShape *>(this)->GetTriangleShape(); }

void pragma::physics::IShape::SetSurfaceMaterial(int32_t surfMatIdx)
{
	auto *surfMat = m_physEnv.GetNetworkState().GetGameState()->GetSurfaceMaterial(surfMatIdx);
	if(surfMat == nullptr) {
		m_material = util::WeakHandle<IMaterial> {};
		return;
	}
	SetMaterial(surfMat->GetPhysicsMaterial());
}
int32_t pragma::physics::IShape::GetSurfaceMaterialIndex() const
{
	auto *surfMat = GetSurfaceMaterial();
	return surfMat ? surfMat->GetIndex() : -1;
}
SurfaceMaterial *pragma::physics::IShape::GetSurfaceMaterial() const { return m_material.valid() ? m_material->GetSurfaceMaterial() : nullptr; }
void pragma::physics::IShape::SetMaterial(const IMaterial &mat) { m_material = util::WeakHandle<IMaterial>(std::static_pointer_cast<IMaterial>(const_cast<IMaterial &>(mat).shared_from_this())); }
pragma::physics::IMaterial *pragma::physics::IShape::GetMaterial() const { return m_material.get(); }

void pragma::physics::IShape::SetDensity(float density) { m_density = density; }
float pragma::physics::IShape::GetDensity() const { return m_density; }

/////////////

pragma::physics::IConvexShape::IConvexShape(IEnvironment &env) : IShape {env} {}
pragma::physics::IConvexShape::~IConvexShape() {}
void pragma::physics::IConvexShape::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IConvexShape>(lua); }

bool pragma::physics::IConvexShape::IsConvex() const { return true; }
pragma::physics::IConvexShape *pragma::physics::IConvexShape::GetConvexShape() { return this; }

void pragma::physics::IConvexShape::SetCollisionMesh(CollisionMesh &collisionMesh) { m_collisionMesh = collisionMesh.shared_from_this(); }
void pragma::physics::IConvexShape::SetCollisionMesh() { m_collisionMesh = nullptr; }
const CollisionMesh *pragma::physics::IConvexShape::GetCollisionMesh() const { return const_cast<IConvexShape *>(this)->GetCollisionMesh(); }
CollisionMesh *pragma::physics::IConvexShape::GetCollisionMesh() { return m_collisionMesh.get(); }

/////////////

pragma::physics::ICapsuleShape::ICapsuleShape(IEnvironment &env) : IConvexShape {env} {}
pragma::physics::ICapsuleShape::~ICapsuleShape() {}
void pragma::physics::ICapsuleShape::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<ICapsuleShape>(lua); }

/////////////

pragma::physics::IBoxShape::IBoxShape(IEnvironment &env) : IConvexShape {env} {}
pragma::physics::IBoxShape::~IBoxShape() {}
void pragma::physics::IBoxShape::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IBoxShape>(lua); }

/////////////

pragma::physics::IConvexHullShape::IConvexHullShape(IEnvironment &env) : IConvexShape {env} {}
pragma::physics::IConvexHullShape *pragma::physics::IConvexHullShape::GetConvexHullShape() { return this; }
bool pragma::physics::IConvexHullShape::IsConvexHull() const { return true; }
void pragma::physics::IConvexHullShape::Build()
{
	DoBuild();
	auto *pSurfMat = GetSurfaceMaterial();
	if(pSurfMat == nullptr)
		return;
	ApplySurfaceMaterial(pSurfMat->GetPhysicsMaterial());
}
void pragma::physics::IConvexHullShape::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IConvexHullShape>(lua); }

/////////////

pragma::physics::ICompoundShape::ICompoundShape(IEnvironment &env) : IShape {env} {}
pragma::physics::ICompoundShape::ICompoundShape(IEnvironment &env, pragma::physics::IShape &shape, const Vector3 &origin) : IShape {env} {}
void pragma::physics::ICompoundShape::AddShape(pragma::physics::IShape &shape, const umath::Transform &localPose)
{
	m_shapes.push_back({std::static_pointer_cast<IShape>(shape.shared_from_this()), localPose});
	if(m_shapes.size() == 1) {
		shape.GetAABB(m_min, m_max);
		m_min = localPose * m_min;
		m_max = localPose * m_max;
		return;
	}
	Vector3 min, max;
	shape.GetAABB(min, max);
	uvec::min(&m_min, localPose * min);
	uvec::max(&m_max, localPose * max);
}
void pragma::physics::ICompoundShape::GetAABB(Vector3 &min, Vector3 &max) const
{
	min = m_min;
	max = m_max;
}
void pragma::physics::ICompoundShape::SetMass(float mass) { Con::cwar << "Attempted to set mass of compound shape. This is illegal, change the mass of the individual sub-shapes instead!" << Con::endl; }
float pragma::physics::ICompoundShape::GetMass() const
{
	if(m_shapes.empty())
		return 0.f;
	auto mass = 0.f;
	for(auto &shape : m_shapes)
		mass += shape.shape->GetMass();
	return mass;
}
bool pragma::physics::ICompoundShape::IsCompoundShape() const { return true; }
pragma::physics::ICompoundShape *pragma::physics::ICompoundShape::GetCompoundShape() { return this; }
const std::vector<pragma::physics::ICompoundShape::ShapeInfo> &pragma::physics::ICompoundShape::GetShapes() const { return m_shapes; }
void pragma::physics::ICompoundShape::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<ICompoundShape>(lua); }

/////////////

pragma::physics::IHeightfield::IHeightfield(IEnvironment &env, uint32_t width, uint32_t length, float maxHeight, uint8_t upAxis) : IShape {env}, m_width {width}, m_length {length}, m_maxHeight {maxHeight}, m_upAxis {upAxis} {}
bool pragma::physics::IHeightfield::IsHeightfield() const { return true; }
pragma::physics::IHeightfield *pragma::physics::IHeightfield::GetHeightfield() { return this; }
void pragma::physics::IHeightfield::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IHeightfield>(lua); }

/////////////

pragma::physics::ITriangleShape::ITriangleShape(IEnvironment &env) : IShape {env} {}
bool pragma::physics::ITriangleShape::IsTriangleShape() const { return true; }
pragma::physics::ITriangleShape *pragma::physics::ITriangleShape::GetTriangleShape() { return this; }
void pragma::physics::ITriangleShape::ReserveTriangles(std::size_t count)
{
	m_vertices.reserve(count * 3);
	m_triangles.reserve(count * 3);
	m_faceMaterials.reserve(count);
}
void pragma::physics::ITriangleShape::Build(const std::vector<SurfaceMaterial> *materials)
{
	DoBuild(materials);
	auto *pSurfMat = GetSurfaceMaterial();
	if(pSurfMat == nullptr)
		return;
	ApplySurfaceMaterial(pSurfMat->GetPhysicsMaterial());
}
size_t pragma::physics::ITriangleShape::GetVertexCount() const { return m_vertices.size(); }
Vector3 *pragma::physics::ITriangleShape::GetVertex(size_t idx) { return idx < m_vertices.size() ? &m_vertices.at(idx) : nullptr; }
const Vector3 *pragma::physics::ITriangleShape::GetVertex(size_t idx) const { return const_cast<ITriangleShape *>(this)->GetVertex(idx); }
std::vector<Vector3> &pragma::physics::ITriangleShape::GetVertices() { return m_vertices; }
const std::vector<Vector3> &pragma::physics::ITriangleShape::GetVertices() const { return const_cast<ITriangleShape *>(this)->GetVertices(); }

std::vector<uint32_t> &pragma::physics::ITriangleShape::GetTriangles() { return m_triangles; }
const std::vector<uint32_t> &pragma::physics::ITriangleShape::GetTriangles() const { return const_cast<ITriangleShape *>(this)->GetTriangles(); }
std::vector<int32_t> &pragma::physics::ITriangleShape::GetSurfaceMaterials() { return m_faceMaterials; }
const std::vector<int32_t> &pragma::physics::ITriangleShape::GetSurfaceMaterials() const { return const_cast<ITriangleShape *>(this)->GetSurfaceMaterials(); }
void pragma::physics::ITriangleShape::AddTriangle(const Vector3 &a, const Vector3 &b, const Vector3 &c, const SurfaceMaterial *mat)
{
	assert(!m_bBuilt); // If already built, we'd have to remove our btTriangleIndexVertexMaterialArray, due to its internal structure (It points directly to our vector data).
	m_vertices.push_back(a);
	m_triangles.push_back(static_cast<int>(m_vertices.size() - 1));

	m_vertices.push_back(b);
	m_triangles.push_back(static_cast<int>(m_vertices.size() - 1));

	m_vertices.push_back(c);
	m_triangles.push_back(static_cast<int>(m_vertices.size() - 1));

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
void pragma::physics::ITriangleShape::CalculateLocalInertia(float, Vector3 *localInertia) const { *localInertia = Vector3(0.f, 0.f, 0.f); }
void pragma::physics::ITriangleShape::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<ITriangleShape>(lua); }
