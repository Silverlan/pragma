// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.flammable;

import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SFlammableComponent final : public BaseFlammableComponent, public SBaseNetComponent {
		  protected:
			struct DLLSERVER IgniteInfo {
				IgniteInfo();
				~IgniteInfo();
				void Clear();
				std::shared_ptr<TimerHandle> damageTimer;
				EntityHandle hAttacker;
				EntityHandle hInflictor;
			} m_igniteInfo;
		  public:
			SFlammableComponent(ecs::BaseEntity &ent) : BaseFlammableComponent(ent) {}
			virtual util::EventReply Ignite(float duration, ecs::BaseEntity *attacker = nullptr, ecs::BaseEntity *inflictor = nullptr) override;
			virtual void Extinguish() override;
			virtual void SetIgnitable(bool b) override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			void ApplyIgnitionDamage();
		};
	};
};
