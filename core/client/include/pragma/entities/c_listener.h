/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LISTENER_H__
#define __C_LISTENER_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/lua/c_listener_handle.hpp"

namespace al {class Listener;};
namespace pragma
{
	class DLLCLIENT CListenerComponent final
		: public BaseEntityComponent
	{
	public:
		CListenerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		CListenerComponent();
		virtual void Initialize() override;
		float GetGain();
		void SetGain(float gain);
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	private:
		al::Listener *m_listener = nullptr;
	};
};

class DLLCLIENT CListener
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif