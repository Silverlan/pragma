/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_BOT_H__
#define __C_BOT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/basebot.h"
#include <pragma/entities/components/base_character_component.hpp>

namespace pragma
{
	class DLLCLIENT CBotComponent final
		: public BaseBotComponent
	{
	public:
		CBotComponent(BaseEntity &ent) : BaseBotComponent(ent) {}
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	protected:
		void OnFootStep(BaseCharacterComponent::FootType foot);
	};
};

class DLLCLIENT CBot
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif