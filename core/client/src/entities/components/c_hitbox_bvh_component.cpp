/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_hitbox_bvh_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/debug/c_debugoverlay.h"
#include <pragma/entities/components/bvh_data.hpp>
#include <pragma/entities/components/util_bvh.hpp>
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/logging.hpp"
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>
#include <panima/skeleton.hpp>
#include <pragma/entities/components/base_bvh_component.hpp>
#include <mathutil/boundingvolume.h>
#include <bvh/v2/stack.h>
#include <ranges>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static spdlog::logger &LOGGER = pragma::register_logger("bvh");

using namespace pragma;
#pragma optimize("", off)
CHitboxBvhComponent::CHitboxBvhComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void CHitboxBvhComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CHitboxBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnModelChanged(); });
}

void CHitboxBvhComponent::InitializeBvh()
{
	if(!GetEntity().IsSpawned())
		return;
	InitializeModel();
	InitializeHitboxBvh();
	DebugDraw();
}

void CHitboxBvhComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeBvh();
}

void CHitboxBvhComponent::OnModelChanged() { InitializeBvh(); }

bool CHitboxBvhComponent::InitializeModel()
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	mdl->GenerateLowLevelLODs(GetGame());
	auto extData = mdl->GetExtensionData();
	auto udmHbMeshes = extData["hitboxMeshes"];
	if(!udmHbMeshes)
		pragma::CHitboxBvhComponent::generate_hitbox_meshes(*mdl);
	InitializeHitboxMeshes();
	// TODO: Save model
	// std::string err;
	// mdl->Save(GetGame(), err);

	// TODO: Only initialize bvh meshes ONCE per model! -> Re-use between entities
	return true;
}

void CHitboxBvhComponent::UpdateTest() { UpdateHitboxBvh(); }

void CHitboxBvhComponent::UpdateHitboxBvh()
{
	if(!m_hitboxBvh)
		return;
	auto *animC = static_cast<CAnimatedComponent *>(GetEntity().GetAnimatedComponent().get());
	if(!animC)
		return;
	auto &bonePoses = animC->GetProcessedBones();
	auto &bvh = m_hitboxBvh->bvh;
	// It's important to process the nodes in reverse order, as this makes
	// sure that children are processed before their parents.
	for(auto &node : std::ranges::reverse_view {bvh.nodes}) {
		if(node.is_leaf()) {
			// refit node according to contents
			auto begin = node.index.first_id;
			auto end = begin + node.index.prim_count;
			for(size_t i = begin; i < end; ++i) {
				size_t j = bvh.prim_ids[i];

				auto &hbObb = m_hitboxBvh->primitives[j];
				if(hbObb.boneId >= bonePoses.size())
					continue;
				auto &pose = bonePoses[hbObb.boneId];
				Vector3 origin;
				auto bbox = hbObb.ToBvhBBox(pose, origin);
				node.set_bbox(bbox);
			}
		}
		else {
			auto &left = bvh.nodes[node.index.first_id];
			auto &right = bvh.nodes[node.index.first_id + 1];
			node.set_bbox(left.get_bbox().extend(right.get_bbox()));
		}
	}
	m_hitboxBvh->Refit();
}

void CHitboxBvhComponent::InitializeHitboxBvh()
{
	auto &ent = GetEntity();
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return;
	auto &mdl = ent.GetModel();
	auto &hitboxes = mdl->GetHitboxes();
	auto bvhTree = std::make_unique<ObbBvhTree>();
	auto &hitboxObbs = bvhTree->primitives;
	hitboxObbs.reserve(hitboxes.size());
	auto numBones = mdl->GetSkeleton().GetBoneCount();
	for(auto &pair : hitboxes) {
		if(pair.first >= numBones)
			continue;
		auto it = m_hitboxBvhs.find(pair.first);
		if(it == m_hitboxBvhs.end())
			continue; // No mesh for this hitbox
		auto &hb = pair.second;
		hitboxObbs.push_back({hb.min, hb.max});
		auto &hbObb = hitboxObbs.back();
		hbObb.boneId = pair.first;
	}
	bvhTree->InitializeBvh(animC->GetProcessedBones());
	m_hitboxBvh = std::move(bvhTree);
}

