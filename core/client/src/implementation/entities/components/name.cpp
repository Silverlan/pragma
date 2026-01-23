// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.name;
using namespace pragma;

void CNameComponent::ReceiveData(NetPacket &packet)
{
	std::string name = packet->ReadString();
	SetName(name);
}
void CNameComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
