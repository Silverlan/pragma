/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_FIRE_H__
#define __C_ENV_FIRE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/effects/env_fire.h"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma {
	class CParticleSystemComponent;
	class DLLCLIENT CFireComponent final : public BaseEnvFireComponent, public CBaseNetComponent {
	  public:
		CFireComponent(BaseEntity &ent) : BaseEnvFireComponent(ent) {}
		virtual ~CFireComponent() override;
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		ComponentHandle<CParticleSystemComponent> m_hParticle;
		void InitializeParticle();
		void DestroyParticle();
	};
};

class DLLCLIENT CEnvFire : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
