// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/lua/base_lua_handle_method.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/lua/sh_lua_component.hpp>
#include <pragma/lua/handle_holder.hpp>

module pragma.client.entities.components;

import :lua;

using namespace pragma;

CLuaBaseEntityComponent::CLuaBaseEntityComponent(BaseEntity &ent) : BaseLuaBaseEntityComponent(ent), CBaseSnapshotComponent() {}

void CLuaBaseEntityComponent::ReceiveData(NetPacket &packet)
{
	if(m_networkedMemberInfo != nullptr) {
		auto &members = GetMembers();
		for(auto idx : m_networkedMemberInfo->networkedMembers) {
			auto &member = members.at(idx);
			std::any value;
			Lua::ReadAny(packet, detail::member_type_to_util_type(member.type), value);
			SetMemberValue(member, value);
		}
	}
	CallLuaMethod<void, NetPacket>("ReceiveData", packet);
}
Bool CLuaBaseEntityComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(m_networkedMemberInfo != nullptr && eventId == m_networkedMemberInfo->netEvSetMember) {
		auto nwIdx = packet->Read<uint8_t>();
		if(nwIdx >= m_networkedMemberInfo->networkedMembers.size()) {
			Con::cwar << "Invalid networked variable index '" << nwIdx << "'!" << Con::endl;
			return true;
		}
		auto memberIdx = m_networkedMemberInfo->networkedMembers.at(nwIdx);
		auto &member = GetMembers().at(memberIdx);
		std::any value;
		Lua::ReadAny(packet, detail::member_type_to_util_type(member.type), value);
		SetMemberValue(member, value);
		return true;
	}

	auto it = m_boundNetEvents.find(eventId);
	if(it != m_boundNetEvents.end()) {
		it->second.Call<void, std::reference_wrapper<NetPacket>>(std::reference_wrapper<NetPacket> {packet});
		return true;
	}
	auto handled = static_cast<uint32_t>(util::EventReply::Unhandled);
	CallLuaMethod<uint32_t, uint32_t, NetPacket>("ReceiveNetEvent", &handled, eventId, packet);
	return static_cast<util::EventReply>(handled) == util::EventReply::Handled;
}
void CLuaBaseEntityComponent::ReceiveSnapshotData(NetPacket &packet)
{
	auto &members = GetMembers();
	if(m_networkedMemberInfo != nullptr) {
		for(auto idx : m_networkedMemberInfo->snapshotMembers) {
			auto &member = members.at(idx);
			std::any value;
			Lua::ReadAny(packet, detail::member_type_to_util_type(member.type), value);
			SetMemberValue(member, value);
		}
	}
	CallLuaMethod<void, NetPacket>("ReceiveSnapshotData", packet);
}
bool CLuaBaseEntityComponent::ShouldTransmitNetData() const { return IsNetworked(); }
bool CLuaBaseEntityComponent::ShouldTransmitSnapshotData() const { return BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData(); }
void CLuaBaseEntityComponent::InvokeNetEventHandle(const std::string &methodName, NetPacket &packet, pragma::BasePlayerComponent *pl) { CallLuaMethod<void, NetPacket>(methodName, packet); }
