// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.lua;

import :entities;

using namespace pragma;

SLuaBaseEntityComponent::SLuaBaseEntityComponent(ecs::BaseEntity &ent) : BaseLuaBaseEntityComponent(ent), SBaseSnapshotComponent() {}
void SLuaBaseEntityComponent::OnMemberValueChanged(uint32_t memberIdx)
{
	BaseLuaBaseEntityComponent::OnMemberValueChanged(memberIdx);
	if(m_networkedMemberInfo == nullptr)
		return;
	auto &members = GetMembers();
	if(memberIdx >= members.size())
		return;
	auto &member = members.at(memberIdx);
	if((member.flags & MemberFlags::TransmitOnChange) != MemberFlags::TransmitOnChange) {
		if((member.flags & MemberFlags::SnapshotData) == MemberFlags::SnapshotData)
			static_cast<SBaseEntity &>(GetEntity()).MarkForSnapshot(true);
		return;
	}
	auto itNwIndex = m_networkedMemberInfo->memberIndexToNetworkedIndex.find(memberIdx);
	if(itNwIndex == m_networkedMemberInfo->memberIndexToNetworkedIndex.end()) {
		// This should be unreachable!
		throw std::logic_error("Invalid networked variable '" + std::string {member.functionName} + "'!");
		return;
	}
	auto nwIndex = itNwIndex->second;
	const auto maxNwVars = std::numeric_limits<uint8_t>::max();
	if(nwIndex > maxNwVars) {
		Con::CWAR << "Networked member index of '" << member.functionName << "' exceeds maximum allowed number of networked variables (" << maxNwVars << ")!" << Con::endl;
		return;
	}

	auto value = GetMemberValue(member);
	NetPacket p {};
	p->Write<uint8_t>(nwIndex);
	Lua::WriteAny(p, detail::member_type_to_util_type(member.type), value);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_networkedMemberInfo->netEvSetMember, p, networking::Protocol::SlowReliable);
}
void SLuaBaseEntityComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	if(m_networkedMemberInfo != nullptr) {
		auto &members = GetMembers();
		for(auto idx : m_networkedMemberInfo->networkedMembers) {
			auto &member = members.at(idx);
			auto value = GetMemberValue(member);
			Lua::WriteAny(packet, detail::member_type_to_util_type(member.type), value);
		}
	}

	CallLuaMethod<void, NetPacket, networking::ClientRecipientFilter>("SendData", packet, rp);
}
Bool SLuaBaseEntityComponent::ReceiveNetEvent(BasePlayerComponent &pl, NetEventId evId, NetPacket &packet)
{
	auto it = m_boundNetEvents.find(evId);
	if(it != m_boundNetEvents.end()) {
		it->second.Call<void, std::reference_wrapper<NetPacket>, BasePlayerComponent *>(std::reference_wrapper<NetPacket> {packet}, &pl);
		return true;
	}

	auto handled = static_cast<uint32_t>(util::EventReply::Unhandled);
	CallLuaMethod<uint32_t, luabind::object, uint32_t, NetPacket>("ReceiveNetEvent", &handled, pl.GetLuaObject(), evId, packet);
	return static_cast<util::EventReply>(handled) == util::EventReply::Handled;
}
void SLuaBaseEntityComponent::SendSnapshotData(NetPacket &packet, BasePlayerComponent &pl)
{
	auto &members = GetMembers();
	if(m_networkedMemberInfo != nullptr) {
		for(auto idx : m_networkedMemberInfo->snapshotMembers) {
			auto &member = members.at(idx);
			auto value = GetMemberValue(member);
			Lua::WriteAny(packet, detail::member_type_to_util_type(member.type), value);
		}
	}
	CallLuaMethod<void, NetPacket, luabind::object>("SendSnapshotData", packet, pl.GetLuaObject());
}
bool SLuaBaseEntityComponent::ShouldTransmitNetData() const { return IsNetworked(); }
bool SLuaBaseEntityComponent::ShouldTransmitSnapshotData() const { return BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData(); }
void SLuaBaseEntityComponent::InvokeNetEventHandle(const std::string &methodName, NetPacket &packet, BasePlayerComponent *pl) { CallLuaMethod<void, luabind::object, NetPacket>(methodName, pl->GetLuaObject(), packet); }
