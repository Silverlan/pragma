// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <material_manager2.hpp>
#include <pragma/entities/environment/lights/env_light_spot_vol.h>

export module pragma.client:entities.components.lights.spot_vol;

import :entities.base_entity;
import :entities.components.entity;
import :model.mesh;

export namespace pragma {
	class DLLCLIENT CLightSpotVolComponent final : public BaseEnvLightSpotVolComponent, public CBaseNetComponent {
	  public:
		CLightSpotVolComponent(BaseEntity &ent) : BaseEnvLightSpotVolComponent(ent) {}
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
	  protected:
		float CalcEndRadius() const;
		uint32_t CalcSegmentCount() const;
		bool UpdateMeshData();
		void InitializeVolumetricLight();
		virtual bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		std::vector<std::shared_ptr<CModelSubMesh>> m_subMeshes;
		std::shared_ptr<Model> m_model;
		msys::MaterialHandle m_material;
	};
};

export class DLLCLIENT CEnvLightSpotVol : public CBaseEntity {
	virtual void Initialize() override;
};