void CHitboxBvhComponent::InitializeHitboxMeshes()
{
	auto &mdl = GetEntity().GetModel();
	auto extData = mdl->GetExtensionData();
	auto udmHbMeshes = extData["hitboxMeshes"];
	if(!udmHbMeshes)
		return;
	std::unordered_map<std::string, std::shared_ptr<ModelSubMesh>> mdlMeshes;
	for(auto &mg : mdl->GetMeshGroups()) {
		for(auto &m : mg->GetMeshes()) {
			for(auto &sm : m->GetSubMeshes()) {
				mdlMeshes[util::uuid_to_string(sm->GetUuid())] = sm;
			}
		}
	}
	auto &skeleton = mdl->GetSkeleton();
	auto &ref = mdl->GetReference();
	for(auto udmHbMeshPair : udmHbMeshes.ElIt()) {
		auto &boneName = udmHbMeshPair.key;
		auto boneId = skeleton.LookupBone(std::string {boneName});
		if(boneId < 0)
			continue;
		umath::ScaledTransform pose;
		if(!ref.GetBonePose(boneId, pose))
			continue;
		auto invPose = pose.GetInverse();

		auto &udmBoneMeshes = udmHbMeshPair.property;
		for(auto udmBoneMesh : udmBoneMeshes) {
			std::string meshUuid;
			udmBoneMesh["meshUuid"](meshUuid);
			auto itMesh = mdlMeshes.find(meshUuid);
			if(itMesh == mdlMeshes.end())
				continue;
			auto &mesh = itMesh->second;
			std::vector<uint32_t> triIndices;
			udmBoneMesh["triangleIndices"](triIndices);
			if(triIndices.empty())
				continue;
			auto &verts = mesh->GetVertices();

			std::vector<bvh::Primitive> bvhTris;
			bvhTris.reserve(triIndices.size());
			auto valid = true;
			mesh->VisitIndices([&triIndices, &verts, &invPose, &bvhTris, &valid](auto *indexDataSrc, uint32_t numIndicesSrc) {
				auto numVerts = verts.size();
				for(auto triIdx : triIndices) {
					if(triIdx + 2 >= numIndicesSrc) {
						valid = false;
						return;
					}
					auto idx0 = indexDataSrc[triIdx * 3];
					auto idx1 = indexDataSrc[triIdx * 3 + 1];
					auto idx2 = indexDataSrc[triIdx * 3 + 2];
					if(idx0 >= numVerts || idx1 >= numVerts || idx2 >= numVerts) {
						valid = false;
						return;
					}
					auto &v0 = verts[idx0];
					auto &v1 = verts[idx1];
					auto &v2 = verts[idx2];
					auto pos0 = invPose * v0.position;
					auto pos1 = invPose * v1.position;
					auto pos2 = invPose * v2.position;
					bvhTris.push_back(bvh::create_triangle(pos0, pos1, pos2));
				}
			});

			if(!valid)
				continue;
			auto bvhData = bvh::create_bvh_data(std::move(bvhTris));
			auto it = m_hitboxBvhs.find(boneId);
			if(it == m_hitboxBvhs.end())
				it = m_hitboxBvhs.insert(std::make_pair(boneId, std::vector<HitboxBvhInfo> {})).first;
			it->second.push_back({});
			auto &hitboxBvhInfo = it->second.back();
			hitboxBvhInfo.bvhTree = std::move(bvhData);
			hitboxBvhInfo.bvhTriToOriginalTri = std::move(triIndices);
			hitboxBvhInfo.mesh = mesh;
		}
	}
}

static void draw_mesh(const pragma::bvh::MeshBvhTree &bvhTree, const umath::ScaledTransform &pose, const Color &color, const Color &outlineColor, float duration = 0.1f)
{
	std::vector<Vector3> verts;
	verts.reserve(bvhTree.primitives.size() * 3);
	for(auto &tri : bvhTree.primitives) {
		verts.push_back(pose * pragma::bvh::from_bvh_vector(tri.p0));
		verts.push_back(pose * pragma::bvh::from_bvh_vector(tri.p1));
		verts.push_back(pose * pragma::bvh::from_bvh_vector(tri.p2));
	}
	DebugRenderer::DrawMesh(verts, color, outlineColor, duration);
}

