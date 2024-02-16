/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/hitbox_mesh_bvh_builder.hpp"
#include "pragma/logging.hpp"
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>
#include <pragma/entities/components/bvh_data.hpp>
#include <pragma/entities/components/util_bvh.hpp>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#pragma optimize("", off)
static spdlog::logger &LOGGER = pragma::register_logger("bvh");

static std::unique_ptr<pragma::bvh::MeshBvhTree> generate_mesh_bvh(ModelSubMesh &mesh, const std::vector<uint32_t> &triIndices, const umath::ScaledTransform &invPose)
{
	auto &verts = mesh.GetVertices();

	std::vector<pragma::bvh::Primitive> bvhTris;
	bvhTris.reserve(triIndices.size());
	auto valid = true;
	mesh.VisitIndices([&triIndices, &verts, &invPose, &bvhTris, &valid](auto *indexDataSrc, uint32_t numIndicesSrc) {
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
			bvhTris.push_back(pragma::bvh::create_triangle(pos0, pos1, pos2));
		}
	});

	if(!valid)
		return nullptr;
	return pragma::bvh::create_bvh_data(std::move(bvhTris));
}

static bool generate_mesh_bvh(Model &mdl, const std::string &boneName, pragma::bvh::HitboxMeshBvhBuilder::BoneMeshInfo &boneMeshInfo)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &ref = mdl.GetReference();
	using BoneName = std::string;
	using MeshUuid = std::string;
	auto meshMap = pragma::bvh::get_uuid_mesh_map(mdl);
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId < 0)
		return false;
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId, pose))
		return false;
	auto invPose = pose.GetInverse();
	auto itMesh = meshMap.find(boneMeshInfo.meshUuid);
	if(itMesh == meshMap.end())
		return false;
	auto meshBvh = generate_mesh_bvh(*itMesh->second, boneMeshInfo.usedTris, invPose);
	if(!meshBvh)
		return false;
	boneMeshInfo.meshBvhTree = std::move(meshBvh);
	return true;
}

static bool calc_bone_mesh_info(pragma::bvh::HitboxMeshBvhBuilder::BoneMeshInfo &boneMeshInfo, std::shared_ptr<ModelSubMesh> subMesh, std::vector<umath::Plane> planes, Vector3 hbMin, Vector3 hbMax, umath::ScaledTransform pose, util::Uuid uuid)
{
	Vector3 smMin, smMax;
	subMesh->GetBounds(smMin, smMax);
	if(umath::intersection::aabb_in_plane_mesh(smMin, smMax, planes) == umath::intersection::Intersect::Outside)
		return false;
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
		return false;
	boneMeshInfo.subMesh = subMesh;
	boneMeshInfo.meshUuid = util::uuid_to_string(uuid);
	boneMeshInfo.usedTris = std::move(usedTris);
	return true;
}

static bool generate_bvh_mesh(Model &mdl, const std::string &boneName, pragma::bvh::HitboxMeshBvhBuilder::BoneMeshInfo &boneMeshInfo)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &ref = mdl.GetReference();
	using BoneName = std::string;
	using MeshUuid = std::string;
	auto meshMap = pragma::bvh::get_uuid_mesh_map(mdl);
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId < 0)
		return false;
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId, pose))
		return false;
	auto invPose = pose.GetInverse();

	auto itMesh = meshMap.find(boneMeshInfo.meshUuid);
	if(itMesh == meshMap.end())
		return false;
	auto meshBvh = generate_mesh_bvh(*itMesh->second, boneMeshInfo.usedTris, invPose);
	if(!meshBvh)
		return false;
	boneMeshInfo.meshBvhTree = std::move(meshBvh);
	return true;
}

static void serialize(Model &mdl, const std::string &boneName, pragma::bvh::HitboxMeshBvhBuilder::BoneMeshInfo &boneMeshInfo)
{
	::bvh::v2::StdOutputStream outputStream {boneMeshInfo.serializedBvh};
	auto &bvh = boneMeshInfo.meshBvhTree->bvh;
	bvh.serialize(outputStream);
}

pragma::bvh::HitboxMeshBvhBuilder::HitboxMeshBvhBuilder(BS::thread_pool &threadPool) : m_threadPool {threadPool} {}

bool pragma::bvh::HitboxMeshBvhBuilder::Generate(Model &mdl)
{
	auto &lods = mdl.GetLODs();
	if(lods.empty())
		return false;
	auto &lodLast = lods.back();

	auto &hitboxes = mdl.GetHitboxes();
	auto &skeleton = mdl.GetSkeleton();
	for(auto &[boneId, hb] : hitboxes) {
		auto bone = skeleton.GetBone(boneId).lock();
		if(!bone)
			continue;
		Generate(mdl, boneId, hb, lodLast);
	}

	Serialize(mdl);
	return true;
}

