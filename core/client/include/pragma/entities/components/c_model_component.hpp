/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_MODEL_COMPONENT_HPP__
#define __C_MODEL_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_model_component.hpp>
#include <vector>
#include <optional>

namespace pragma
{
	class CCameraComponent;
	class CSceneComponent;
	class DLLCLIENT CModelComponent final
		: public BaseModelComponent,
		public CBaseNetComponent
	{
	public:
		static ComponentEventId EVENT_ON_UPDATE_LOD;
		static ComponentEventId EVENT_ON_UPDATE_LOD_BY_POS;
		static ComponentEventId EVENT_ON_RENDER_MESHES_UPDATED;

		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CModelComponent(BaseEntity &ent) : BaseModelComponent(ent) {}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void Initialize() override;

		void SetMaterialOverride(uint32_t idx,const std::string &matOverride);
		void SetMaterialOverride(uint32_t idx,CMaterial &mat);
		void ClearMaterialOverride(uint32_t idx);
		CMaterial *GetMaterialOverride(uint32_t idx) const;
		const std::vector<MaterialHandle> &GetMaterialOverrides() const;
		CMaterial *GetRenderMaterial(uint32_t idx) const;

		bool IsWeighted() const;

		void UpdateLOD(uint32_t lod);
		uint8_t GetLOD();
		void UpdateLOD(const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp);
		std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes() const;
		std::vector<std::shared_ptr<ModelSubMesh>> &GetRenderMeshes();
		const std::vector<std::shared_ptr<ModelSubMesh>> &GetRenderMeshes() const;
		using BaseModelComponent::SetBodyGroup;
		using BaseModelComponent::SetModel;
		virtual bool SetBodyGroup(uint32_t groupId,uint32_t id) override;

		// Only use if LOD is handled externally!
		void SetLOD(uint8_t lod);
	protected:
		virtual void OnModelChanged(const std::shared_ptr<Model> &model) override;

		std::vector<MaterialHandle> m_materialOverrides = {};
		uint8_t m_lod = 0u; // Current level of detail
		std::vector<std::shared_ptr<ModelMesh>> m_lodMeshes;
		std::vector<std::shared_ptr<ModelSubMesh>> m_renderMeshes;
	};

	// Events

	struct DLLCLIENT CEOnUpdateLOD
		: public ComponentEvent
	{
		CEOnUpdateLOD(uint32_t lod);
		virtual void PushArguments(lua_State *l) override;
		uint32_t lod;
	};

	struct DLLCLIENT CEOnUpdateLODByPos
		: public ComponentEvent
	{
		CEOnUpdateLODByPos(const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp);
		virtual void PushArguments(lua_State *l) override;
		const CSceneComponent &scene;
		const CCameraComponent &camera;
		const Mat4 &viewProjection;
	};
};

#endif
