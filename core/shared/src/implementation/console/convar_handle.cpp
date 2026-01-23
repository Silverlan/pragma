// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.convar_handle;

pragma::console::PtrConVar::PtrConVar() : m_cvar(nullptr) {}
pragma::console::ConVar *pragma::console::PtrConVar::get() { return m_cvar; }
void pragma::console::PtrConVar::set(ConVar *cvar) { m_cvar = cvar; }

///////////////////////

pragma::console::ConVarHandle::ConVarHandle(std::shared_ptr<PtrConVar> &sptr) : m_cvar(sptr) {}

pragma::console::ConVar *pragma::console::ConVarHandle::operator->() { return m_cvar.get()->get(); }
