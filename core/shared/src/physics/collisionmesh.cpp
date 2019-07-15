#include "stdafx_shared.h"
#include "pragma/physics/collisionmesh.h"
#include <mathutil/uvec.h>
#include <pragma/math/intersection.h>
#include "pragma/physics/shape.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/model/modelmesh.h"

std::shared_ptr<CollisionMesh> CollisionMesh::Create(Game *game) {return std::shared_ptr<CollisionMesh>(new CollisionMesh(game));}
std::shared_ptr<CollisionMesh> CollisionMesh::Create(const CollisionMesh &other) {return std::shared_ptr<CollisionMesh>(new CollisionMesh(other));}
CollisionMesh::SoftBodyInfo::SoftBodyInfo()
{
	info = std::make_shared<PhysSoftBodyInfo>();
}
CollisionMesh::CollisionMesh(Game *game)
	: std::enable_shared_from_this<CollisionMesh>(),m_game(game)
{}
CollisionMesh::CollisionMesh(const CollisionMesh &other)
{
	m_game = other.m_game;
	m_vertices = other.m_vertices;
	m_triangles = other.m_triangles;
	m_surfaceMaterials = other.m_surfaceMaterials;
	m_min = other.m_min;
	m_max = other.m_max;
	m_origin = other.m_origin;
	m_shape = other.m_shape;
	m_bConvex = other.m_bConvex;
	m_boneID = other.m_boneID;
	m_surfaceMaterialId = other.m_surfaceMaterialId;
	m_centerOfMass = other.m_centerOfMass;
	m_volume = other.m_volume;
	m_softBodyInfo = (m_softBodyInfo != nullptr) ? std::make_shared<SoftBodyInfo>(*other.m_softBodyInfo) : nullptr;
}

