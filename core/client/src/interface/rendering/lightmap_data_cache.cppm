// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.lightmap_data_cache;

export import pragma.udm;

export namespace pragma::rendering {
	struct DLLCLIENT LmUuid {
		util::Uuid uuid;
		bool operator==(const LmUuid &other) const { return uuid == other.uuid; }
	};
};
export namespace std {
	template<>
	struct hash<pragma::rendering::LmUuid> {
		std::size_t operator()(const pragma::rendering::LmUuid &k) const
		{
			using std::hash;
			using std::size_t;
			using std::string;

			pragma::util::Hash h = 0;
			h = pragma::util::hash_combine<size_t>(h, std::hash<int64_t> {}(k.uuid[0]));
			h = pragma::util::hash_combine<size_t>(h, std::hash<int64_t> {}(k.uuid[1]));
			return h;
		}
	};
}

export namespace pragma::rendering {
	struct DLLCLIENT LightmapDataCache : public std::enable_shared_from_this<LightmapDataCache> {
		static constexpr auto PLMD_IDENTIFIER = "PLMD";
		static constexpr udm::Version PLMD_VERSION = 1;
		static constexpr auto FORMAT_MODEL_BINARY = "lmd_b";
		static constexpr auto FORMAT_MODEL_ASCII = "lmd";
		static size_t CalcPoseHash(const math::Transform &pose);
		static bool Load(const std::string &path, LightmapDataCache &outCache, std::string &outErr);
		static std::string GetCacheFileName(const std::string &path);
		struct MeshCacheData {
			std::vector<Vector2> uvs;
		};
		struct InstanceCacheData {
			std::string model;    // Needed for debugging purposes only
			math::Transform pose; // Needed for debugging purposes only
			util::Uuid entityUuid;
			std::unordered_map<LmUuid, MeshCacheData> meshData;
		};
		std::unordered_map<LmUuid, InstanceCacheData> cacheData;
		util::Uuid lightmapEntityId {};
		void AddInstanceData(const util::Uuid &entUuid, const std::string &model, const math::Transform &pose, const util::Uuid &meshUuid, std::vector<Vector2> &&uvs);
		const std::vector<Vector2> *FindLightmapUvs(const util::Uuid &entUuid, const util::Uuid &meshUuid) const;
		bool Save(udm::AssetDataArg outData, std::string &outErr) const;
		bool SaveAs(const std::string &path, std::string &outErr) const;
		bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
	};
};
