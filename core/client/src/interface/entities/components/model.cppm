// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.model;

export import :entities.components.entity;
export import :model.render_mesh_group;
import :rendering.material_property_block;
export import :rendering.model_render_buffer_data;
export import :rendering.shaders.textured_enums;

export namespace pragma {
	class CCameraComponent;
	class CSceneComponent;
	class CMaterialOverrideComponent;
	namespace geometry {
		class CModelSubMesh;
	}
};
export namespace pragma {
	namespace cModelComponent {
		using namespace baseModelComponent;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RENDER_MESHES_UPDATED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_GAME_SHADER_SPECIALIZATION_CONSTANT_FLAGS_UPDATED;
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

		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CModelComponent(ecs::BaseEntity &ent);

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void Initialize() override;

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		material::CMaterial *GetRenderMaterial(uint32_t idx) const;
		material::CMaterial *GetRenderMaterial(uint32_t idx, uint32_t skin) const;

		CMaterialOverrideComponent *GetMaterialOverrideComponent();
		const CMaterialOverrideComponent *GetMaterialOverrideComponent() const { return const_cast<CModelComponent *>(this)->GetMaterialOverrideComponent(); }

		void SetRenderBufferListUpdateRequired();

		bool IsWeighted() const;

		void UpdateLOD(uint32_t lod);
		uint32_t GetLOD() const;
		void UpdateLOD(const CSceneComponent &scene, const CCameraComponent &cam, const Mat4 &vp);

		std::vector<std::shared_ptr<geometry::ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<geometry::ModelMesh>> &GetLODMeshes() const;
		std::vector<std::shared_ptr<geometry::ModelSubMesh>> &GetRenderMeshes();
		const std::vector<std::shared_ptr<geometry::ModelSubMesh>> &GetRenderMeshes() const;
		const std::shared_ptr<prosper::IRenderBuffer> &GetRenderBuffer(uint32_t idx) const;
		const rendering::RenderBufferData *GetRenderBufferData(uint32_t idx) const;
		GameShaderSpecializationConstantFlag GetPipelineSpecializationFlags(uint32_t idx) const;
		const std::vector<rendering::RenderBufferData> &GetRenderBufferData() const { return const_cast<CModelComponent *>(this)->GetRenderBufferData(); };
		std::vector<rendering::RenderBufferData> &GetRenderBufferData() { return m_lodMeshRenderBufferData; }
		void SetRenderBufferData(const std::vector<rendering::RenderBufferData> &renderBufferData);
		void AddRenderMesh(geometry::CModelSubMesh &mesh, material::CMaterial &mat, rendering::RenderBufferData::StateFlags stateFlags = rendering::RenderBufferData::StateFlags::EnableDepthPrepass);

		rendering::RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod);
		const rendering::RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod) const;
		rendering::RenderMeshGroup &GetLodMeshGroup(uint32_t lod);
		const rendering::RenderMeshGroup &GetLodMeshGroup(uint32_t lod) const;

		using BaseModelComponent::SetBodyGroup;
		using BaseModelComponent::SetModel;
		virtual bool SetBodyGroup(uint32_t groupId, uint32_t id) override;

		void SetAutoLodEnabled(bool enabled);
		bool IsAutoLodEnabled() const;

		void GetBaseModelMeshes(std::vector<std::shared_ptr<geometry::ModelMesh>> &outMeshes, uint32_t lod = 0) const;
		void SetRenderMeshesDirty();

		GameShaderSpecializationConstantFlag GetBaseShaderSpecializationFlags() const { return m_baseShaderSpecializationConstantFlags; }
		void SetBaseShaderSpecializationFlags(GameShaderSpecializationConstantFlag flags) { m_baseShaderSpecializationConstantFlags = flags; }
		void SetBaseShaderSpecializationFlag(GameShaderSpecializationConstantFlag flag, bool enabled = true);

		GameShaderSpecializationConstantFlag GetStaticShaderSpecializationFlags() const { return m_staticShaderSpecializationConstantFlags; }
		void SetStaticShaderSpecializationFlags(GameShaderSpecializationConstantFlag flags)
		{
			m_staticShaderSpecializationConstantFlags = flags;
			UpdateBaseShaderSpecializationFlags();
		}

		bool IsDepthPrepassEnabled() const;
		void SetDepthPrepassEnabled(bool enabled);

		void SetLightmapUvBuffer(const geometry::CModelSubMesh &mesh, const std::shared_ptr<prosper::IBuffer> &buffer);
		std::shared_ptr<prosper::IBuffer> GetLightmapUvBuffer(const geometry::CModelSubMesh &mesh) const;

		virtual void OnTick(double tDelta) override;
		void FlushRenderData();

		void UpdateRenderBufferList();
		void UpdateRenderMeshes(bool requireBoundingVolumeUpdate = true);
		void ReloadRenderBufferList(bool immediate = false);
		// Only use if LOD is handled externally!
		void SetLOD(uint32_t lod);
	  protected:
		void UpdateBaseShaderSpecializationFlags();
		virtual void OnModelChanged(const std::shared_ptr<asset::Model> &model) override;

		std::unordered_map<const geometry::CModelSubMesh *, std::shared_ptr<prosper::IBuffer>> m_lightmapUvBuffers {};
		uint32_t m_lod = 0u;
		float m_tNextLodUpdate = 0.f;
		float m_lastLodCamDistance = 0.f;
		StateFlags m_stateFlags = StateFlags::None;
		std::vector<rendering::RenderBufferData> m_lodMeshRenderBufferData;
		std::vector<std::shared_ptr<geometry::ModelMesh>> m_lodMeshes;
		std::vector<std::shared_ptr<geometry::ModelSubMesh>> m_lodRenderMeshes;
		GameShaderSpecializationConstantFlag m_baseShaderSpecializationConstantFlags;
		GameShaderSpecializationConstantFlag m_staticShaderSpecializationConstantFlags;

		CMaterialOverrideComponent *m_materialOverrideComponent = nullptr;

		std::vector<rendering::RenderMeshGroup> m_lodMeshGroups;
		std::vector<rendering::RenderMeshGroup> m_lodRenderMeshGroups;
	};

	struct DLLCLIENT CEOnRenderMeshesUpdated : public ComponentEvent {
		CEOnRenderMeshesUpdated(bool requireBoundingVolumeUpdate = true);
		virtual void PushArguments(lua::State *l) override;
		bool requireBoundingVolumeUpdate = true;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::CModelComponent::StateFlags)
}
