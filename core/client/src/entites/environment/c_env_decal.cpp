#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_decal.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/physics/raytraces.h>
#include <pragma/entities/environment/env_decal.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/math/intersection.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_decal,CEnvDecal);

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

class DecalProjector
{
public:
	DecalProjector(const Vector3 &pos,const Quat &rot,float size);
	const Vector3 &GetPos() const;
	const Quat &GetRotation() const;
	const physics::Transform &GetPose() const;
	float GetSize() const;

	void GenerateDecalMesh(CModelSubMesh &mesh,const physics::Transform &pose,std::vector<Vertex> &outVerts,std::vector<uint16_t> &outTris);
	void DebugDraw(float duration) const;
private:
	struct VertexInfo
	{
		// Position in projector space
		Vector2 position = {};
		uint32_t originalMeshVertexIndex = std::numeric_limits<uint32_t>::max();
		std::optional<Vector2> barycentricCoordinates = {};
	};
	void GetOrthogonalBasis(Vector3 &forward,Vector3 &right,Vector3 &up) const;
	const std::array<Vector3,8> &GetProjectorCubePoints() const;
	std::pair<Vector3,Vector3> GetProjectorCubeBounds() const;
	std::vector<VertexInfo> CropTriangleVertsByLine(
		const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,
		const std::vector<VertexInfo> &verts,const Vector2 &lineStart,const Vector2 &lineEnd
	);

	physics::Transform m_pose = {};
	float m_size = 0.f;
};

