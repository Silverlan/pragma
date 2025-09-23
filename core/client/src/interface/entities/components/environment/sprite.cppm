// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/environment/effects/env_sprite.h>

export module pragma.client:entities.components.effects.sprite;

import :entities.base_entity;
import :entities.components.entity;
import :particle_system.enums;

export namespace pragma::ecs {class CParticleSystemComponent;}
export namespace pragma {
	class DLLCLIENT CSpriteComponent final : public BaseEnvSpriteComponent, public CBaseNetComponent {
	  public:
		CSpriteComponent(BaseEntity &ent) : BaseEnvSpriteComponent(ent) {}
		virtual void Initialize() override;
		virtual ~CSpriteComponent() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnTick(double dt) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		void SetOrientationType(ecs::ParticleOrientationType orientationType);
		virtual void StartParticle();
		virtual void StopParticle();
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

		void StopAndRemoveEntity();
	  protected:
		void UpdateColor();
		ComponentHandle<ecs::CParticleSystemComponent> m_hParticle = {};
		CallbackHandle m_hCbRenderCallback;
		ecs::ParticleOrientationType m_orientationType = ecs::ParticleOrientationType::Aligned;
	};
};

export class DLLCLIENT CEnvSprite : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
