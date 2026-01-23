// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:map.world_data;

export import :map.entity_data;
export import :util.bsp_tree;
export import pragma.image;
import pragma.materialsystem;

export {
	namespace pragma {
		class NetworkState;
	}
	namespace pragma::asset {
		using WorldModelMeshIndex = uint32_t;
		class DLLNETWORK WorldData {
		  public:
			static constexpr udm::Version PMAP_VERSION = 2;
			static constexpr auto PMAP_IDENTIFIER = "PMAP";
			static constexpr auto PMAP_EXTENSION_BINARY = "pmap_b";
			static constexpr auto PMAP_EXTENSION_ASCII = "pmap";
			enum class DataFlags : uint64_t {
				None = 0u,
				HasLightmapAtlas = 1u,
				HasBSPTree = HasLightmapAtlas << 1u,
			};

			using Edge = std::array<uint16_t, 2>; // Vertex indices

			static std::shared_ptr<WorldData> Create(NetworkState &nw);
			static std::string GetLightmapAtlasTexturePath(const std::string &mapName);

			static std::shared_ptr<WorldData> load(NetworkState &nw, const std::string &fileName, std::string &outErr, EntityData::Flags entMask = EntityData::Flags::None);
			static std::shared_ptr<WorldData> load_from_udm_data(NetworkState &nw, udm::LinkedPropertyWrapper &prop, std::string &outErr, EntityData::Flags entMask = EntityData::Flags::None);
			static const std::vector<std::string> &get_supported_extensions();

			bool Write(const std::string &fileName, std::string *optOutErrMsg = nullptr);
			void Write(fs::VFilePtrReal &f);
			bool Read(fs::VFilePtr &f, EntityData::Flags entMask = EntityData::Flags::None, std::string *optOutErrMsg = nullptr);
			void AddEntity(EntityData &ent, bool isWorld = false);
			EntityData *FindWorld();
			void SetBSPTree(util::BSPTree &bspTree);
			util::BSPTree *GetBSPTree();
			std::vector<std::vector<WorldModelMeshIndex>> &GetClusterMeshIndices() { return m_meshesPerCluster; }
			std::vector<uint16_t> &GetStaticPropLeaves();
			NetworkState &GetNetworkState() const;

			void SetLightMapAtlas(image::ImageBuffer &imgAtlas);
			void SetLightMapEnabled(bool enabled);
			bool IsLegacyLightMapEnabled() const;

			void SetLightMapIntensity(float intensity);
			void SetLightMapExposure(float exp);

			float GetLightMapIntensity() const;
			float GetLightMapExposure() const;

			udm::Version GetVersion() const { return m_version; }
			void SetVersion(udm::Version version) { m_version = version; }

			const std::vector<std::shared_ptr<EntityData>> &GetEntities() const;
			const std::vector<std::string> &GetMaterialTable() const;
			std::vector<std::string> &GetMaterialTable();
			void SetMessageLogger(const std::function<void(const std::string &)> &msgLogger);

			bool Save(udm::AssetDataArg outData, const std::string &mapName, std::string &outErr);
			bool Save(const std::string &fileName, const std::string &mapName, std::string &outErr);
			bool LoadFromAssetData(udm::AssetDataArg data, EntityData::Flags entMask, std::string &outErr);
		  private:
			WorldData(NetworkState &nw);
			void WriteDataOffset(fs::VFilePtrReal &f, uint64_t offsetToOffset);
			void WriteMaterials(fs::VFilePtrReal &f);
			void WriteBSPTree(fs::VFilePtrReal &f);
			bool SaveLightmapAtlas(const std::string &mapName);
			void WriteEntities(fs::VFilePtrReal &f);

			bool LoadFromAssetData(udm::LinkedPropertyWrapper &prop, EntityData::Flags entMask, std::string &outErr);
			std::vector<material::MaterialHandle> ReadMaterials(fs::VFilePtr &f);
			void ReadBSPTree(fs::VFilePtr &f, uint32_t version);
			void ReadEntities(fs::VFilePtr &f, const std::vector<material::MaterialHandle> &materials, EntityData::Flags entMask);

			NetworkState &m_nw;
			std::vector<std::vector<WorldModelMeshIndex>> m_meshesPerCluster;
			std::shared_ptr<image::ImageBuffer> m_lightMapAtlas = nullptr;
			bool m_lightMapAtlasEnabled = false;
			float m_lightMapIntensity = 1.f;
			float m_lightMapExposure = 0.f;
			std::vector<uint16_t> m_staticPropLeaves {};
			std::vector<std::shared_ptr<EntityData>> m_entities {};
			std::vector<std::string> m_materialTable {};
			std::function<void(const std::string &)> m_messageLogger = nullptr;
			std::shared_ptr<util::BSPTree> m_bspTree = nullptr;
			udm::Version m_version = PMAP_VERSION;

			bool m_useLegacyLightmapDefinition = false;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::WorldData &worldData);
	REGISTER_ENUM_FLAGS(pragma::asset::WorldData::DataFlags)
};
