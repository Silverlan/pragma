// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_FLAMMABLE_COMPONENT_HPP__
#define __S_FLAMMABLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_flammable_component.hpp>
#include <pragma/util/timer_handle.h>

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
		SFlammableComponent(BaseEntity &ent) : BaseFlammableComponent(ent) {}
		virtual util::EventReply Ignite(float duration, BaseEntity *attacker = nullptr, BaseEntity *inflictor = nullptr) override;
		virtual void Extinguish() override;
		virtual void SetIgnitable(bool b) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		void ApplyIgnitionDamage();
	};
};

#endif
