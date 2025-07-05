// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_BOT_H__
#define __S_BOT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/basebot.h"

namespace pragma {
	class DLLSERVER SBotComponent final : public BaseBotComponent {
	  public:
		SBotComponent(BaseEntity &ent) : BaseBotComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER Bot : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
