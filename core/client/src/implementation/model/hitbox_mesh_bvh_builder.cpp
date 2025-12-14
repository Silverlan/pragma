// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <bvh/v2/stream.h>
#include <cassert>

module pragma.client;

import :model.hitbox_mesh_bvh_builder;

static spdlog::logger &LOGGER = pragma::register_logger("bvh");

static std::unique_ptr<pragma::bvh::MeshBvhTree> generate_mesh_bvh(pragma::geometry::ModelSubMesh &mesh, const std::vector<uint32_t> &triIndices, const pragma::math::ScaledTransform &invPose)
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

static bool generate_mesh_bvh(pragma::asset::Model &mdl, const std::string &boneName, pragma::bvh::HitboxMeshBvhBuildTask::BoneMeshInfo &boneMeshInfo)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &ref = mdl.GetReference();
	using BoneName = std::string;
	using MeshUuid = std::string;
	auto meshMap = pragma::bvh::get_uuid_mesh_map(mdl);
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId < 0)
		return false;
	pragma::math::ScaledTransform pose;
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

static bool calc_bone_mesh_info(pragma::bvh::HitboxMeshBvhBuildTask::BoneMeshInfo &boneMeshInfo, pragma::animation::BoneId boneId, std::shared_ptr<pragma::geometry::ModelSubMesh> subMesh, std::array<pragma::math::Plane, 6> planes, Vector3 hbMin, Vector3 hbMax, pragma::math::ScaledTransform pose, pragma::util::Uuid uuid)
{
	Vector3 smMin, smMax;
	subMesh->GetBounds(smMin, smMax);
	if(pragma::math::intersection::aabb_in_plane_mesh(smMin, smMax, planes.begin(), planes.end()) == pragma::math::intersection::Intersect::Outside)
		return false;
	auto &verts = subMesh->GetVertices();
	auto &vertWeights = subMesh->GetVertexWeights();
	auto numVerts = verts.size();
	if(vertWeights.size() != numVerts)
		return false;

	std::vector<uint32_t> usedTris;
	usedTris.reserve(subMesh->GetTriangleCount());
	subMesh->VisitIndices([&verts, &vertWeights, &usedTris, &hbMin, &hbMax, &pose, boneId](auto *indexDataSrc, uint32_t numIndicesSrc) {
		for(auto i = decltype(numIndicesSrc) {0u}; i < numIndicesSrc; i += 3) {
			auto idx0 = indexDataSrc[i];
			auto idx1 = indexDataSrc[i + 1];
			auto idx2 = indexDataSrc[i + 2];
			auto &v0 = verts[idx0];
			auto &v1 = verts[idx1];
			auto &v2 = verts[idx2];
			auto &vw0 = vertWeights[idx0];
			auto &vw1 = vertWeights[idx1];
			auto &vw2 = vertWeights[idx2];

			auto hasBone = false;
			for(uint8_t j = 0; j < 4; ++j) {
				for(auto &vw : {vw0, vw1, vw2}) {
					if(vw.boneIds[j] == boneId && vw.weights[j] > 0.5f) {
						hasBone = true;
						goto endLoop;
					}
				}
			}
		endLoop:
			if(!hasBone)
				continue;
			if(pragma::math::intersection::obb_triangle(hbMin, hbMax, pose, v0.position, v1.position, v2.position))
				usedTris.push_back(i / 3);
		}
	});

	if(usedTris.empty())
		return false;
	boneMeshInfo.subMesh = subMesh;
	boneMeshInfo.meshUuid = pragma::util::uuid_to_string(uuid);
	boneMeshInfo.usedTris = std::move(usedTris);
	return true;
}

static bool generate_bvh_mesh(pragma::asset::Model &mdl, const std::string &boneName, pragma::bvh::HitboxMeshBvhBuildTask::BoneMeshInfo &boneMeshInfo)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &ref = mdl.GetReference();
	using BoneName = std::string;
	using MeshUuid = std::string;
	auto meshMap = pragma::bvh::get_uuid_mesh_map(mdl);
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId < 0)
		return false;
	pragma::math::ScaledTransform pose;
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

