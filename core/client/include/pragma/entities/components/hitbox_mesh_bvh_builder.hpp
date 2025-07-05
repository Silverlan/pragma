// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __HITBOX_MESH_BVH_BUILDER_HPP__
#define __HITBOX_MESH_BVH_BUILDER_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/util/util_thread_pool.hpp>
#include <sharedutils/BS_thread_pool.hpp>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

class Model;
struct LODInfo;
struct Hitbox;
namespace pragma::bvh {
	struct MeshBvhTree;
	class DLLCLIENT HitboxMeshBvhBuildTask {
	  public:
		using BoneName = std::string;
		struct DLLCLIENT BoneMeshInfo {
			std::string meshUuid;
			std::shared_ptr<ModelSubMesh> subMesh;
			std::vector<uint32_t> usedTris;
			std::unique_ptr<pragma::bvh::MeshBvhTree> meshBvhTree {};

			std::future<bool> genResult;
			std::stringstream serializedBvh;
		};

		HitboxMeshBvhBuildTask(BS::thread_pool &threadPool);
		bool Build(Model &mdl);
		const std::unordered_map<BoneName, std::vector<std::shared_ptr<BoneMeshInfo>>> &GetResult() const { return m_boneMeshMap; }
	  private:
		bool Build(Model &mdl, pragma::animation::BoneId boneId, const Hitbox &hitbox, const LODInfo &lodInfo);
		void Serialize(Model &mdl);
		void BuildHitboxMesh(Model &mdl, ModelSubMesh &subMesh);
		void BuildMeshBvh(Model &mdl, ModelSubMesh &subMesh);

		BS::thread_pool &m_threadPool;
		std::unordered_map<BoneName, std::vector<std::shared_ptr<BoneMeshInfo>>> m_boneMeshMap;
	};

	class DLLCLIENT HitboxMeshBvhBuilder {
	  public:
		HitboxMeshBvhBuilder();
		HitboxMeshBvhBuildTask BuildModel(Model &mdl);
		BS::thread_pool &GetThreadPool() { return m_threadPool; }
	  private:
		BS::thread_pool m_threadPool;
	};
};

#endif
