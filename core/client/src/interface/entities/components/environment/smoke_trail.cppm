// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/effects/env_smoke_trail.h"
#include "pragma/entities/components/c_entity_component.hpp"

export module pragma.client.entities.components.effects.smoke_trail;

import pragma.client.entities.components.particle_system;

export namespace pragma {
	class DLLCLIENT CSmokeTrailComponent final : public BaseEnvSmokeTrailComponent, public CBaseNetComponent {
	  public:
		CSmokeTrailComponent(BaseEntity &ent) : BaseEnvSmokeTrailComponent(ent) {}
		virtual ~CSmokeTrailComponent() override;
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
	  protected:
		ComponentHandle<ecs::CParticleSystemComponent> m_hParticle = {};
		void InitializeParticle();
		void DestroyParticle();
	};
};

export class DLLCLIENT CEnvSmokeTrail : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
