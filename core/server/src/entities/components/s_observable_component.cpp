// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/components/s_observable_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

using namespace pragma;

void SObservableComponent::SetLocalCameraOrigin(CameraType type, const Vector3 &origin)
{
	BaseObservableComponent::SetLocalCameraOrigin(type, origin);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->Write<CameraType>(type);
	p->Write<Vector3>(origin);
	ent.SendNetEvent(m_netSetObserverOrigin, p, pragma::networking::Protocol::SlowReliable);
}
void SObservableComponent::SetLocalCameraOffset(CameraType type, const Vector3 &offset)
{
	BaseObservableComponent::SetLocalCameraOffset(type, offset);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->Write<CameraType>(type);
	p->Write<Vector3>(offset);
	ent.SendNetEvent(m_netSetObserverOffset, p, pragma::networking::Protocol::SlowReliable);
}
void SObservableComponent::SetViewOffset(const Vector3 &offset)
{
	BaseObservableComponent::SetViewOffset(offset);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->Write<Vector3>(offset);
	ent.SendNetEvent(m_netSetViewOffset, p, pragma::networking::Protocol::SlowReliable);
}
void SObservableComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SObservableComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	constexpr auto numTypes = umath::to_integral(CameraType::Count);
	for(auto i = 0u; i < numTypes; ++i) {
		auto &data = GetCameraData(static_cast<CameraType>(i));
		packet->Write<bool>(*data.enabled);
		auto hasLocalOrigin = data.localOrigin.has_value();
		packet->Write<bool>(hasLocalOrigin);
		if(hasLocalOrigin)
			packet->Write<Vector3>(*data.localOrigin);
		packet->Write<Vector3>(*data.offset);
		packet->Write<bool>(data.rotateWithObservee);
		auto hasLimits = data.angleLimits.has_value();
		packet->Write<bool>(hasLimits);
		if(hasLimits) {
			packet->Write<EulerAngles>(data.angleLimits->first);
			packet->Write<EulerAngles>(data.angleLimits->second);
		}
	}
}
