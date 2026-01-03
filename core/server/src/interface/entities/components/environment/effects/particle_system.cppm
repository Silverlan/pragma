// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.effects.particle_system;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SParticleSystemComponent final : public BaseEnvParticleSystemComponent, public SBaseNetComponent {
		  public:
			SParticleSystemComponent(ecs::BaseEntity &ent) : BaseEnvParticleSystemComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void SetContinuous(bool b) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvParticleSystem : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
