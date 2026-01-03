// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.env_decal;
import :client_state;
import :debug;
import :engine;
import :entities.components.bvh;
import :entities.components.physics;
import :entities.components.render;
import :entities.components.static_bvh_cache;
import :entities.components.static_bvh_user;
import :game;

using namespace pragma;

DecalProjector::DecalProjector(const Vector3 &pos, const Quat &rot, float size) : m_pose {pos, rot}, m_size {size} {}
const Vector3 &DecalProjector::GetPos() const { return m_pose.GetOrigin(); }
const Quat &DecalProjector::GetRotation() const { return m_pose.GetRotation(); }
const math::Transform &DecalProjector::GetPose() const { return m_pose; }
float DecalProjector::GetSize() const { return m_size; }
std::pair<Vector3, Vector3> DecalProjector::GetAABB() const
{
	auto &rot = GetRotation();

	auto &cubePoints = GetProjectorCubePoints();
	Vector3 min {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	Vector3 max {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
	for(auto p : cubePoints) {
		uvec::rotate(&p, rot);
		uvec::min(&min, p);
		uvec::max(&max, p);
	}
	auto size = GetSize();
	return {min * size, max * size};
}
void DecalProjector::GetOrthogonalBasis(Vector3 &forward, Vector3 &right, Vector3 &up) const
{
	auto projectorSpaceRot = uquat::identity();
	forward = uquat::forward(projectorSpaceRot);
	right = uquat::right(projectorSpaceRot);
	up = uquat::up(projectorSpaceRot);
}
const std::array<Vector3, 8> &DecalProjector::GetProjectorCubePoints() const
{
	static const std::array<Vector3, 8> g_projectorCubePoints = {-uvec::PRM_FORWARD - uvec::PRM_RIGHT - uvec::PRM_UP, +uvec::PRM_FORWARD - uvec::PRM_RIGHT - uvec::PRM_UP, -uvec::PRM_FORWARD + uvec::PRM_RIGHT - uvec::PRM_UP, +uvec::PRM_FORWARD + uvec::PRM_RIGHT - uvec::PRM_UP,
	  -uvec::PRM_FORWARD - uvec::PRM_RIGHT + uvec::PRM_UP, +uvec::PRM_FORWARD - uvec::PRM_RIGHT + uvec::PRM_UP, -uvec::PRM_FORWARD + uvec::PRM_RIGHT + uvec::PRM_UP, +uvec::PRM_FORWARD + uvec::PRM_RIGHT + uvec::PRM_UP};
	return g_projectorCubePoints;
}
std::pair<Vector3, Vector3> DecalProjector::GetProjectorCubeBounds() const
{
	auto prismSize = GetSize();
	return {(-uvec::PRM_FORWARD + uvec::PRM_RIGHT - uvec::PRM_UP) * prismSize, (+uvec::PRM_FORWARD - uvec::PRM_RIGHT + uvec::PRM_UP) * prismSize};
}
std::vector<DecalProjector::VertexInfo> DecalProjector::CropTriangleVertsByLine(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const std::vector<VertexInfo> &verts, const Vector2 &lineStart, const Vector2 &lineEnd)
{
	std::vector<bool> pointsOutOfBounds {};
	pointsOutOfBounds.resize(verts.size());
	for(auto i = decltype(verts.size()) {0u}; i < verts.size(); ++i) {
		auto &vInfo = verts.at(i);
		auto side = math::geometry::get_side_of_point_to_line(lineStart, lineEnd, vInfo.position);
		pointsOutOfBounds.at(i) = (side == math::geometry::LineSide::Right);
	}

	std::vector<VertexInfo> newVerts {};
	for(auto i = decltype(verts.size()) {0u}; i < verts.size(); ++i) {
		auto v0Idx = i;
		auto v1Idx = (i < (verts.size() - 1)) ? (i + 1) : 0;

		// If the current point is in bounds, keep it
		if(pointsOutOfBounds.at(v0Idx) == false)
			newVerts.push_back(verts.at(v0Idx));

		// Check if the line to the next vertex crosses the bounds
		if((pointsOutOfBounds.at(v0Idx) == true && pointsOutOfBounds.at(v1Idx) == false) || (pointsOutOfBounds.at(v0Idx) == false && pointsOutOfBounds.at(v1Idx) == true)) {
			auto intersectionPos = math::intersection::line_line(lineStart, lineEnd, verts.at(v0Idx).position, verts.at(v1Idx).position);
			if(intersectionPos.has_value()) {
				float u, v;
				if(math::geometry::calc_barycentric_coordinates(v0, v1, v2, Vector3 {intersectionPos->x, intersectionPos->y, 0.f}, u, v))
					newVerts.push_back({*intersectionPos, std::numeric_limits<uint32_t>::max(), Vector2 {u, v}});
			}
		}
	}
	return newVerts;
}
bool DecalProjector::GenerateDecalMesh(const std::vector<MeshData> &meshDatas, std::vector<math::Vertex> &outVerts, std::vector<uint16_t> &outTris)
{
	auto bounds = GetProjectorCubeBounds();

	Vector3 forward, right, up;
	GetOrthogonalBasis(forward, right, up);
	auto &n = forward;
	auto d = 0.f;

	// pragma::math::Plane p {n,d};
	// p = GetPose() *p;
	// pragma::get_cgame()->DrawPlane(p.GetNormal(),p.GetDistance(),Color{0,0,255,64},12.f);

	struct TriangleInfo {
		std::array<uint32_t, 3> originalMeshVertexIndex = {};
		std::array<Vector3, 3> vertices = {};
	};

	for(auto &meshData : meshDatas) {
		auto effectivePose = GetPose().GetInverse() * meshData.pose;
		for(auto *subMesh : meshData.subMeshes) {
			if(subMesh->GetGeometryType() != geometry::ModelSubMesh::GeometryType::Triangles)
				continue;
			auto &verts = subMesh->GetVertices();

			std::vector<TriangleInfo> intersectingTris {};
			subMesh->VisitIndices([&](auto *indexData, uint32_t numIndices) {
				intersectingTris.reserve(numIndices);
				// Cull triangles that are outside of the projector bounds
				for(auto i = decltype(numIndices) {0u}; i < numIndices; i += 3) {
					std::array<uint32_t, 3> indices = {indexData[i], indexData[i + 1], indexData[i + 2]};
					auto &v0 = verts.at(indices.at(0));
					auto &v1 = verts.at(indices.at(1));
					auto &v2 = verts.at(indices.at(2));

					auto p0 = effectivePose * v0.position;
					auto p1 = effectivePose * v1.position;
					auto p2 = effectivePose * v2.position;

					if(math::intersection::aabb_triangle(bounds.first, bounds.second, p0, p1, p2) == false)
						continue;
					intersectingTris.push_back({indices, std::array<Vector3, 3> {p0, p1, p2}});
				}
			});

			std::pair<Vector3, Vector3> projectorAABBBounds = {uvec::project_to_plane(bounds.first, n, d), uvec::project_to_plane(bounds.second, n, d)};
			std::array<Vector2, 4> projectorAABBPoints = {Vector2 {projectorAABBBounds.first.x, projectorAABBBounds.first.y}, Vector2 {projectorAABBBounds.second.x, projectorAABBBounds.first.y}, Vector2 {projectorAABBBounds.second.x, projectorAABBBounds.second.y},
			  Vector2 {projectorAABBBounds.first.x, projectorAABBBounds.second.y}};

			// TODO
			auto vertexOffset = outVerts.size();
			outVerts.reserve(outVerts.size() + verts.size());
			outTris.reserve(outTris.size() + subMesh->GetIndexCount());
			for(auto &triInfo : intersectingTris) {
				// Vertex positions in origin space
				auto &p0 = triInfo.vertices.at(0);
				auto &p1 = triInfo.vertices.at(1);
				auto &p2 = triInfo.vertices.at(2);

				std::array<Vector3, 3> vertsPs = {uvec::project_to_plane(triInfo.vertices.at(0), n, d), uvec::project_to_plane(triInfo.vertices.at(1), n, d), uvec::project_to_plane(triInfo.vertices.at(2), n, d)};
				auto area = math::geometry::calc_triangle_area(vertsPs.at(0), vertsPs.at(1), vertsPs.at(2));
				constexpr auto AREA_EPSILON = 0.004f;
				if(area < AREA_EPSILON)
					continue; // Points don't actually create a triangle; skip it; TODO: It would be cheaper to use dot products for this!
				std::vector<VertexInfo> triDecalVerts
				  = {{Vector2 {vertsPs.at(0).x, vertsPs.at(0).y}, triInfo.originalMeshVertexIndex.at(0)}, {Vector2 {vertsPs.at(1).x, vertsPs.at(1).y}, triInfo.originalMeshVertexIndex.at(1)}, {Vector2 {vertsPs.at(2).x, vertsPs.at(2).y}, triInfo.originalMeshVertexIndex.at(2)}};
				triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0), vertsPs.at(1), vertsPs.at(2), triDecalVerts, projectorAABBPoints.at(0), projectorAABBPoints.at(1));
				triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0), vertsPs.at(1), vertsPs.at(2), triDecalVerts, projectorAABBPoints.at(1), projectorAABBPoints.at(2));
				triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0), vertsPs.at(1), vertsPs.at(2), triDecalVerts, projectorAABBPoints.at(2), projectorAABBPoints.at(3));
				triDecalVerts = CropTriangleVertsByLine(vertsPs.at(0), vertsPs.at(1), vertsPs.at(2), triDecalVerts, projectorAABBPoints.at(3), projectorAABBPoints.at(0));

				auto triOffset = outVerts.size();
				for(auto &vInfo : triDecalVerts) {
					outVerts.push_back({});
					auto &v = outVerts.back();
					if(vInfo.barycentricCoordinates.has_value()) {
						// This is a new vertex; Calculate the position
						auto f0 = (1.f - (vInfo.barycentricCoordinates->x + vInfo.barycentricCoordinates->y));
						auto f1 = vInfo.barycentricCoordinates->x;
						auto f2 = vInfo.barycentricCoordinates->y;

						auto &p0Orig = verts.at(triInfo.originalMeshVertexIndex.at(0));
						auto &p1Orig = verts.at(triInfo.originalMeshVertexIndex.at(1));
						auto &p2Orig = verts.at(triInfo.originalMeshVertexIndex.at(2));

						v.normal = p0Orig.normal * f0 + p1Orig.normal * f1 + p2Orig.normal * f2;
						v.position = p0Orig.position * f0 + p1Orig.position * f1 + p2Orig.position * f2;
						uvec::normalize(&v.normal);
					}
					else
						v = verts.at(vInfo.originalMeshVertexIndex); // Keep the original vertex
					v.uv = {1.0 - (vInfo.position.x - projectorAABBBounds.first.x) / (projectorAABBBounds.second.x - projectorAABBBounds.first.x), 1.0 - (vInfo.position.y - projectorAABBBounds.first.y) / (projectorAABBBounds.second.y - projectorAABBBounds.first.y)};
				}

				// Note: triDecalVerts contains triangles as strips; We need it as actual triangles
				for(auto i = decltype(triDecalVerts.size()) {2}; i < triDecalVerts.size(); ++i) {
					std::array<uint32_t, 3> triIndices = {0u, i - 1, i};
					for(auto idx : triIndices)
						outTris.push_back(triOffset + idx);
				}
			}
			for(auto i = vertexOffset; i < outVerts.size(); ++i) {
				auto &v = outVerts.at(i);
				v.position += v.normal * 0.01f; // Small offset to avoid z-fighting
				// Move the vertex positions into entity space
				v.position = meshData.pose * v.position;
			}
		}
	}
	return outTris.empty() == false;
}
void DecalProjector::DebugDraw(float duration) const
{
	auto &points = GetProjectorCubePoints();
	std::vector<Vector3> lines = {points.at(0), points.at(1), points.at(1), points.at(3), points.at(3), points.at(2), points.at(2), points.at(0),

	  points.at(0), points.at(4), points.at(1), points.at(5), points.at(3), points.at(7), points.at(2), points.at(6),

	  points.at(4), points.at(5), points.at(5), points.at(7), points.at(7), points.at(6), points.at(6), points.at(4)};
	auto prismSize = GetSize();
	auto &pose = GetPose();
	for(auto &l : lines)
		l = pose * (l * prismSize);
	debug::DebugRenderer::DrawLines(lines, {colors::White, duration});

	lines.clear();
	auto &prismPos = pose.GetOrigin();
	Vector3 forward, right, up;
	GetOrthogonalBasis(forward, right, up);
	auto posCenter = prismPos - forward * prismSize * 0.5f;
	lines.push_back(prismPos - forward * prismSize);
	lines.push_back(posCenter);

	lines.push_back(points.at(0));
	lines.push_back(points.at(6));

	lines.push_back(points.at(2));
	lines.push_back(points.at(4));
	for(auto &l : lines)
		l = pose * (l * prismSize);
	debug::DebugRenderer::DrawLines(lines, {colors::Black, duration});
}