DecalProjector::DecalProjector(const Vector3 &pos,const Quat &rot,float size)
	: m_pose{pos,rot},m_size{size}
{}
const Vector3 &DecalProjector::GetPos() const {return m_pose.GetOrigin();}
const Quat &DecalProjector::GetRotation() const {return m_pose.GetRotation();}
const physics::Transform &DecalProjector::GetPose() const {return m_pose;}
float DecalProjector::GetSize() const {return m_size;}
void DecalProjector::GetOrthogonalBasis(Vector3 &forward,Vector3 &right,Vector3 &up) const
{
	auto projectorSpaceRot = uquat::identity();
	forward = uquat::forward(projectorSpaceRot);
	right = uquat::right(projectorSpaceRot);
	up = uquat::up(projectorSpaceRot);
}
const std::array<Vector3,8> &DecalProjector::GetProjectorCubePoints() const
{
	static const std::array<Vector3,8> g_projectorCubePoints = {
		-uvec::FORWARD -uvec::RIGHT -uvec::UP,
		+uvec::FORWARD -uvec::RIGHT -uvec::UP,
		-uvec::FORWARD +uvec::RIGHT -uvec::UP,
		+uvec::FORWARD +uvec::RIGHT -uvec::UP,
		-uvec::FORWARD -uvec::RIGHT +uvec::UP,
		+uvec::FORWARD -uvec::RIGHT +uvec::UP,
		-uvec::FORWARD +uvec::RIGHT +uvec::UP,
		+uvec::FORWARD +uvec::RIGHT +uvec::UP
	};
	return g_projectorCubePoints;
}
std::pair<Vector3,Vector3> DecalProjector::GetProjectorCubeBounds() const
{
	auto prismSize = GetSize();
	return {
		(-uvec::FORWARD +uvec::RIGHT -uvec::UP) *prismSize,
		(+uvec::FORWARD -uvec::RIGHT +uvec::UP) *prismSize
	};
}
std::vector<DecalProjector::VertexInfo> DecalProjector::CropTriangleVertsByLine(
	const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,
	const std::vector<VertexInfo> &verts,const Vector2 &lineStart,const Vector2 &lineEnd
)
{
	std::vector<bool> pointsOutOfBounds {};
	pointsOutOfBounds.resize(verts.size());
	for(auto i=decltype(verts.size()){0u};i<verts.size();++i)
	{
		auto &vInfo = verts.at(i);
		auto side = Geometry::get_side_of_point_to_line(lineStart,lineEnd,vInfo.position);
		pointsOutOfBounds.at(i) = (side == Geometry::LineSide::Right);
	}

	std::vector<VertexInfo> newVerts {};
	for(auto i=decltype(verts.size()){0u};i<verts.size();++i)
	{
		auto v0Idx = i;
		auto v1Idx = (i < (verts.size() -1)) ? (i +1) : 0;

		// If the current point is in bounds, keep it
		if(pointsOutOfBounds.at(v0Idx) == false)
			newVerts.push_back(verts.at(v0Idx));
		
		// Check if the line to the next vertex crosses the bounds
		if(
			(pointsOutOfBounds.at(v0Idx) == true && pointsOutOfBounds.at(v1Idx) == false) ||
			(pointsOutOfBounds.at(v0Idx) == false && pointsOutOfBounds.at(v1Idx) == true)
		)
		{
			auto intersectionPos = Intersection::LineLine(lineStart,lineEnd,verts.at(v0Idx).position,verts.at(v1Idx).position);
			if(intersectionPos.has_value())
			{
				float u,v;
				if(Geometry::calc_barycentric_coordinates(v0,v1,v2,Vector3{intersectionPos->x,intersectionPos->y,0.f},u,v))
					newVerts.push_back({*intersectionPos,std::numeric_limits<uint32_t>::max(),Vector2{u,v}});
			}
		}
	}
	return newVerts;
}
void DecalProjector::GenerateDecalMesh(CModelSubMesh &mesh,const physics::Transform &pose,std::vector<Vertex> &outVerts,std::vector<uint16_t> &outTris)
{
	if(mesh.GetGeometryType() != ModelSubMesh::GeometryType::Triangles)
		return;
	auto bounds = GetProjectorCubeBounds();

	auto effectivePose = GetPose().GetInverse() *pose.GetInverse();
	auto &verts = mesh.GetVertices();
	auto &tris = mesh.GetTriangles();

	struct TriangleInfo
	{
		std::array<uint32_t,3> originalMeshVertexIndex = {};
		std::array<Vector3,3> vertices = {};
	};
	std::vector<TriangleInfo> intersectingTris {};
	intersectingTris.reserve(tris.size());
	// Cull triangles that are outside of the projector bounds
	for(auto i=decltype(tris.size()){0u};i<tris.size();i+=3)
	{
		std::array<uint32_t,3> indices = {tris.at(i),tris.at(i +1),tris.at(i +2)};
		auto &v0 = verts.at(indices.at(0));
		auto &v1 = verts.at(indices.at(1));
		auto &v2 = verts.at(indices.at(2));

		auto p0 = effectivePose *v0.position;
		auto p1 = effectivePose *v1.position;
		auto p2 = effectivePose *v2.position;

		if(Intersection::AABBTriangle(bounds.first,bounds.second,p0,p1,p2) == false)
			continue;
		intersectingTris.push_back({
			indices,
			std::array<Vector3,3>{p0,p1,p2}
		});
	}

	Vector3 forward,right,up;
	GetOrthogonalBasis(forward,right,up);
	auto &n = forward;
	auto d = 0.f;
	std::pair<Vector3,Vector3> projectorAABBBounds = {
		uvec::project_to_plane(bounds.first,n,d),
		uvec::project_to_plane(bounds.second,n,d)
	};
	std::array<Vector2,4> projectorAABBPoints = {
		Vector2{projectorAABBBounds.first.x,projectorAABBBounds.first.y},
		Vector2{projectorAABBBounds.second.x,projectorAABBBounds.first.y},
		Vector2{projectorAABBBounds.second.x,projectorAABBBounds.second.y},
		Vector2{projectorAABBBounds.first.x,projectorAABBBounds.second.y}
	};

	// TODO
	outVerts.reserve(verts.size());
	outTris.reserve(tris.size());
	for(auto triInfo : intersectingTris)
	{
		// Original vertex positions
		auto &p0 = triInfo.vertices.at(0);
		auto &p1 = triInfo.vertices.at(1);
		auto &p2 = triInfo.vertices.at(2);

		std::array<Vector3,3> vertsPs = {
			uvec::project_to_plane(triInfo.vertices.at(0),n,d),
			uvec::project_to_plane(triInfo.vertices.at(1),n,d),
			uvec::project_to_plane(triInfo.vertices.at(2),n,d)
		};
		auto area = Geometry::calc_triangle_area(vertsPs.at(0),vertsPs.at(1),vertsPs.at(2));
		constexpr auto AREA_EPSILON = 0.004f;
		if(area < AREA_EPSILON)
			continue; // Points don't actually create a triangle; skip it; TODO: It would be cheaper to use dot products for this!
		std::vector<VertexInfo> triDecalVerts = {
			{Vector2{vertsPs.at(0).x,vertsPs.at(0).y},triInfo.originalMeshVertexIndex.at(0)},
			{Vector2{vertsPs.at(1).x,vertsPs.at(1).y},triInfo.originalMeshVertexIndex.at(1)},
			{Vector2{vertsPs.at(2).x,vertsPs.at(2).y},triInfo.originalMeshVertexIndex.at(2)}
		};
		triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0),vertsPs.at(1),vertsPs.at(2),triDecalVerts,projectorAABBPoints.at(0),projectorAABBPoints.at(1));
		triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0),vertsPs.at(1),vertsPs.at(2),triDecalVerts,projectorAABBPoints.at(1),projectorAABBPoints.at(2));
		triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0),vertsPs.at(1),vertsPs.at(2),triDecalVerts,projectorAABBPoints.at(2),projectorAABBPoints.at(3));
		triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0),vertsPs.at(1),vertsPs.at(2),triDecalVerts,projectorAABBPoints.at(3),projectorAABBPoints.at(0));

		auto triOffset = outVerts.size();
		for(auto &vInfo : triDecalVerts)
		{
			outVerts.push_back({});
			auto &v = outVerts.back();
			if(vInfo.barycentricCoordinates.has_value())
			{
				// This is a new vertex; Calculate the position
				auto f0 = (1.f -(vInfo.barycentricCoordinates->x +vInfo.barycentricCoordinates->y));
				auto f1 = vInfo.barycentricCoordinates->x;
				auto f2 = vInfo.barycentricCoordinates->y;

				v.position = p0 *f0 +p1 *f1 +p2 *f2;
			}
			else
				v.position = verts.at(vInfo.originalMeshVertexIndex).position; // Keep the original vertex position
			v.uv = {
				1.0 -(v.position.x -projectorAABBBounds.first.x) /(projectorAABBBounds.second.x -projectorAABBBounds.first.x),
				1.0 -(v.position.y -projectorAABBBounds.first.y) /(projectorAABBBounds.second.y -projectorAABBBounds.first.y)
			};
		}

		// Note: triDecalVerts contains triangles as strips; We need it as actual triangles
		for(auto i=decltype(triDecalVerts.size()){2};i<triDecalVerts.size();++i)
		{
			std::array<uint32_t,3> triIndices = {0u,i,i +1};
			for(auto idx : triIndices)
				outTris.push_back(triOffset +idx);
		}
	}
}
void DecalProjector::DebugDraw(float duration) const
{
	auto &points = GetProjectorCubePoints();
	std::vector<Vector3> lines = {
		points.at(1),points.at(2),
		points.at(2),points.at(4),
		points.at(4),points.at(3),
		points.at(3),points.at(1),

		points.at(1),points.at(5),
		points.at(2),points.at(6),
		points.at(4),points.at(8),
		points.at(3),points.at(7),

		points.at(5),points.at(6),
		points.at(6),points.at(8),
		points.at(8),points.at(7),
		points.at(7),points.at(5)
	};
	auto prismSize = GetSize();
	auto &pose = GetPose();
	for(auto &l : lines)
		l = pose *(l *prismSize);
	::DebugRenderer::DrawLines(lines,Color::White,duration);

	lines.clear();
	auto &prismPos = pose.GetOrigin();
	Vector3 forward,right,up;
	GetOrthogonalBasis(forward,right,up);
	auto posCenter = prismPos -forward *prismSize *0.5f;
	lines.push_back(prismPos -forward *prismSize);
	lines.push_back(posCenter);

	lines.push_back(points.at(1));
	lines.push_back(points.at(7));

	lines.push_back(points.at(3));
	lines.push_back(points.at(5));
	for(auto &l : lines)
		l = pose *(l *prismSize);
	::DebugRenderer::DrawLines(lines,Color::Black,duration);
}

