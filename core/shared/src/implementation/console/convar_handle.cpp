// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <string>

module pragma.shared;

import :console.convar_handle;

PtrConVar::PtrConVar() : m_cvar(nullptr) {}
ConVar *PtrConVar::get() { return m_cvar; }
void PtrConVar::set(ConVar *cvar) { m_cvar = cvar; }

///////////////////////

ConVarHandle::ConVarHandle(std::shared_ptr<PtrConVar> &sptr) : m_cvar(sptr) {}

ConVar *ConVarHandle::operator->() { return m_cvar.get()->get(); }