/////////

void CDecalComponent::Initialize()
{
	BaseEnvDecalComponent::Initialize();
	auto renderC = GetEntity().AddComponent<CRenderComponent>();
	renderC->SetCastShadows(false);
	/*auto &ent = static_cast<pragma::ecs::CBaseEntity&>(GetEntity());
	auto pSpriteComponent = ent.AddComponent<pragma::CSpriteComponent>();
	if(pSpriteComponent.valid())
		pSpriteComponent->SetOrientationType(pragma::pts::ParticleOrientationType::World);*/
}

void CDecalComponent::OnEntitySpawn()
{
	BaseEnvDecalComponent::OnEntitySpawn();
	/*auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto &pos = pTrComponent->GetPosition();
	auto dir = -pTrComponent->GetForward();

	auto *pSpriteComponent = static_cast<pragma::CSpriteComponent*>(ent.FindComponent("sprite").get());
	TraceData tr {};
	tr.SetSource(pos);
	tr.SetTarget(pos +dir *18.f);
	tr.SetFlags(RayCastFlags::Default | RayCastFlags::IgnoreDynamic);
	auto r = pragma::get_cgame()->RayCast(tr);
	if(r.hitType != pragma::physics::RayCastHitType::None)
	{
		pTrComponent->SetPosition(r.position +dir *-std::numeric_limits<float>::epsilon());
		auto &rot = pTrComponent->GetOrientation();
		auto angle = uquat::get_angle(rot);
		pTrComponent->SetOrientation(uquat::create(r.normal,angle));
	}
	if(pSpriteComponent != nullptr)
		pSpriteComponent->StartParticle();*/
	if(m_startDisabled == false)
		SetTickPolicy(TickPolicy::Always);
}

