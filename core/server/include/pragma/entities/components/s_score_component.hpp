// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_SCORE_COMPONENT_HPP__
#define __S_SCORE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_score_component.hpp>

namespace pragma {
	class DLLSERVER SScoreComponent final : public BaseScoreComponent, public SBaseNetComponent {
	  public:
		SScoreComponent(BaseEntity &ent) : BaseScoreComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