bool CHitboxBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo, const bvh::DebugDrawInfo *debugDrawInfo)
{
	if(!m_hitboxBvh)
		return false;
	auto &ent = GetEntity();
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return false;
	auto &effectiveBonePoses = animC->GetProcessedBones();

	// Move ray to entity space
	auto entPoseInv = ent.GetPose().GetInverse();
	auto originEs = entPoseInv * origin;
	auto dirEs = dir;
	uvec::rotate(&dirEs, entPoseInv.GetRotation());

	auto debugDraw = (debugDrawInfo->flags != bvh::DebugDrawInfo::Flags::None);

	// Raycast against our hitbox BVH
	std::vector<ObbBvhTree::HitData> hits;
	auto res = m_hitboxBvh->Raycast(originEs, dirEs, minDist, maxDist, effectiveBonePoses, hits, debugDrawInfo);
	if(!res)
		return false;

	// Now we can do a more precise raycast against the hitbox mesh bvh trees
	Con::cout << Con::endl;
	for(auto &hitData : hits) {
		auto &hObb = m_hitboxBvh->primitives[hitData.primitiveIndex];
		auto it = m_hitboxBvhs.find(hObb.boneId);
		if(it == m_hitboxBvhs.end())
			continue;
		auto boneId = hObb.boneId;
		if(boneId >= effectiveBonePoses.size())
			continue;
		Con::cout << ent.GetModel()->GetSkeleton().GetBone(hObb.boneId).lock()->name << Con::endl;
		auto bonePoseInv = effectiveBonePoses[boneId].GetInverse();

		// Move ray to bone space
		auto originBs = bonePoseInv * originEs;
		auto dirBs = dirEs;
		uvec::rotate(&dirBs, bonePoseInv.GetRotation());

		auto &hitboxBvhInfos = it->second;
		std::optional<float> closestDistance {};
		for(auto &hitboxBvhInfo : hitboxBvhInfos) {
			auto &meshBvh = hitboxBvhInfo.bvhTree;

			bvh::MeshBvhTree::HitData meshHitData;
			auto res = meshBvh->Raycast(originBs, dirBs, minDist, maxDist, meshHitData);
			if(debugDraw) {
				if(umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawTraversedMeshesBit) || (res && umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawHitMeshesBit))) {
					auto color = res ? Color {0, 255, 0, 64} : Color {255, 0, 0, 64};
					draw_mesh(*meshBvh, effectiveBonePoses[boneId] * debugDrawInfo->basePose, color, Color::White, debugDrawInfo->duration);
				}
			}
			if(!res)
				continue;
			if(meshHitData.primitiveIndex >= hitboxBvhInfo.bvhTriToOriginalTri.size())
				continue; // Unreachable
			if(closestDistance && meshHitData.t >= *closestDistance)
				continue;
			closestDistance = meshHitData.t;

			outHitInfo.mesh = hitboxBvhInfo.mesh;
			outHitInfo.distance = meshHitData.t * maxDist;
			outHitInfo.primitiveIndex = hitboxBvhInfo.bvhTriToOriginalTri[meshHitData.primitiveIndex];
			outHitInfo.u = meshHitData.u;
			outHitInfo.v = meshHitData.v;
			outHitInfo.t = meshHitData.t;
		}
		outHitInfo.entity = GetEntity().GetHandle();
		if(closestDistance.has_value())
			return true;
	}
	return false;
}

void CHitboxBvhComponent::OnRemove() {}

