// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.client:entities.components.light_map_receiver;

export import :entities.base_entity;
export import :entities.components.light_map;
export import :entities.components.render;
export import :model.mesh;
export import :rendering.lightmap_data_cache;

export namespace pragma {
	class DLLCLIENT CLightMapReceiverComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void SetupLightMapUvData(ecs::CBaseEntity &ent, LightmapDataCache *cache = nullptr);
		enum class StateFlags : uint8_t { None = 0, IsModelBakedWithLightMaps = 1u, RenderMeshBufferIndexTableDirty = IsModelBakedWithLightMaps << 1u };
		using MeshIdx = uint32_t;
		using BufferIdx = uint32_t;

		CLightMapReceiverComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;

		void UpdateLightMapUvData();
		const std::unordered_map<MeshIdx, std::vector<Vector2>> &GetMeshLightMapUvData() const;
		void AssignBufferIndex(MeshIdx meshIdx, BufferIdx bufIdx);
		std::optional<BufferIdx> FindBufferIndex(pragma::geometry::CModelSubMesh &mesh) const;
		std::optional<BufferIdx> GetBufferIndex(RenderMeshIndex meshIdx) const;

		void UpdateMeshLightmapUvBuffers(CLightMapComponent &lightMapC);

		const LightmapDataCache *GetLightmapDataCache() const;
		void SetLightmapDataCache(LightmapDataCache *cache);
	  protected:
		const std::vector<Vector2> *FindLightmapUvSet(pragma::geometry::ModelSubMesh &mesh) const;
		void UpdateModelMeshes();
		void UpdateRenderMeshBufferList();

		std::shared_ptr<LightmapDataCache> m_lightmapDataCache;
		std::unordered_map<MeshIdx, std::vector<Vector2>> m_uvDataPerMesh {};
		std::unordered_map<MeshIdx, std::shared_ptr<pragma::geometry::ModelSubMesh>> m_meshes {};
		std::unordered_map<pragma::geometry::CModelSubMesh *, MeshIdx> m_meshToMeshIdx {};
		std::unordered_map<pragma::geometry::CModelSubMesh *, BufferIdx> m_meshToBufIdx {};

		// Matches the render meshes from the model component (for faster lookup)
		std::vector<BufferIdx> m_meshBufferIndices;

		std::string m_modelName; // Uvs are only valid for the model they were built with
		StateFlags m_stateFlags = StateFlags::RenderMeshBufferIndexTableDirty;
	};
	using namespace umath::scoped_enum::bitwise;
};
export {REGISTER_ENUM_FLAGS(pragma::CLightMapReceiverComponent::StateFlags)}
