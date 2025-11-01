// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"





export module pragma.client:entities.components.flammable;

export import :entities.components.entity;

export namespace pragma::ecs {class CParticleSystemComponent;}
export namespace pragma {
	class DLLCLIENT CFlammableComponent final : public BaseFlammableComponent, public CBaseNetComponent {
	  public:
		CFlammableComponent(pragma::ecs::BaseEntity &ent) : BaseFlammableComponent(ent) {}
		virtual void Initialize() override;
		virtual util::EventReply Ignite(float duration, pragma::ecs::BaseEntity *attacker = nullptr, pragma::ecs::BaseEntity *inflictor = nullptr) override;
		virtual void Extinguish() override;
		virtual void OnTick(double dt) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		void UpdateFlameParticlePositions();
		struct DLLCLIENT IgniteInfo {
			IgniteInfo();
			~IgniteInfo();
			struct Particle {
				Particle(pragma::ecs::CParticleSystemComponent &pt, uint32_t boneId = 0);
				Particle(pragma::ecs::CParticleSystemComponent &pt, const Vector3 &offset);
				util::WeakHandle<pragma::ecs::CParticleSystemComponent> hParticle;
				uint32_t boneId;
				Vector3 offset;
			};
			void Clear();
			std::shared_ptr<ALSound> sound = nullptr;
			std::vector<Particle> flameParticles;
		} m_igniteInfo = {};
	};
};
