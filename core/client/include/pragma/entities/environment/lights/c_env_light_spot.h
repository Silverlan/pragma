// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_LIGHT_SPOT_H__
#define __C_ENV_LIGHT_SPOT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include <pragma/entities/environment/lights/env_light_spot.h>
#include <pragma/util/mvpbase.h>

namespace pragma {
	class DLLCLIENT CLightSpotComponent final : public BaseEnvLightSpotComponent, public CBaseNetComponent, public MVPBias<1> {
	  public:
		CLightSpotComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SetConeStartOffset(float offset) override;
		virtual void OnEntitySpawn() override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void SetFieldAngleComponent(BaseFieldAngleComponent &c) override;
		void SetShadowDirty();

		void UpdateInnerConeAngle();
		void UpdateViewMatrices();
		void UpdateProjectionMatrix();
		void UpdateTransformMatrix();
	};
};

class DLLCLIENT CEnvLightSpot : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
