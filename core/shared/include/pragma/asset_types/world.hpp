/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __PRAGMA_ASSET_TYPES_WORLD_HPP__
#define __PRAGMA_ASSET_TYPES_WORLD_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/util/util_bsp_tree.hpp"
#include "pragma/game/game_resources.hpp"
#include "pragma/physics/transform.hpp"
#include <sharedutils/util_path.hpp>
#include <fsys/filesystem.h>
#include <mathutil/uvec.h>
#include <unordered_set>

#undef GetClassName

namespace uimg {class ImageBuffer;};
namespace pragma::asset
{
	struct DLLNETWORK Output
	{
		std::string name;
		std::string target;
		std::string input;
		std::string param;
		float delay = 0.f;
		int32_t times = -1;

		void Write(VFilePtrReal &f);
		void Read(VFilePtr &f);
	};

	class WorldData;
	class DLLNETWORK EntityData
		: public std::enable_shared_from_this<EntityData>
	{
	public:
		enum class Flags : uint64_t
		{
			None = 0u,
			ClientsideOnly = 1u
		};

		static std::shared_ptr<EntityData> Create();

		bool IsWorld() const;
		bool IsSkybox() const;
		bool IsClientSideOnly() const;
		void SetClassName(const std::string &className);
		void SetOrigin(const Vector3 &origin);
		void SetLeafData(uint32_t firstLeaf,uint32_t numLeaves);
		void SetKeyValue(const std::string &key,const std::string &value);
		void AddOutput(const Output &output);

		uint32_t GetMapIndex() const;
		const std::string &GetClassName() const;
		Flags GetFlags() const;
		void SetFlags(Flags flags);
		const std::vector<std::string> &GetComponents() const;
		std::vector<std::string> &GetComponents();
		const std::unordered_map<std::string,std::string> &GetKeyValues() const;
		std::unordered_map<std::string,std::string> &GetKeyValues();
		std::optional<std::string> GetKeyValue(const std::string &key) const;
		std::string GetKeyValue(const std::string &key,const std::string &default) const;
		const std::vector<Output> &GetOutputs() const;
		std::vector<Output> &GetOutputs();
		const std::vector<uint16_t> &GetLeaves() const;
		std::vector<uint16_t> &GetLeaves();
		const Vector3 &GetOrigin() const;
		pragma::physics::Transform GetPose() const;
		void GetLeafData(uint32_t &outFirstLeaf,uint32_t &outNumLeaves) const;
	private:
		friend WorldData;
		EntityData()=default;
		std::string m_className;
		std::vector<std::string> m_components;
		std::unordered_map<std::string,std::string> m_keyValues;
		std::vector<Output> m_outputs;
		uint32_t m_mapIndex = 0u;
		Vector3 m_origin = {};
		std::vector<uint16_t> m_leaves = {};
		Flags m_flags = Flags::None;

		uint32_t m_firstLeaf = 0u;
		uint32_t m_numLeaves = 0u;
	};

	class DLLNETWORK WorldData
	{
	public:
		enum class DataFlags : uint64_t
		{
			None = 0u,
			HasLightmapAtlas = 1u,
			HasBSPTree = HasLightmapAtlas<<1u
		};

		using Edge = std::array<uint16_t,2>; // Vertex indices

		static std::shared_ptr<WorldData> Create(NetworkState &nw);
		static std::string GetLightmapAtlasTexturePath(const std::string &mapName);

		bool Write(const std::string &fileName,std::string *optOutErrMsg=nullptr);
		void Write(VFilePtrReal &f);
		bool Read(VFilePtr &f,EntityData::Flags entMask=EntityData::Flags::None,std::string *optOutErrMsg=nullptr);
		void AddEntity(EntityData &ent,bool isWorld=false);
		EntityData *FindWorld();
		void SetBSPTree(util::BSPTree &bspTree);
		util::BSPTree *GetBSPTree();
		std::vector<uint16_t> &GetStaticPropLeaves();
		NetworkState &GetNetworkState() const;

		void SetLightMapAtlas(uimg::ImageBuffer &imgAtlas);
		void SetLightMapEnabled(bool enabled);

		void SetLightMapIntensity(float intensity);
		void SetLightMapSqrtFactor(float factor);

		float GetLightMapIntensity() const;
		float GetLightMapSqrtFactor() const;

		const std::vector<std::shared_ptr<EntityData>> &GetEntities() const;
		const std::vector<std::string> &GetMaterialTable() const;
		std::vector<std::string> &GetMaterialTable();
		void SetMessageLogger(const std::function<void(const std::string&)> &msgLogger);
	private:
		WorldData(NetworkState &nw);
		void WriteDataOffset(VFilePtrReal &f,uint64_t offsetToOffset);
		void WriteMaterials(VFilePtrReal &f);
		void WriteBSPTree(VFilePtrReal &f);
		bool SaveLightmapAtlas(const std::string &mapName);
		void WriteEntities(VFilePtrReal &f);

		std::vector<MaterialHandle> ReadMaterials(VFilePtr &f);
		void ReadBSPTree(VFilePtr &f);
		void ReadEntities(VFilePtr &f,const std::vector<MaterialHandle> &materials,EntityData::Flags entMask);

		NetworkState &m_nw;
		std::shared_ptr<uimg::ImageBuffer> m_lightMapAtlas = nullptr;
		bool m_lightMapAtlasEnabled = false;
		float m_lightMapIntensity = 1.f;
		float m_lightMapSqrtFactor = 0.f;
		std::vector<uint16_t> m_staticPropLeaves {};
		std::vector<std::shared_ptr<EntityData>> m_entities {};
		std::vector<std::string> m_materialTable {};
		std::function<void(const std::string&)> m_messageLogger = nullptr;
		std::shared_ptr<util::BSPTree> m_bspTree = nullptr;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::asset::EntityData::Flags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::asset::WorldData::DataFlags)

#endif