void CDecalComponent::OnTick(double dt)
{
	if(m_decalDirty)
		ApplyDecal();
	SetTickPolicy(TickPolicy::Never);
}

void CDecalComponent::SetSize(float size)
{
	BaseEnvDecalComponent::SetSize(size);
	m_decalDirty = true;
	SetTickPolicy(TickPolicy::Always);
}
void CDecalComponent::SetMaterial(const std::string &mat)
{
	BaseEnvDecalComponent::SetMaterial(mat);
	m_decalDirty = true;
	SetTickPolicy(TickPolicy::Always);
}

DecalProjector CDecalComponent::GetProjector() const
{
	auto &ent = GetEntity();
	auto size = GetSize();
	return DecalProjector {ent.GetPosition(), ent.GetRotation(), size};
}

void CDecalComponent::DebugDraw(float duration) const
{
	auto projector = GetProjector();
	projector.DebugDraw(duration);
}

bool CDecalComponent::ApplyDecal(const std::vector<DecalProjector::MeshData> &meshDatas)
{
	auto projector = GetProjector();
	return ApplyDecal(projector, meshDatas);
}

bool CDecalComponent::ApplyDecal(DecalProjector &projector, const std::vector<DecalProjector::MeshData> &meshDatas)
{
	auto *mat = get_client_state()->LoadMaterial(GetMaterial());
	if(mat == nullptr)
		return false;
	std::vector<math::Vertex> verts;
	std::vector<uint16_t> tris;
	if(projector.GenerateDecalMesh(meshDatas, verts, tris) == false)
		return false;

	// Vertices are in world space; Move them into entity space
	auto &pose = GetEntity().GetPose();
	auto invPose = pose.GetInverse();
	for(auto &v : verts)
		v.position = invPose * v.position;

	auto mdl = get_cgame()->CreateModel();
	auto meshGroup = mdl->GetMeshGroup(0);
	auto subMesh = get_cgame()->CreateModelSubMesh();
	subMesh->GetVertices() = std::move(verts);
	subMesh->SetIndices(tris);
	subMesh->SetSkinTextureIndex(0);
	subMesh->GenerateNormals();

	auto mesh = get_cgame()->CreateModelMesh();
	mesh->AddSubMesh(subMesh);
	meshGroup->AddMesh(mesh);
	mdl->AddMaterial(0, mat);

	mdl->Update(asset::ModelUpdateFlags::All | asset::ModelUpdateFlags::UpdateChildren);

	//decalRenderC->SetDepthBias(-1'000.f,0.f,-2.f);
	// TODO

	GetEntity().SetModel(mdl);

	// GetEntity().SetPose(pose);

	// projector.DebugDraw(15.f);
	return true;
}

