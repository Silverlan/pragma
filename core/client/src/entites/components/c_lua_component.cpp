/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_lua_component.hpp"

using namespace pragma;

CLuaBaseEntityComponent::CLuaBaseEntityComponent(BaseEntity &ent,luabind::object &o)
	: BaseLuaBaseEntityComponent(ent,o),CBaseSnapshotComponent()
{}

void CLuaBaseEntityComponent::ReceiveData(NetPacket &packet)
{
	if(m_networkedMemberInfo != nullptr)
	{
		auto &members = GetMembers();
		for(auto idx : m_networkedMemberInfo->networkedMembers)
		{
			auto &member = members.at(idx);
			std::any value;
			Lua::ReadAny(packet,member.type,value);
			SetMemberValue(member,value);
		}
	}
	CallLuaMember<void,NetPacket>("ReceiveData",packet);
}
Bool CLuaBaseEntityComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(m_networkedMemberInfo != nullptr && eventId == m_networkedMemberInfo->netEvSetMember)
	{
		auto nwIdx = packet->Read<uint8_t>();
		if(nwIdx >= m_networkedMemberInfo->networkedMembers.size())
		{
			Con::cwar<<"WARNING: Invalid networked variable index '"<<nwIdx<<"'!"<<Con::endl;
			return true;
		}
		auto memberIdx = m_networkedMemberInfo->networkedMembers.at(nwIdx);
		auto &member = GetMembers().at(memberIdx);
		std::any value;
		Lua::ReadAny(packet,member.type,value);
		SetMemberValue(member,value);
		return true;
	}

	auto it = m_boundNetEvents.find(eventId);
	if(it != m_boundNetEvents.end())
	{
		it->second.Call<void,std::reference_wrapper<NetPacket>>(std::reference_wrapper<NetPacket>{packet});
		return true;
	}
	auto handled = static_cast<uint32_t>(util::EventReply::Unhandled);
	CallLuaMember<uint32_t,uint32_t,NetPacket>("ReceiveNetEvent",&handled,eventId,packet);
	return static_cast<util::EventReply>(handled) == util::EventReply::Handled;
}
void CLuaBaseEntityComponent::ReceiveSnapshotData(NetPacket &packet)
{
	auto &members = GetMembers();
	if(m_networkedMemberInfo != nullptr)
	{
		for(auto idx : m_networkedMemberInfo->snapshotMembers)
		{
			auto &member = members.at(idx);
			std::any value;
			Lua::ReadAny(packet,member.type,value);
			SetMemberValue(member,value);
		}
	}
	CallLuaMember<void,NetPacket>("ReceiveSnapshotData",packet);
}
bool CLuaBaseEntityComponent::ShouldTransmitNetData() const {return IsNetworked();}
bool CLuaBaseEntityComponent::ShouldTransmitSnapshotData() const {return BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData();}
void CLuaBaseEntityComponent::InvokeNetEventHandle(const std::string &methodName,NetPacket &packet,pragma::BasePlayerComponent *pl)
{
	CallLuaMember<void,NetPacket>(methodName,packet);
}