/////////

void CDecalComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pSpriteComponent = ent.AddComponent<pragma::CSpriteComponent>();
	if(pSpriteComponent.valid())
		pSpriteComponent->SetOrientationType(pragma::CParticleSystemComponent::OrientationType::World);
}

void CDecalComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.expired())
		return;
	auto &pos = pTrComponent->GetPosition();
	auto dir = -pTrComponent->GetForward();

	auto *pSpriteComponent = static_cast<pragma::CSpriteComponent*>(ent.FindComponent("sprite").get());
	TraceData tr {};
	tr.SetSource(pos);
	tr.SetTarget(pos +dir *18.f);
	tr.SetFlags(RayCastFlags::Default | RayCastFlags::IgnoreDynamic);
	auto r = c_game->RayCast(tr);
	if(r.hitType != RayCastHitType::None)
	{
		pTrComponent->SetPosition(r.position +dir *-std::numeric_limits<float>::epsilon());
		auto &rot = pTrComponent->GetOrientation();
		auto angle = uquat::get_angle(rot);
		pTrComponent->SetOrientation(uquat::create(r.normal,angle));
	}
	if(pSpriteComponent != nullptr)
		pSpriteComponent->StartParticle();
}

void CDecalComponent::ApplyDecal()
{
	auto &ent = GetEntity();
	float size = 16.f; // TODO
	DecalProjector projector {ent.GetPosition(),ent.GetRotation(),size};
	//projector.GenerateDecalMesh(mesh,
}

luabind::object CDecalComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CDecalComponentHandleWrapper>(l);}

//////////////

void CEnvDecal::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDecalComponent>();
}
