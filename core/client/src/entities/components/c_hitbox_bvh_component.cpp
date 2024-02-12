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
	for(auto &hbObb : m_hitboxBvh->primitives) {
		if(hbObb.boneId >= bonePoses.size())
			continue;
		auto &pose = bonePoses[hbObb.boneId];
		hbObb.pose = pose;
		// TODO: Re-scale min,max bounds?
	}
	//m_hitboxBvh->Refit();
}

void CHitboxBvhComponent::InitializeHitboxBvh()
{
	auto &mdl = GetEntity().GetModel();
	auto &hitboxes = mdl->GetHitboxes();
	auto bvhTree = std::make_unique<ObbBvhTree>();
	auto &hitboxObbs = bvhTree->primitives;
	hitboxObbs.reserve(hitboxes.size());
	auto &ref = mdl->GetReference();
	for(auto &pair : hitboxes) {
		umath::ScaledTransform pose;
		if(!ref.GetBonePose(pair.first, pose))
			continue;
		auto it = m_hitboxBvhs.find(pair.first);
		if(it == m_hitboxBvhs.end())
			continue; // No mesh for this hitbox
		auto &hb = pair.second;
		hitboxObbs.push_back({hb.min, hb.max});
		auto &hbObb = hitboxObbs.back();
		hbObb.pose = pose;
		hbObb.boneId = pair.first;
	}

	bvhTree->InitializeBvh();
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
	for(auto udmHbMesh : udmHbMeshes.ElIt()) {
		auto &boneName = udmHbMesh.key;
		auto boneId = skeleton.LookupBone(std::string {boneName});
		if(boneId < 0)
			continue;

		umath::ScaledTransform pose;
		if(!ref.GetBonePose(boneId, pose))
			continue;
		auto invPose = pose.GetInverse();

		std::string meshUuid;
		udmHbMesh.property["meshUuid"](meshUuid);
		auto itMesh = mdlMeshes.find(meshUuid);
		if(itMesh == mdlMeshes.end())
			continue;
		auto &mesh = itMesh->second;
		std::vector<uint32_t> triIndices;
		udmHbMesh.property["triangleIndices"](triIndices);
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

bool CHitboxBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo)
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

	// Raycast against our hitbox BVH
	std::vector<ObbBvhTree::HitData> hits;
	auto res = m_hitboxBvh->Raycast(originEs, dirEs, minDist, maxDist, hits);
	if(!res)
		return false;

	// Now we can do a more precise raycast against the hitbox mesh bvh trees
	for(auto &hitData : hits) {
		auto &hObb = m_hitboxBvh->primitives[hitData.primitiveIndex];
		auto it = m_hitboxBvhs.find(hObb.boneId);
		if(it == m_hitboxBvhs.end())
			continue;
		auto boneId = hObb.boneId;
		if(boneId >= effectiveBonePoses.size())
			continue;
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
		auto isTestBone = (bone->name == "ValveBiped.Bip01_L_ForeArm");
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
							::DebugRenderer::DrawBox(smMin, smMax, EulerAngles {}, Color {0, 0, 255, 64}, Color::White, 20.f);
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
						auto udmHb = udmHbMeshes[bone->name];
						udmHb["meshUuid"] = util::uuid_to_string(uuid);
						udmHb.AddArray("triangleIndices", usedTris, udm::ArrayType::Compressed);
					}
				}
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

void CHitboxBvhComponent::DebugDraw()
{
	if(!m_hitboxBvh)
		return;
	auto color = Color::Red;
	auto outlineColor = Color::Lime;
	auto duration = 20.f;
	for(auto &hObb : m_hitboxBvh->primitives) {
		auto pose = hObb.pose;
		pose.TranslateLocal(hObb.position);
		auto &pos = pose.GetOrigin();
		::DebugRenderer::DrawBox(pos, -hObb.halfExtents, hObb.halfExtents, EulerAngles {pose.GetRotation()}, color, outlineColor, duration);
	}

	auto &mdl = GetEntity().GetModel();
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

pragma::bvh::BBox pragma::CHitboxBvhComponent::HitboxObb::ToBvhBBox(Vector3 &outOrigin) const
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
	std::vector<pragma::bvh::BBox> bboxes {numObbs};
	std::vector<pragma::bvh::Vec> centers {numObbs};
	executor.for_each(0, numObbs, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			Vector3 center;
			bboxes[i] = primitives[i].ToBvhBBox(center);
			centers[i] = bvh::to_bvh_vector(center);
		}
	});

	bvh = ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::build(GetThreadPool(), bboxes, centers, config);
	return true;
}

bool ObbBvhTree::Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, std::vector<HitData> &outHits)
{
	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	auto distDiff = maxDist - minDist;
	::bvh::v2::SmallStack<pragma::bvh::Bvh::Index, stack_size> stack;
	auto ray = pragma::bvh::get_ray(origin, dir, minDist, maxDist);
	bvh.intersect<false, use_robust_traversal>(ray, bvh.get_root().index, stack, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			size_t j = bvh.prim_ids[i];

			auto &obb = primitives[j];
			float dist;

			auto tmpDir = dir * maxDist;
			auto hit = umath::intersection::line_obb(origin, tmpDir, obb.min, obb.max, &dist, obb.pose.GetOrigin(), obb.pose.GetRotation());
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
	});
	return !outHits.empty();
}
