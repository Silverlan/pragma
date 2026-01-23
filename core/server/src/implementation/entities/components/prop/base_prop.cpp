// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.prop.base;

using namespace pragma;

void PropComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PropComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<float>(m_kvMass); }
