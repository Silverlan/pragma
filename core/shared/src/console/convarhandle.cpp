/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/console/convarhandle.h"
#include <pragma/console/convars.h>

PtrConVar::PtrConVar() : m_cvar(nullptr) {}
ConVar *PtrConVar::get() { return m_cvar; }
void PtrConVar::set(ConVar *cvar) { m_cvar = cvar; }

///////////////////////

ConVarHandle::ConVarHandle(std::shared_ptr<PtrConVar> &sptr) : m_cvar(sptr) {}

ConVar *ConVarHandle::operator->() { return m_cvar.get()->get(); }
