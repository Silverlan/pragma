/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASEENTITY_LUAOBJECT_H__
#define __BASEENTITY_LUAOBJECT_H__

#include "pragma/entities/baseentity.h"

template<class THandle>
	void BaseEntity::InitializeHandle()
{
	m_handle = new THandle(new PtrEntity(this));
}

#endif