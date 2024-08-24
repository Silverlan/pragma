/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_ASSET_TYPES_WORLD_HPP__
#define __PRAGMA_ASSET_TYPES_WORLD_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/util/util_bsp_tree.hpp"
#include "pragma/game/game_resources.hpp"
#include <mathutil/transform.hpp>
#include <sharedutils/util_path.hpp>
#include <fsys/filesystem.h>
#include <mathutil/uvec.h>
#include <unordered_set>
#include <udm.hpp>

#undef GetClassName

namespace uimg {
	class ImageBuffer;
};
namespace udm {
	struct AssetData;
};
namespace pragma::asset {
	struct DLLNETWORK Output {
		std::string name;
		std::string target;
		std::string input;
		std::string param;
		float delay = 0.f;
		int32_t times = -1;

		void Write(VFilePtrReal &f);
		void Read(VFilePtr &f);
	};

	class DLLNETWORK ComponentData : public std::enable_shared_from_this<ComponentData> {
	  public:
		enum class Flags : uint64_t {
			None = 0u,
			ClientsideOnly = 1u,
		};

		static std::shared_ptr<ComponentData> Create();

		Flags GetFlags() const;
		void SetFlags(Flags flags);
		udm::PProperty GetData() const { return m_data; }
	  private:
		ComponentData();
		udm::PProperty m_data;
		Flags m_flags = Flags::None;
	};

	class WorldData;
	class DLLNETWORK EntityData : public std::enable_shared_from_this<EntityData> {
	  public:
		enum class Flags : uint64_t { None = 0u, ClientsideOnly = 1u };

		static std::shared_ptr<EntityData> Create();

		bool IsWorld() const;
		bool IsSkybox() const;
		bool IsClientSideOnly() const;
		void SetClassName(const std::string &className);
		void SetLeafData(uint32_t firstLeaf, uint32_t numLeaves);
		void SetKeyValue(const std::string &key, const std::string &value);
		void AddOutput(const Output &output);

		uint32_t GetMapIndex() const;
		const std::string &GetClassName() const;
		Flags GetFlags() const;
		void SetFlags(Flags flags);
		std::shared_ptr<ComponentData> AddComponent(const std::string &name);
		const std::unordered_map<std::string, std::shared_ptr<ComponentData>> &GetComponents() const;
		std::unordered_map<std::string, std::shared_ptr<ComponentData>> &GetComponents();
		const std::unordered_map<std::string, std::string> &GetKeyValues() const;
		std::unordered_map<std::string, std::string> &GetKeyValues();
		std::optional<std::string> GetKeyValue(const std::string &key) const;
		std::string GetKeyValue(const std::string &key, const std::string &def) const;
		const std::vector<Output> &GetOutputs() const;
		std::vector<Output> &GetOutputs();
		const std::vector<uint16_t> &GetLeaves() const;
		std::vector<uint16_t> &GetLeaves();
		void GetLeafData(uint32_t &outFirstLeaf, uint32_t &outNumLeaves) const;

		const std::optional<umath::ScaledTransform> &GetPose() const;
		umath::ScaledTransform GetEffectivePose() const;
		void SetPose(const umath::ScaledTransform &pose);
		void ClearPose();
	  private:
		friend WorldData;
		EntityData() = default;
		std::string m_className = "entity";
		std::unordered_map<std::string, std::shared_ptr<ComponentData>> m_components;
		std::unordered_map<std::string, std::string> m_keyValues;
		std::vector<Output> m_outputs;
		uint32_t m_mapIndex = 0u;
		std::optional<umath::ScaledTransform> m_pose {};
		std::vector<uint16_t> m_leaves = {};
		Flags m_flags = Flags::None;

		uint32_t m_firstLeaf = 0u;
		uint32_t m_numLeaves = 0u;
	};

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
		void Write(VFilePtrReal &f);
		bool Read(VFilePtr &f, EntityData::Flags entMask = EntityData::Flags::None, std::string *optOutErrMsg = nullptr);
		void AddEntity(EntityData &ent, bool isWorld = false);
		EntityData *FindWorld();
		void SetBSPTree(util::BSPTree &bspTree);
		util::BSPTree *GetBSPTree();
		std::vector<std::vector<WorldModelMeshIndex>> &GetClusterMeshIndices() { return m_meshesPerCluster; }
		std::vector<uint16_t> &GetStaticPropLeaves();
		NetworkState &GetNetworkState() const;

		void SetLightMapAtlas(uimg::ImageBuffer &imgAtlas);
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
		void WriteDataOffset(VFilePtrReal &f, uint64_t offsetToOffset);
		void WriteMaterials(VFilePtrReal &f);
		void WriteBSPTree(VFilePtrReal &f);
		bool SaveLightmapAtlas(const std::string &mapName);
		void WriteEntities(VFilePtrReal &f);

		bool LoadFromAssetData(udm::LinkedPropertyWrapper &prop, EntityData::Flags entMask, std::string &outErr);
		std::vector<msys::MaterialHandle> ReadMaterials(VFilePtr &f);
		void ReadBSPTree(VFilePtr &f, uint32_t version);
		void ReadEntities(VFilePtr &f, const std::vector<msys::MaterialHandle> &materials, EntityData::Flags entMask);

		NetworkState &m_nw;
		std::vector<std::vector<WorldModelMeshIndex>> m_meshesPerCluster;
		std::shared_ptr<uimg::ImageBuffer> m_lightMapAtlas = nullptr;
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
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::asset::EntityData::Flags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::asset::WorldData::DataFlags)

DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::Output &output);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::ComponentData &componentData);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::EntityData &entityData);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::WorldData &worldData);

#endif
