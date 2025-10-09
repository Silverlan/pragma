// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:entities.components.light_map_receiver;

import :entities.base_entity;
import :entities.components.light_map;
import :entities.components.render;
import :model.mesh;
import :rendering.lightmap_data_cache;

export namespace pragma {
	class DLLCLIENT CLightMapReceiverComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void SetupLightMapUvData(CBaseEntity &ent, LightmapDataCache *cache = nullptr);
		enum class StateFlags : uint8_t { None = 0, IsModelBakedWithLightMaps = 1u, RenderMeshBufferIndexTableDirty = IsModelBakedWithLightMaps << 1u };
		using MeshIdx = uint32_t;
		using BufferIdx = uint32_t;

		CLightMapReceiverComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		void UpdateLightMapUvData();
		const std::unordered_map<MeshIdx, std::vector<Vector2>> &GetMeshLightMapUvData() const;
		void AssignBufferIndex(MeshIdx meshIdx, BufferIdx bufIdx);
		std::optional<BufferIdx> FindBufferIndex(CModelSubMesh &mesh) const;
		std::optional<BufferIdx> GetBufferIndex(RenderMeshIndex meshIdx) const;

		void UpdateMeshLightmapUvBuffers(CLightMapComponent &lightMapC);

		const LightmapDataCache *GetLightmapDataCache() const;
		void SetLightmapDataCache(LightmapDataCache *cache);
	  protected:
		const std::vector<Vector2> *FindLightmapUvSet(ModelSubMesh &mesh) const;
		void UpdateModelMeshes();
		void UpdateRenderMeshBufferList();

		std::shared_ptr<LightmapDataCache> m_lightmapDataCache;
		std::unordered_map<MeshIdx, std::vector<Vector2>> m_uvDataPerMesh {};
		std::unordered_map<MeshIdx, std::shared_ptr<ModelSubMesh>> m_meshes {};
		std::unordered_map<CModelSubMesh *, MeshIdx> m_meshToMeshIdx {};
		std::unordered_map<CModelSubMesh *, BufferIdx> m_meshToBufIdx {};

		// Matches the render meshes from the model component (for faster lookup)
		std::vector<BufferIdx> m_meshBufferIndices;

		std::string m_modelName; // Uvs are only valid for the model they were built with
		StateFlags m_stateFlags = StateFlags::RenderMeshBufferIndexTableDirty;
	};
};
export {
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::CLightMapReceiverComponent::StateFlags)
};