void pragma::CHitboxBvhComponent::generate_hitbox_meshes(Model &mdl)
{
	auto &lods = mdl.GetLODs();
	if(lods.empty())
		return;
	auto extData = mdl.GetExtensionData();
	auto udmHbMeshes = extData["hitboxMeshes"];

	auto lodLast = lods.back();

	auto &ref = mdl.GetReference();
	auto &hitboxes = mdl.GetHitboxes();
	auto &skeleton = mdl.GetSkeleton();
	for(auto &pair : hitboxes) {
		auto bone = skeleton.GetBone(pair.first).lock();
		if(!bone)
			continue;
		Con::cout << "Bone: " << bone->name << Con::endl;
		auto &hb = pair.second;
		umath::ScaledTransform pose;
		if(!ref.GetBonePose(pair.first, pose))
			continue;
		auto isTestBone = (bone->name == "lShldrTwist");
		//ValveBiped.Bip01_L_UpperArm "); // No hitbox for UpperArm? -> but rShldrTwist2 has hitbox!
		//ValveBiped.Bip01_L_ForeArm ");
		auto &pos = pose.GetOrigin();
		auto &rot = pose.GetRotation();
		std::vector<umath::Plane> planes {
		  umath::Plane {uquat::up(rot), pos + uquat::up(rot) * hb.max.y},
		  umath::Plane {-uquat::up(rot), pos + uquat::up(rot) * hb.min.y},
		  umath::Plane {uquat::forward(rot), pos + uquat::forward(rot) * hb.max.z},
		  umath::Plane {-uquat::forward(rot), pos + uquat::forward(rot) * hb.min.z},
		  umath::Plane {-uquat::right(rot), pos - uquat::right(rot) * hb.max.x},
		  umath::Plane {uquat::right(rot), pos - uquat::right(rot) * hb.min.x},
		};
		auto &hbMin = hb.min;
		auto &hbMax = hb.max;
		if(isTestBone) {
			::DebugRenderer::DrawBox(pos, hbMin, hbMax, rot, Color {255, 255, 0, 64}, Color::White, 20.f);
		}
		struct BoneMesh {
			std::string meshUuid;
			std::vector<uint32_t> usedTris;
		};
		std::vector<std::shared_ptr<BoneMesh>> boneMeshes;
		for(auto &pair : lodLast.meshReplacements) {
			auto mg = mdl.GetMeshGroup(pair.second);
			if(!mg)
				continue;
			for(auto &mesh : mg->GetMeshes()) {
				for(auto &subMesh : mesh->GetSubMeshes()) {
					auto &uuid = subMesh->GetUuid();
					if(uuid == util::Uuid {}) {
						LOGGER.warn("Mesh with invalid uuid in model '{}'! Skipping...", mdl.GetName());
						continue;
					}
					Vector3 smMin, smMax;
					subMesh->GetBounds(smMin, smMax);
					if(umath::intersection::aabb_in_plane_mesh(smMin, smMax, planes) != umath::intersection::Intersect::Outside) {
						if(isTestBone) {
							::DebugRenderer::DrawBox(smMin, smMax, EulerAngles {}, Color {0, 0, 255, 64}, Color::White, 60.f);
						}
						auto &verts = subMesh->GetVertices();
						auto numVerts = verts.size();

						std::vector<uint32_t> usedTris;
						usedTris.reserve(subMesh->GetTriangleCount());
						subMesh->VisitIndices([&verts, &usedTris, &hbMin, &hbMax, &pose](auto *indexDataSrc, uint32_t numIndicesSrc) {
							for(auto i = decltype(numIndicesSrc) {0u}; i < numIndicesSrc; i += 3) {
								auto idx0 = indexDataSrc[i];
								auto idx1 = indexDataSrc[i + 1];
								auto idx2 = indexDataSrc[i + 2];
								auto &v0 = verts[idx0];
								auto &v1 = verts[idx1];
								auto &v2 = verts[idx2];

								if(umath::intersection::obb_triangle(hbMin, hbMax, pose, v0.position, v1.position, v2.position))
									usedTris.push_back(i / 3);
							}
						});

						if(usedTris.empty())
							continue;
						if(boneMeshes.size() == boneMeshes.capacity())
							boneMeshes.reserve(boneMeshes.size() * 2 + 10);
						auto bm = std::make_shared<BoneMesh>();
						bm->meshUuid = util::uuid_to_string(uuid);
						bm->usedTris = std::move(usedTris);
						boneMeshes.push_back(bm);
					}
				}
			}
		}

		if(!boneMeshes.empty()) {
			auto udmBoneMeshes = udmHbMeshes.AddArray(bone->name, boneMeshes.size());
			uint32_t idx = 0;
			for(auto &bm : boneMeshes) {
				udmBoneMeshes[idx]["meshUuid"] = bm->meshUuid;
				udmBoneMeshes[idx].AddArray("triangleIndices", bm->usedTris, udm::ArrayType::Compressed);
				++idx;
			}
		}
	}
	/*
	// Debug Print
	std::stringstream ss;
	extData->ToAscii(udm::AsciiSaveFlags::DontCompressLz4Arrays, ss, "");
	Con::cout << "Extension data:" << Con::endl;
	Con::cout << ss.str() << Con::endl;
	*/
}

