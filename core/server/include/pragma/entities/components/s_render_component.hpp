// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_RENDER_COMPONENT_HPP__
#define __S_RENDER_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_render_component.hpp>

namespace pragma {
	class DLLSERVER SRenderComponent final : public BaseRenderComponent, public SBaseNetComponent {
	  public:
		SRenderComponent(BaseEntity &ent) : BaseRenderComponent(ent) {}
		virtual void SetUnlit(bool b) override;
		virtual void SetCastShadows(bool b) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