int CollisionMesh::GetSurfaceMaterial() const {return m_surfaceMaterialId;}
void CollisionMesh::SetSurfaceMaterial(int id) {m_surfaceMaterialId = id;}
void CollisionMesh::SetSurfaceMaterial(const std::string &surfMat)
{
	m_surfaceMaterialId = 0;
	auto *mat = m_game->GetSurfaceMaterial(surfMat);
	if(mat == nullptr)
		return;
	SetSurfaceMaterial(static_cast<int32_t>(mat->GetIndex()));
}
std::vector<int> &CollisionMesh::GetSurfaceMaterials() {return m_surfaceMaterials;}
void CollisionMesh::SetConvex(bool bConvex) {m_bConvex = bConvex;}
bool CollisionMesh::IsConvex() const {return m_bConvex;}
void CollisionMesh::AddVertex(const Vector3 &v)
{
	if(m_vertices.size() == m_vertices.capacity())
		m_vertices.reserve(static_cast<uint32_t>(m_vertices.size() *1.5));
	m_vertices.push_back(v);
}
void CollisionMesh::Rotate(const Quat &rot)
{
	for(auto &v : m_vertices)
		uvec::rotate(&v,rot);
	uvec::rotate(&m_origin,rot);
	uvec::rotate(&m_min,rot);
	uvec::rotate(&m_max,rot);
}
void CollisionMesh::Translate(const Vector3 &t)
{
	for(auto &v : m_vertices)
		v += t;
	m_origin += t;
	m_min += t;
	m_max += t;
}
std::shared_ptr<pragma::physics::IShape> CollisionMesh::CreateShape(const Vector3 &scale) const
{
	auto *physEnv = m_game->GetPhysicsEnvironment();
	if(IsSoftBody() || physEnv == nullptr)
		return nullptr;
	auto &materials = m_game->GetSurfaceMaterials();
	auto bConvex = IsConvex();
	std::shared_ptr<pragma::physics::IShape> shape = nullptr;
	auto bScale = (scale != Vector3{1.f,1.f,1.f}) ? true : false;

	pragma::physics::IMaterial *mat = nullptr;
	if(materials.empty())
		mat = &physEnv->GetGenericMaterial();
	else
		mat = &materials.front().GetPhysicsMaterial();
	if(bConvex == true)
	{
		shape = physEnv->CreateConvexHullShape(*mat);
		if(shape == nullptr)
			return nullptr;
		auto *ptrShape = shape->GetConvexHullShape();
		ptrShape->SetCollisionMesh(*const_cast<CollisionMesh*>(this));
		ptrShape->SetSurfaceMaterial(GetSurfaceMaterial());
		ptrShape->SetLocalScaling(Vector3(1.f,1.f,1.f));
		ptrShape->ReservePoints(m_vertices.size());
		for(unsigned int i=0;i<m_vertices.size();i++)
		{
			auto &v = m_vertices[i];
			if(bScale == false)
				ptrShape->AddPoint(v);
			else
				ptrShape->AddPoint(v *scale);
		}
		ptrShape->ReserveTriangles(m_triangles.size() /3);
		for(auto i=decltype(m_triangles.size()){0u};i<m_triangles.size();i+=3)
			ptrShape->AddTriangle(m_triangles.at(i),m_triangles.at(i +1),m_triangles.at(i +2));
		ptrShape->Build();
	}
	else
	{
		shape = physEnv->CreateTriangleShape(*mat);
		if(shape == nullptr)
			return nullptr;
		auto *ptrShape = shape->GetTriangleShape();
		auto numMats = m_surfaceMaterials.size();
		assert((m_vertices.size() %3) == 0);
		ptrShape->ReserveTriangles(m_vertices.size() /3);
		for(UInt i=0;i<m_vertices.size();i+=3)
		{
			auto triId = i /3;
			const SurfaceMaterial *mat = nullptr;
			auto matId = 0;
			if(triId < numMats)
				matId = m_surfaceMaterials[triId];
			mat = &materials[matId];
			auto &a = m_vertices[i];
			auto &b = m_vertices[i +1];
			auto &c = m_vertices[i +2];
			if(bScale == false)
				ptrShape->AddTriangle(a,b,c,mat);
			else
				ptrShape->AddTriangle(a *scale,b *scale,c *scale,mat);
		}
		ptrShape->Build(&materials);
	}
	return shape;
}
void CollisionMesh::UpdateShape()
{
	m_shape = nullptr;
	if(m_vertices.empty() == true)
		return;
	m_shape = CreateShape();
}
void CollisionMesh::SetBoneParent(int boneID) {m_boneID = boneID;}
int CollisionMesh::GetBoneParent() {return m_boneID;}
void CollisionMesh::SetOrigin(const Vector3 &origin) {m_origin = origin;}
const Vector3 &CollisionMesh::GetOrigin() const {return const_cast<CollisionMesh*>(this)->GetOrigin();}
Vector3 &CollisionMesh::GetOrigin() {return m_origin;}
std::vector<Vector3> &CollisionMesh::GetVertices() {return m_vertices;}
void CollisionMesh::CalculateBounds()
{
	auto numVerts = m_vertices.size();
	if(numVerts == 0)
		return;
	m_min = m_vertices[0];
	m_max = m_vertices[0];
	for(size_t i=1;i<numVerts;i++)
	{
		uvec::min(&m_min,m_vertices[i]);
		uvec::max(&m_max,m_vertices[i]);
	}
}
void CollisionMesh::Update(ModelUpdateFlags flags)
{
	if((flags &ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None)
		CalculateBounds();
	if((flags &ModelUpdateFlags::UpdateCollisionShapes) != ModelUpdateFlags::None)
		UpdateShape(); // TODO: Surface materials?
}
void CollisionMesh::Centralize()
{
	Vector3 center {};
	for(auto &v : m_vertices)
		center += v;
	if(m_vertices.empty() == false)
		center /= static_cast<float>(m_vertices.size());
	SetOrigin(GetOrigin() +-center);
	for(auto &v : m_vertices)
		v -= center;
}
void CollisionMesh::GetAABB(Vector3 *min,Vector3 *max)
{
	*min = m_min;
	*max = m_max;
}
void CollisionMesh::SetAABB(Vector3 &min,Vector3 &max)
{
	m_min = min;
	m_max = max;
}
std::shared_ptr<pragma::physics::IShape> CollisionMesh::GetShape() {return m_shape;}
bool CollisionMesh::IntersectAABB(Vector3 *min,Vector3 *max)
{
	if(!Intersection::AABBAABB(m_min,m_max,*min,*max))
		return false;
	for(int i=0;i<m_vertices.size();i+=3)
	{
		if(Intersection::AABBTriangle(m_min,m_max,m_vertices[i],m_vertices[i +1],m_vertices[i +2]))
			return true;
	}
	// TODO: Check if bounds are WITHIN mesh (Before checking triangles?)
	return false;
}

const std::vector<uint16_t> &CollisionMesh::GetTriangles() const {return const_cast<CollisionMesh*>(this)->GetTriangles();}
std::vector<uint16_t> &CollisionMesh::GetTriangles() {return m_triangles;}
void CollisionMesh::CalculateVolumeAndCom()
{
	m_volume = Geometry::calc_volume_of_polyhedron(m_vertices,m_triangles,&m_centerOfMass);
}
const Vector3 &CollisionMesh::GetCenterOfMass() const {return m_centerOfMass;}
void CollisionMesh::SetCenterOfMass(const Vector3 &com) {m_centerOfMass = com;}
double CollisionMesh::GetVolume() const {return m_volume;}
void CollisionMesh::SetVolume(double vol) {m_volume = vol;}

void CollisionMesh::SetSoftBody(bool b)
{
	if(b == false)
	{
		m_softBodyInfo = nullptr;
		return;
	}
	m_softBodyInfo = std::make_shared<SoftBodyInfo>();
}
bool CollisionMesh::IsSoftBody() const {return m_softBodyInfo != nullptr;}
ModelSubMesh *CollisionMesh::GetSoftBodyMesh() const
{
	if(m_softBodyInfo == nullptr || m_softBodyInfo->subMesh.expired())
		return nullptr;
	return m_softBodyInfo->subMesh.lock().get();
}
void CollisionMesh::SetSoftBodyMesh(ModelSubMesh &mesh)
{
	if(m_softBodyInfo == nullptr)
		return;
	m_softBodyInfo->subMesh = mesh.shared_from_this();
}
const std::vector<uint32_t> *CollisionMesh::GetSoftBodyTriangles() const {return const_cast<CollisionMesh*>(this)->GetSoftBodyTriangles();}
std::vector<uint32_t> *CollisionMesh::GetSoftBodyTriangles() {return (m_softBodyInfo != nullptr) ? &m_softBodyInfo->triangles : nullptr;}
PhysSoftBodyInfo *CollisionMesh::GetSoftBodyInfo() const {return (m_softBodyInfo != nullptr) ? m_softBodyInfo->info.get() : nullptr;}
bool CollisionMesh::AddSoftBodyAnchor(uint16_t vertIdx,uint32_t boneIdx,SoftBodyAnchor::Flags flags,float influence,uint32_t *anchorIdx)
{
	if(m_softBodyInfo == nullptr)
		return false;
	auto &anchors = m_softBodyInfo->anchors;
	if(anchors.size() == anchors.capacity())
		anchors.reserve(anchors.size() +10);
	anchors.push_back({});
	auto &anchor = anchors.back();
	anchor.vertexIndex = vertIdx;
	anchor.boneId = boneIdx;
	anchor.flags = flags;
	anchor.influence = influence;
	if(anchorIdx != nullptr)
		*anchorIdx = anchors.size() -1;
	return true;
}
void CollisionMesh::RemoveSoftBodyAnchor(uint32_t anchorIdx)
{
	if(m_softBodyInfo == nullptr || anchorIdx >= m_softBodyInfo->anchors.size())
		return;
	m_softBodyInfo->anchors.erase(m_softBodyInfo->anchors.begin() +anchorIdx);
}
void CollisionMesh::ClearSoftBodyAnchors()
{
	if(m_softBodyInfo == nullptr)
		return;
	m_softBodyInfo->anchors.clear();
}
const std::vector<CollisionMesh::SoftBodyAnchor> *CollisionMesh::GetSoftBodyAnchors() const {return const_cast<CollisionMesh*>(this)->GetSoftBodyAnchors();}
std::vector<CollisionMesh::SoftBodyAnchor> *CollisionMesh::GetSoftBodyAnchors()
{
	if(m_softBodyInfo == nullptr)
		return nullptr;
	return &m_softBodyInfo->anchors;
}