void CHitboxBvhComponent::DebugDrawHitboxMeshes(BoneId boneId, float duration) const
{
	auto it = m_hitboxBvhs.find(boneId);
	if(it == m_hitboxBvhs.end())
		return;
	const std::array<Color, 6> colors {Color::Red, Color::Lime, Color::Blue, Color::Yellow, Color::Cyan, Color::Magenta};
	auto &hitboxBvhInfos = it->second;
	uint32_t colorIdx = 0;
	for(auto &hitboxBvhInfo : hitboxBvhInfos) {
		auto &col = colors[colorIdx];
		auto &verts = hitboxBvhInfo.mesh->GetVertices();

		std::vector<Vector3> dbgVerts;
		dbgVerts.reserve(dbgVerts.size() + hitboxBvhInfo.mesh->GetIndexCount());
		hitboxBvhInfo.mesh->VisitIndices([&verts, &dbgVerts, &hitboxBvhInfo](auto *indexDataSrc, uint32_t numIndicesSrc) {
			dbgVerts.reserve(hitboxBvhInfo.bvhTriToOriginalTri.size());
			for(auto triIdx : hitboxBvhInfo.bvhTriToOriginalTri) {
				auto idx0 = triIdx * 3;
				auto idx1 = idx0 + 1;
				auto idx2 = idx0 + 2;
				if(idx2 >= numIndicesSrc)
					continue;
				idx0 = indexDataSrc[idx0];
				idx1 = indexDataSrc[idx1];
				idx2 = indexDataSrc[idx2];
				dbgVerts.push_back(verts[idx0].position);
				dbgVerts.push_back(verts[idx1].position);
				dbgVerts.push_back(verts[idx2].position);
			}
		});

		DebugRenderer::DrawMesh(dbgVerts, col, Color::White, duration);

		colorIdx = (colorIdx + 1) % colors.size();
	}
}

void CHitboxBvhComponent::DebugDraw()
{
	if(!m_hitboxBvh)
		return;
	auto &ent = GetEntity();
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return;
	auto &effectivePoses = animC->GetProcessedBones();
	auto numBones = effectivePoses.size();
	auto color = Color::Red;
	auto outlineColor = Color::Lime;
	auto duration = 20.f;
	for(auto &hObb : m_hitboxBvh->primitives) {
		if(hObb.boneId >= numBones)
			continue;
		auto &pose = effectivePoses[hObb.boneId];
		pose.TranslateLocal(hObb.position);
		auto &pos = pose.GetOrigin();
		::DebugRenderer::DrawBox(pos, -hObb.halfExtents, hObb.halfExtents, EulerAngles {pose.GetRotation()}, color, outlineColor, duration);
	}

	auto &mdl = ent.GetModel();
	auto &ref = mdl->GetReference();
	for(auto &pair : m_hitboxBvhs) {
		umath::ScaledTransform pose;
		if(!ref.GetBonePose(pair.first, pose))
			continue;
		std::vector<Vector3> dbgMeshVerts;
		auto &hitboxBvhInfos = pair.second;
		for(auto &hitboxBvhInfo : hitboxBvhInfos) {
			dbgMeshVerts.reserve(dbgMeshVerts.size() + hitboxBvhInfo.bvhTree->primitives.size() * 3);
			for(auto &prim : hitboxBvhInfo.bvhTree->primitives) {
				auto &p0 = prim.p0;
				auto &p1 = prim.p1;
				auto &p2 = prim.p2;
				auto v0 = Vector3 {p0[0], p0[1], p0[2]};
				auto v1 = Vector3 {p1[0], p1[1], p1[2]};
				auto v2 = Vector3 {p2[0], p2[1], p2[2]};
				v0 = pose * v0;
				v1 = pose * v1;
				v2 = pose * v2;
				dbgMeshVerts.push_back(v0);
				dbgMeshVerts.push_back(v1);
				dbgMeshVerts.push_back(v2);
			}
		}
		::DebugRenderer::DrawMesh(dbgMeshVerts, color, outlineColor, duration);
	}
}

pragma::CHitboxBvhComponent::HitboxObb::HitboxObb(const Vector3 &min, const Vector3 &max) : min {min}, max {max}
{
	position = (max + min) / 2.f;
	halfExtents = (max - min) / 2.f;
}

pragma::bvh::BBox pragma::CHitboxBvhComponent::HitboxObb::ToBvhBBox(const umath::ScaledTransform &pose, Vector3 &outOrigin) const
{
	// Calculate AABB around OBB
	auto rotationMatrix = glm::mat3_cast(pose.GetRotation());
	auto worldX = rotationMatrix * Vector3(1.0f, 0.0f, 0.0f);
	auto worldY = rotationMatrix * Vector3(0.0f, 1.0f, 0.0f);
	auto worldZ = rotationMatrix * Vector3(0.0f, 0.0f, 1.0f);

	auto xAxisExtents = glm::abs(worldX * halfExtents.x);
	auto yAxisExtents = glm::abs(worldY * halfExtents.y);
	auto zAxisExtents = glm::abs(worldZ * halfExtents.z);

	auto posRot = position;
	uvec::rotate(&posRot, pose.GetRotation());
	auto &origin = outOrigin;
	origin = pose.GetOrigin() + posRot;

	auto aabbMin = origin - xAxisExtents - yAxisExtents - zAxisExtents;
	auto aabbMax = origin + xAxisExtents + yAxisExtents + zAxisExtents;
	return pragma::bvh::BBox {pragma::bvh::to_bvh_vector(aabbMin), pragma::bvh::to_bvh_vector(aabbMax)};
}