bool CDecalComponent::ApplyDecal()
{
	m_decalDirty = false;
	auto *mat = get_client_state()->LoadMaterial(GetMaterial());
	if(mat == nullptr)
		return false;
	std::vector<ecs::CBaseEntity *> targetEnts {};

	auto projector = GetProjector();
	auto projectorAABB = projector.GetAABB();
	auto &projectorOrigin = projector.GetPos();
	projectorAABB.first += projectorOrigin;
	projectorAABB.second += projectorOrigin;

	// pragma::get_cgame()->DrawBox(projectorAABB.first,projectorAABB.second,{},colors::Red,12.f);

	PrimitiveIntersectionInfo bvhIntersectInfo {};
	std::vector<DecalProjector::MeshData> meshDatas {};
	std::unordered_set<geometry::ModelSubMesh *> coveredMeshes;
	auto findIntersectionMeshes = [&projectorAABB, &bvhIntersectInfo, &meshDatas, &coveredMeshes](const BaseBvhComponent &bvhC) {
		if(!bvhC.IntersectionTestAabb(projectorAABB.first, projectorAABB.second, bvhIntersectInfo)) {
			bvhIntersectInfo.primitives.clear();
			return;
		}
		for(auto idx : bvhIntersectInfo.primitives) {
			auto *meshInfo = bvhC.FindPrimitiveMeshInfo(idx);
			if(!meshInfo)
				continue;
			if(coveredMeshes.find(meshInfo->mesh.get()) != coveredMeshes.end())
				continue;
			coveredMeshes.insert(meshInfo->mesh.get());
			if(!meshInfo->entity || meshInfo->entity->HasComponent<CDecalComponent>())
				continue;
			meshDatas.push_back({});
			auto &meshData = meshDatas.back();
			meshData.pose = meshInfo->entity->GetPose();
			meshData.subMeshes.push_back(meshInfo->mesh.get());
		}
		bvhIntersectInfo.primitives.clear();
	};

	{
		ecs::EntityIterator entIt {*get_cgame()};
		entIt.AttachFilter<TEntityIteratorFilterComponent<CStaticBvhCacheComponent>>();
		for(auto *ent : entIt) {
			auto bvhC = ent->GetComponent<CStaticBvhCacheComponent>();
			findIntersectionMeshes(*bvhC);
		}
	}

	{
		ecs::EntityIterator entIt {*get_cgame()};
		entIt.AttachFilter<TEntityIteratorFilterComponent<CBvhComponent>>();
		for(auto *ent : entIt) {
			auto bvhC = ent->GetComponent<CBvhComponent>();
			auto userC = ent->GetComponent<CStaticBvhUserComponent>();
			if(userC.valid() && userC->IsActive())
				continue; // Already covered by static BVH cache
			findIntersectionMeshes(*bvhC);
		}
	}
	return ApplyDecal(projector, meshDatas);
}

void CDecalComponent::ReceiveData(NetPacket &packet)
{
	m_material = packet->ReadString();
	m_size = packet->Read<float>();
	m_startDisabled = packet->Read<bool>();
}

void CDecalComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////

void CEnvDecal::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDecalComponent>();
}
