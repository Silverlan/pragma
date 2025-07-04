// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_CHILD_COMPONENT_HPP__
#define __S_CHILD_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_child_component.hpp>

namespace pragma {
	class DLLSERVER SChildComponent final : public BaseChildComponent, public SBaseNetComponent {
	  public:
		SChildComponent(BaseEntity &ent) : BaseChildComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void OnParentChanged(BaseEntity *parent) override;
	};
};

#endif
