/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/baseluaobj.h"

BaseLuaObj::BaseLuaObj() : m_luaObj(nullptr), m_bExternalHandle(false) {}

BaseLuaObj::~BaseLuaObj() { m_luaObj = nullptr; }

luabind::object *BaseLuaObj::GetLuaObject() { return m_luaObj.get(); }
