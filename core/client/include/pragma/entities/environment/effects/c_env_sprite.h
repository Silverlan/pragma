/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_SPRITE_H__
#define __C_ENV_SPRITE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/environment/effects/env_sprite.h>

namespace pragma {
	class DLLCLIENT CSpriteComponent final : public BaseEnvSpriteComponent, public CBaseNetComponent {
	  public:
		CSpriteComponent(BaseEntity &ent) : BaseEnvSpriteComponent(ent) {}
		virtual void Initialize() override;
		virtual ~CSpriteComponent() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnTick(double dt) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		void SetOrientationType(CParticleSystemComponent::OrientationType orientationType);
		virtual void StartParticle();
		virtual void StopParticle();
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;

		void StopAndRemoveEntity();
	  protected:
		ComponentHandle<CParticleSystemComponent> m_hParticle = {};
		CallbackHandle m_hCbRenderCallback;
		CParticleSystemComponent::OrientationType m_orientationType = CParticleSystemComponent::OrientationType::Aligned;
	};
};

class DLLCLIENT CEnvSprite : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
