/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_FUNC_BRUSH_H__
#define __S_FUNC_BRUSH_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/func/basefuncbrush.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SBrushComponent final
		: public BaseFuncBrushComponent,
		public SBaseNetComponent
	{
	public:
		SBrushComponent(BaseEntity &ent) : BaseFuncBrushComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER FuncBrush
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif