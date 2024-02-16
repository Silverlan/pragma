/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

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
	class DLLCLIENT HitboxMeshBvhBuilder {
	  public:
		struct DLLCLIENT BoneMeshInfo {
			std::string meshUuid;
			std::shared_ptr<ModelSubMesh> subMesh;
			std::vector<uint32_t> usedTris;
			std::unique_ptr<pragma::bvh::MeshBvhTree> meshBvhTree {};

			std::future<bool> genResult;
			std::stringstream serializedBvh;
		};

		HitboxMeshBvhBuilder(BS::thread_pool &threadPool);
		bool Generate(Model &mdl);
	  private:
		using BoneName = std::string;
		bool Generate(Model &mdl, BoneId boneId, const Hitbox &hitbox, const LODInfo &lodInfo);
		void Serialize(Model &mdl);
		void GenerateHitboxMesh(Model &mdl, ModelSubMesh &subMesh);
		void GenerateMeshBvh(Model &mdl, ModelSubMesh &subMesh);

		BS::thread_pool &m_threadPool;
		std::unordered_map<BoneName, std::vector<std::shared_ptr<BoneMeshInfo>>> m_boneMeshMap;
	};

	class DLLCLIENT HitboxMeshBvhBuilderManager {
	  public:
		HitboxMeshBvhBuilderManager();
		std::optional<std::future<void>> BuildModel(Model &mdl);
	  private:
		BS::thread_pool m_threadPool;
		//
	};
};

#endif
