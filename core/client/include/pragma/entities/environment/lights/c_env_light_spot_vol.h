// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_LIGHT_SPOT_VOL_H__
#define __C_ENV_LIGHT_SPOT_VOL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <material_manager2.hpp>
#include <pragma/entities/environment/lights/env_light_spot_vol.h>

namespace pragma {
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

class DLLCLIENT CEnvLightSpotVol : public CBaseEntity {
	virtual void Initialize() override;
};

#endif
