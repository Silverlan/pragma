// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:entities.components.model;

export import :entities.components.entity;
export import :model.render_mesh_group;
import :rendering.material_property_block;
export import :rendering.model_render_buffer_data;
export import :rendering.shaders.textured_enums;

export class CModelSubMesh;
export namespace pragma {
	class CCameraComponent;
	class CSceneComponent;
	class CMaterialOverrideComponent;
};
export namespace pragma {
	namespace cModelComponent {
		using namespace baseModelComponent;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_RENDER_MESHES_UPDATED;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED;
	}
	class DLLCLIENT CModelComponent final : public BaseModelComponent, public CBaseNetComponent {
	  public:

		enum class StateFlags : uint8_t {
			None = 0u,
			AutoLodDisabled = 1u,
			RenderMeshUpdateRequired = AutoLodDisabled << 1u,
			RenderBufferListUpdateRequired = RenderMeshUpdateRequired << 1u,
			DepthPrepassDisabled = RenderBufferListUpdateRequired << 1u,
		};

		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CModelComponent(pragma::ecs::BaseEntity &ent);

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void Initialize() override;

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		msys::CMaterial *GetRenderMaterial(uint32_t idx) const;
		msys::CMaterial *GetRenderMaterial(uint32_t idx, uint32_t skin) const;

		CMaterialOverrideComponent *GetMaterialOverrideComponent();
		const CMaterialOverrideComponent *GetMaterialOverrideComponent() const { return const_cast<CModelComponent *>(this)->GetMaterialOverrideComponent(); }

		void SetRenderBufferListUpdateRequired();

		bool IsWeighted() const;

		void UpdateLOD(uint32_t lod);
		uint32_t GetLOD() const;
		void UpdateLOD(const CSceneComponent &scene, const CCameraComponent &cam, const Mat4 &vp);

		std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes() const;
		std::vector<std::shared_ptr<pragma::ModelSubMesh>> &GetRenderMeshes();
		const std::vector<std::shared_ptr<pragma::ModelSubMesh>> &GetRenderMeshes() const;
		const std::shared_ptr<prosper::IRenderBuffer> &GetRenderBuffer(uint32_t idx) const;
		const rendering::RenderBufferData *GetRenderBufferData(uint32_t idx) const;
		pragma::GameShaderSpecializationConstantFlag GetPipelineSpecializationFlags(uint32_t idx) const;
		const std::vector<rendering::RenderBufferData> &GetRenderBufferData() const { return const_cast<CModelComponent *>(this)->GetRenderBufferData(); };
		std::vector<rendering::RenderBufferData> &GetRenderBufferData() { return m_lodMeshRenderBufferData; }
		void SetRenderBufferData(const std::vector<rendering::RenderBufferData> &renderBufferData);
		void AddRenderMesh(CModelSubMesh &mesh, msys::CMaterial &mat, pragma::rendering::RenderBufferData::StateFlags stateFlags = pragma::rendering::RenderBufferData::StateFlags::EnableDepthPrepass);

		RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod);
		const RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod) const;
		RenderMeshGroup &GetLodMeshGroup(uint32_t lod);
		const RenderMeshGroup &GetLodMeshGroup(uint32_t lod) const;

		using BaseModelComponent::SetBodyGroup;
		using BaseModelComponent::SetModel;
		virtual bool SetBodyGroup(uint32_t groupId, uint32_t id) override;

		void SetAutoLodEnabled(bool enabled);
		bool IsAutoLodEnabled() const;

		void GetBaseModelMeshes(std::vector<std::shared_ptr<ModelMesh>> &outMeshes, uint32_t lod = 0) const;
		void SetRenderMeshesDirty();

		GameShaderSpecializationConstantFlag GetBaseShaderSpecializationFlags() const { return m_baseShaderSpecializationConstantFlags; }
		void SetBaseShaderSpecializationFlags(pragma::GameShaderSpecializationConstantFlag flags) { m_baseShaderSpecializationConstantFlags = flags; }
		void SetBaseShaderSpecializationFlag(pragma::GameShaderSpecializationConstantFlag flag, bool enabled = true);

		GameShaderSpecializationConstantFlag GetStaticShaderSpecializationFlags() const { return m_staticShaderSpecializationConstantFlags; }
		void SetStaticShaderSpecializationFlags(GameShaderSpecializationConstantFlag flags)
		{
			m_staticShaderSpecializationConstantFlags = flags;
			UpdateBaseShaderSpecializationFlags();
		}

		bool IsDepthPrepassEnabled() const;
		void SetDepthPrepassEnabled(bool enabled);

		void SetLightmapUvBuffer(const CModelSubMesh &mesh, const std::shared_ptr<prosper::IBuffer> &buffer);
		std::shared_ptr<prosper::IBuffer> GetLightmapUvBuffer(const CModelSubMesh &mesh) const;

		virtual void OnTick(double tDelta) override;
		void FlushRenderData();

		void UpdateRenderBufferList();
		void UpdateRenderMeshes(bool requireBoundingVolumeUpdate = true);
		void ReloadRenderBufferList(bool immediate = false);
		// Only use if LOD is handled externally!
		void SetLOD(uint32_t lod);
	  protected:
		void UpdateBaseShaderSpecializationFlags();
		virtual void OnModelChanged(const std::shared_ptr<pragma::Model> &model) override;

		std::unordered_map<const CModelSubMesh *, std::shared_ptr<prosper::IBuffer>> m_lightmapUvBuffers {};
		uint32_t m_lod = 0u;
		float m_tNextLodUpdate = 0.f;
		float m_lastLodCamDistance = 0.f;
		StateFlags m_stateFlags = StateFlags::None;
		std::vector<rendering::RenderBufferData> m_lodMeshRenderBufferData;
		std::vector<std::shared_ptr<ModelMesh>> m_lodMeshes;
		std::vector<std::shared_ptr<pragma::ModelSubMesh>> m_lodRenderMeshes;
		pragma::GameShaderSpecializationConstantFlag m_baseShaderSpecializationConstantFlags;
		pragma::GameShaderSpecializationConstantFlag m_staticShaderSpecializationConstantFlags;

		CMaterialOverrideComponent *m_materialOverrideComponent = nullptr;

		std::vector<RenderMeshGroup> m_lodMeshGroups;
		std::vector<RenderMeshGroup> m_lodRenderMeshGroups;
	};

	struct DLLCLIENT CEOnRenderMeshesUpdated : public ComponentEvent {
		CEOnRenderMeshesUpdated(bool requireBoundingVolumeUpdate = true);
		virtual void PushArguments(lua::State *l) override;
		bool requireBoundingVolumeUpdate = true;
	};
	using namespace umath::scoped_enum::bitwise;
};
export {
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::CModelComponent::StateFlags> : std::true_type {};
	}
}