bool ObbBvhTree::DoInitializeBvh(::bvh::v2::ParallelExecutor &executor, ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config &config)
{
	auto numObbs = primitives.size();
	if(numObbs == 0)
		return false;
	auto numBones = m_poses->size();
	std::vector<pragma::bvh::BBox> bboxes {numObbs};
	std::vector<pragma::bvh::Vec> centers {numObbs};
	executor.for_each(0, numObbs, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			Vector3 center;
			auto &hObb = primitives[i];
			if(hObb.boneId >= numBones)
				continue;
			bboxes[i] = hObb.ToBvhBBox((*m_poses)[hObb.boneId], center);
			centers[i] = bvh::to_bvh_vector(center);
		}
	});

	bvh = ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::build(GetThreadPool(), bboxes, centers, config);
	return true;
}

void ObbBvhTree::InitializeBvh(const std::vector<umath::ScaledTransform> &poses)
{
	m_poses = &poses;
	pragma::bvh::BvhTree::InitializeBvh();
	m_poses = nullptr;
}

bool ObbBvhTree::Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const std::vector<umath::ScaledTransform> &bonePoses, std::vector<HitData> &outHits, const bvh::DebugDrawInfo *debugDrawInfo)
{
	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	std::function<void(const bvh::Node &, const bvh::Node &)> innerFn = [](const bvh::Node &, const bvh::Node &) {};
	if(debugDrawInfo && umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawTraversedNodesBit)) {
		auto &game = *pragma::get_cengine()->GetClientState()->GetGameState();
		innerFn = [&game, debugDrawInfo](const bvh::Node &a, const bvh::Node &b) {
			auto col = Color {255, 0, 255, 64};
			bvh::debug::draw_node(game, a, debugDrawInfo->basePose, col, debugDrawInfo->duration);
			bvh::debug::draw_node(game, b, debugDrawInfo->basePose, col, debugDrawInfo->duration);
		};
	}

	auto distDiff = maxDist - minDist;
	::bvh::v2::SmallStack<pragma::bvh::Bvh::Index, stack_size> stack;
	auto ray = pragma::bvh::get_ray(origin, dir, minDist, maxDist);
	auto numBones = bonePoses.size();
	bvh.intersect<false, use_robust_traversal>(
	  ray, bvh.get_root().index, stack,
	  [&](size_t begin, size_t end) {
		  for(size_t i = begin; i < end; ++i) {
			  size_t j = bvh.prim_ids[i];

			  auto &hObb = primitives[j];
			  if(hObb.boneId >= numBones)
				  continue;
			  auto &pose = bonePoses[hObb.boneId];
			  float dist;

			  auto tmpDir = dir * maxDist;
			  auto hit = umath::intersection::line_obb(origin, tmpDir, hObb.min, hObb.max, &dist, pose.GetOrigin(), pose.GetRotation());
			  if(debugDrawInfo && (umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawTraversedLeavesBit) || (umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawHitLeavesBit) && hit))) {
				  auto &game = *pragma::get_cengine()->GetClientState()->GetGameState();
				  auto col = hit ? Color {0, 255, 0, 64} : Color {0, 0, 255, 64};
				  Vector3 center;
				  auto bbox = hObb.ToBvhBBox(pose, center);
				  bvh::debug::draw_node(game, bbox, debugDrawInfo->basePose, col, debugDrawInfo->duration);
			  }
			  if(hit) {
				  dist *= maxDist;
				  if(outHits.size() == outHits.capacity())
					  outHits.reserve(outHits.size() * 2 + 5);
				  HitData hitData {};
				  hitData.primitiveIndex = j;

				  ray.tmax = dist;
				  if(distDiff > 0.0001f)
					  hitData.t = (ray.tmax - minDist) / distDiff;
				  else
					  hitData.t = 0.f;

				  util::insert_sorted(outHits, hitData, [](const HitData &a, const HitData &b) { return a.t < b.t; });
			  }
		  }
		  return false;
	  },
	  innerFn);
	return !outHits.empty();
}
