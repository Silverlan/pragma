/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LOGIC_RELAY_H__
#define __S_LOGIC_RELAY_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/logic/logic_relay.h"

namespace pragma {
	class DLLSERVER SLogicRelayComponent final : public BaseLogicRelayComponent {
	  public:
		SLogicRelayComponent(BaseEntity &ent) : BaseLogicRelayComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		void Trigger(BaseEntity *activator);
	};
};

class DLLSERVER LogicRelay : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
