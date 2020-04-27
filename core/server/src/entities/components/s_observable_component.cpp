/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/components/s_observable_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

using namespace pragma;

void SObservableComponent::SetLocalCameraOrigin(CameraType type,const Vector3 &origin)
{
	BaseObservableComponent::SetLocalCameraOrigin(type,origin);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->Write<CameraType>(type);
	p->Write<Vector3>(origin);
	ent.SendNetEvent(m_netSetObserverOrigin,p,pragma::networking::Protocol::SlowReliable);
}
void SObservableComponent::SetLocalCameraOffset(CameraType type,const Vector3 &offset)
{
	BaseObservableComponent::SetLocalCameraOffset(type,offset);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->Write<CameraType>(type);
	p->Write<Vector3>(offset);
	ent.SendNetEvent(m_netSetObserverOffset,p,pragma::networking::Protocol::SlowReliable);
}
luabind::object SObservableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SObservableComponentHandleWrapper>(l);}
void SObservableComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	constexpr auto numTypes = umath::to_integral(CameraType::Count);
	for(auto i=0u;i<numTypes;++i)
	{
		auto &data = GetCameraData(static_cast<CameraType>(i));
		packet->Write<bool>(*data.enabled);
		packet->Write<Vector3>(*data.localOrigin);
		packet->Write<Vector3>(*data.offset);
		packet->Write<bool>(data.rotateWithObservee);
		auto hasLimits = data.angleLimits.has_value();
		packet->Write<bool>(hasLimits);
		if(hasLimits)
		{
			packet->Write<EulerAngles>(data.angleLimits->first);
			packet->Write<EulerAngles>(data.angleLimits->second);
		}
	}
}
