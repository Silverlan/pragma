/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_MODEL_COMPONENT_HPP__
#define __C_MODEL_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/c_model_render_buffer_data.hpp"
#include <pragma/entities/components/base_model_component.hpp>
#include <vector>
#include <optional>

namespace pragma {
	enum class GameShaderSpecializationConstantFlag : uint32_t;
	enum class GameShaderSpecialization : uint32_t;
	class CCameraComponent;
	class CSceneComponent;
	class DLLCLIENT CModelComponent final : public BaseModelComponent, public CBaseNetComponent {
	  public:
		static ComponentEventId EVENT_ON_RENDER_MESHES_UPDATED;
		static ComponentEventId EVENT_ON_MATERIAL_OVERRIDES_CLEARED;

		enum class StateFlags : uint8_t { None = 0u, AutoLodDisabled = 1u, RenderMeshUpdateRequired = AutoLodDisabled << 1u, RenderBufferListUpdateRequired = RenderMeshUpdateRequired << 1u, DepthPrepassDisabled = RenderBufferListUpdateRequired << 1u };

		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CModelComponent(BaseEntity &ent);

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void Initialize() override;

		void SetMaterialOverride(uint32_t idx, const std::string &matOverride);
		void SetMaterialOverride(uint32_t idx, CMaterial &mat);
		void ClearMaterialOverride(uint32_t idx);
		void ClearMaterialOverrides();
		CMaterial *GetMaterialOverride(uint32_t idx) const;
		const std::vector<msys::MaterialHandle> &GetMaterialOverrides() const;
		CMaterial *GetRenderMaterial(uint32_t idx) const;
		CMaterial *GetRenderMaterial(uint32_t idx, uint32_t skin) const;

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		bool IsWeighted() const;

		void UpdateLOD(uint32_t lod);
		uint32_t GetLOD() const;
		void UpdateLOD(const CSceneComponent &scene, const CCameraComponent &cam, const Mat4 &vp);

		std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes() const;
		std::vector<std::shared_ptr<ModelSubMesh>> &GetRenderMeshes();
		const std::vector<std::shared_ptr<ModelSubMesh>> &GetRenderMeshes() const;
		const std::shared_ptr<prosper::IRenderBuffer> &GetRenderBuffer(uint32_t idx) const;
		const rendering::RenderBufferData *GetRenderBufferData(uint32_t idx) const;
		pragma::GameShaderSpecializationConstantFlag GetPipelineSpecializationFlags(uint32_t idx) const;
		const std::vector<rendering::RenderBufferData> &GetRenderBufferData() const { return const_cast<CModelComponent *>(this)->GetRenderBufferData(); };
		std::vector<rendering::RenderBufferData> &GetRenderBufferData() { return m_lodMeshRenderBufferData; }
		void SetRenderBufferData(const std::vector<rendering::RenderBufferData> &renderBufferData);
		void AddRenderMesh(CModelSubMesh &mesh, CMaterial &mat, bool enableDepthPrepass = true);

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

		bool IsDepthPrepassEnabled() const;
		void SetDepthPrepassEnabled(bool enabled);

		void SetLightmapUvBuffer(const CModelSubMesh &mesh, const std::shared_ptr<prosper::IBuffer> &buffer);
		std::shared_ptr<prosper::IBuffer> GetLightmapUvBuffer(const CModelSubMesh &mesh) const;

		void UpdateRenderMeshes();
		void ReloadRenderBufferList(bool immediate = false);
		// Only use if LOD is handled externally!
		void SetLOD(uint32_t lod);
	  protected:
		void UpdateBaseShaderSpecializationFlags();
		virtual void OnModelChanged(const std::shared_ptr<Model> &model) override;
		void UpdateRenderBufferList();

		std::unordered_map<const CModelSubMesh *, std::shared_ptr<prosper::IBuffer>> m_lightmapUvBuffers {};
		std::vector<msys::MaterialHandle> m_materialOverrides = {};
		uint32_t m_lod = 0u;
		float m_tNextLodUpdate = 0.f;
		float m_lastLodCamDistance = 0.f;
		StateFlags m_stateFlags = StateFlags::None;
		std::vector<rendering::RenderBufferData> m_lodMeshRenderBufferData;
		std::vector<std::shared_ptr<ModelMesh>> m_lodMeshes;
		std::vector<std::shared_ptr<ModelSubMesh>> m_lodRenderMeshes;
		pragma::GameShaderSpecializationConstantFlag m_baseShaderSpecializationConstantFlags;

		std::vector<RenderMeshGroup> m_lodMeshGroups;
		std::vector<RenderMeshGroup> m_lodRenderMeshGroups;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CModelComponent::StateFlags)

#endif