static void serialize(pragma::asset::Model &mdl, const std::string &boneName, pragma::bvh::HitboxMeshBvhBuildTask::BoneMeshInfo &boneMeshInfo)
{
	bvh::v2::StdOutputStream outputStream {boneMeshInfo.serializedBvh};
	auto &bvh = boneMeshInfo.meshBvhTree->bvh;
	bvh.serialize(outputStream);
}

pragma::bvh::HitboxMeshBvhBuildTask::HitboxMeshBvhBuildTask(BS::light_thread_pool &threadPool) : m_threadPool {threadPool} {}

bool pragma::bvh::HitboxMeshBvhBuildTask::Build(asset::Model &mdl)
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
		Build(mdl, boneId, hb, lodLast);
	}

	Serialize(mdl);
	return true;
}

bool pragma::bvh::HitboxMeshBvhBuildTask::Build(asset::Model &mdl, animation::BoneId boneId, const physics::Hitbox &hb, const asset::LODInfo &lodInfo)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &ref = mdl.GetReference();
	auto bone = skeleton.GetBone(boneId).lock();
	if(!bone)
		return false;
	math::ScaledTransform pose;
	if(!ref.GetBonePose(boneId, pose))
		return false;
	auto &pos = pose.GetOrigin();
	auto &rot = pose.GetRotation();
	auto planes = math::geometry::get_obb_planes(pos, rot, hb.min, hb.max);
	auto &hbMin = hb.min;
	auto &hbMax = hb.max;
	for(auto &pair : lodInfo.meshReplacements) {
		auto mg = mdl.GetMeshGroup(pair.second);
		if(!mg)
			continue;
		for(auto &mesh : mg->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				if(!is_mesh_bvh_compatible(*subMesh))
					continue;
				auto &uuid = subMesh->GetUuid();
				if(uuid == util::Uuid {}) {
					LOGGER.warn("Mesh with invalid uuid in model '{}'! Skipping...", mdl.GetName());
					continue;
				}

				std::string boneName = bone->name;
				auto bm = pragma::util::make_shared<BoneMeshInfo>();
				auto genResult = m_threadPool.submit_task([&mdl, subMesh, planes, hbMin, hbMax, pose, uuid, boneName, bm, boneId]() -> bool {
					auto success = calc_bone_mesh_info(*bm, boneId, subMesh, planes, hbMin, hbMax, pose, uuid);
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

void pragma::bvh::HitboxMeshBvhBuildTask::Serialize(asset::Model &mdl)
{
	auto extData = mdl.GetExtensionData();
	auto ubmHitboxBvh = extData["hitboxBvh"];
	auto udmHbMeshes = ubmHitboxBvh["hitboxMeshes"];
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

			assert(!boneMeshInfo->serializedBvh.str().empty());
			auto &stream = boneMeshInfo->serializedBvh;
			auto view = stream.view();
			auto extData = boneMeshInfo->subMesh->GetExtensionData();
			auto udmBvh = udmBoneMeshes[idx]["bvh"];
			udmBvh["data"] = udm::compress_lz4_blob(view.data(), view.size());
			static_assert(sizeof(decltype(boneMeshInfo->meshBvhTree->primitives[0])) == sizeof(Vector3) * 3);
			auto numVerts = boneMeshInfo->meshBvhTree->primitives.size() * 3;
			auto *verts = reinterpret_cast<Vector3 *>(boneMeshInfo->meshBvhTree->primitives.data());
			udmBvh.AddArray<Vector3>("primitives", numVerts, verts, udm::ArrayType::Compressed);

			++idx;
		}
	}
}

pragma::bvh::HitboxMeshBvhBuilder::HitboxMeshBvhBuilder() : m_threadPool {15} {}

pragma::bvh::HitboxMeshBvhBuildTask pragma::bvh::HitboxMeshBvhBuilder::BuildModel(asset::Model &mdl)
{
	HitboxMeshBvhBuildTask task {m_threadPool};
	task.Build(mdl);
	return task;
}
