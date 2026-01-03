// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.hitbox_mesh_bvh_builder;

export import pragma.shared;

export namespace pragma::bvh {
	class DLLCLIENT HitboxMeshBvhBuildTask {
	  public:
		using BoneName = std::string;
		struct DLLCLIENT BoneMeshInfo {
			std::string meshUuid;
			std::shared_ptr<geometry::ModelSubMesh> subMesh;
			std::vector<uint32_t> usedTris;
			std::unique_ptr<MeshBvhTree> meshBvhTree {};

			std::future<bool> genResult;
			std::stringstream serializedBvh;
		};

		HitboxMeshBvhBuildTask(BS::light_thread_pool &threadPool);
		bool Build(asset::Model &mdl);
		const std::unordered_map<BoneName, std::vector<std::shared_ptr<BoneMeshInfo>>> &GetResult() const { return m_boneMeshMap; }
	  private:
		bool Build(asset::Model &mdl, animation::BoneId boneId, const physics::Hitbox &hitbox, const asset::LODInfo &lodInfo);
		void Serialize(asset::Model &mdl);
		void BuildHitboxMesh(asset::Model &mdl, geometry::ModelSubMesh &subMesh);
		void BuildMeshBvh(asset::Model &mdl, geometry::ModelSubMesh &subMesh);

		BS::light_thread_pool &m_threadPool;
		std::unordered_map<BoneName, std::vector<std::shared_ptr<BoneMeshInfo>>> m_boneMeshMap;
	};

	class DLLCLIENT HitboxMeshBvhBuilder {
	  public:
		HitboxMeshBvhBuilder();
		HitboxMeshBvhBuildTask BuildModel(asset::Model &mdl);
		BS::light_thread_pool &GetThreadPool() { return m_threadPool; }
	  private:
		BS::light_thread_pool m_threadPool;
	};
};