bool pragma::bvh::HitboxMeshBvhBuilder::Generate(Model &mdl, BoneId boneId, const Hitbox &hb, const LODInfo &lodInfo)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &ref = mdl.GetReference();
	auto bone = skeleton.GetBone(boneId).lock();
	if(!bone)
		return false;
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId, pose))
		return false;
	auto &pos = pose.GetOrigin();
	auto &rot = pose.GetRotation();
	const std::vector<umath::Plane> planes {
	  umath::Plane {uquat::up(rot), pos + uquat::up(rot) * hb.max.y},
	  umath::Plane {-uquat::up(rot), pos + uquat::up(rot) * hb.min.y},
	  umath::Plane {uquat::forward(rot), pos + uquat::forward(rot) * hb.max.z},
	  umath::Plane {-uquat::forward(rot), pos + uquat::forward(rot) * hb.min.z},
	  umath::Plane {-uquat::right(rot), pos - uquat::right(rot) * hb.max.x},
	  umath::Plane {uquat::right(rot), pos - uquat::right(rot) * hb.min.x},
	};
	auto &hbMin = hb.min;
	auto &hbMax = hb.max;
	for(auto &pair : lodInfo.meshReplacements) {
		auto mg = mdl.GetMeshGroup(pair.second);
		if(!mg)
			continue;
		for(auto &mesh : mg->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				if(!bvh::is_mesh_bvh_compatible(*subMesh))
					continue;
				auto &uuid = subMesh->GetUuid();
				if(uuid == util::Uuid {}) {
					LOGGER.warn("Mesh with invalid uuid in model '{}'! Skipping...", mdl.GetName());
					continue;
				}

				auto &boneName = bone->name;
				auto bm = std::make_shared<pragma::bvh::HitboxMeshBvhBuilder::BoneMeshInfo>();
				auto genResult = m_threadPool.submit_task([&mdl, subMesh, planes, hbMin, hbMax, pose, uuid, boneName, bm]() -> bool {
					auto success = calc_bone_mesh_info(*bm, subMesh, planes, hbMin, hbMax, pose, uuid);
					if(!success)
						return false;
					if(!generate_bvh_mesh(mdl, boneName, *bm))
						return false;
					serialize(mdl, boneName, *bm);
					return true;
				});

				bm->genResult = std::move(genResult);
				auto it = m_boneMeshMap.find(boneName);
				if(it == m_boneMeshMap.end())
					it = m_boneMeshMap.insert(std::make_pair(boneName, std::vector<std::shared_ptr<BoneMeshInfo>> {})).first;
				it->second.push_back(bm);
			}
		}
	}
	return true;
}

void pragma::bvh::HitboxMeshBvhBuilder::Serialize(Model &mdl)
{
	auto extData = mdl.GetExtensionData();
	auto udmHbMeshes = extData["hitboxMeshes"];
	for(auto &[boneName, boneMeshInfos] : m_boneMeshMap) {
		for(auto it = boneMeshInfos.begin(); it != boneMeshInfos.end();) {
			auto &boneMeshInfo = *it;
			boneMeshInfo->genResult.wait();
			if(!boneMeshInfo->genResult.get()) {
				it = boneMeshInfos.erase(it);
				continue;
			}
			++it;
		}

		if(boneMeshInfos.empty())
			continue;
		auto udmBoneMeshes = udmHbMeshes.AddArray(boneName, boneMeshInfos.size());
		uint32_t idx = 0;
		for(auto &boneMeshInfo : boneMeshInfos) {
			udmBoneMeshes[idx]["meshUuid"] = boneMeshInfo->meshUuid;
			udmBoneMeshes[idx].AddArray("triangleIndices", boneMeshInfo->usedTris, udm::ArrayType::Compressed);

			assert(boneMeshInfo->serializedBvh != nullptr);
			auto &stream = boneMeshInfo->serializedBvh;
			auto view = stream.view();
			auto extData = boneMeshInfo->subMesh->GetExtensionData();
			auto udmBvh = udmBoneMeshes[idx]["bvh"];
			udmBvh["data"] = ::udm::compress_lz4_blob(view.data(), view.size());
			static_assert(sizeof(decltype(boneMeshInfo->meshBvhTree->primitives[0])) == sizeof(Vector3) * 3);
			auto numVerts = boneMeshInfo->meshBvhTree->primitives.size() * 3;
			auto *verts = reinterpret_cast<Vector3 *>(boneMeshInfo->meshBvhTree->primitives.data());
			udmBvh.AddArray<Vector3>("primitives", numVerts, verts, udm::ArrayType::Compressed);

			++idx;
		}
	}
}

pragma::bvh::HitboxMeshBvhBuilderManager::HitboxMeshBvhBuilderManager() : m_threadPool {10} {}

std::optional<std::future<void>> pragma::bvh::HitboxMeshBvhBuilderManager::BuildModel(Model &mdl)
{
	HitboxMeshBvhBuilder builder {m_threadPool};
	builder.Generate(mdl);
	return {};
}
