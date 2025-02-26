/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/entities/components/c_observer_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;
CObservableComponent::CObservableComponent(BaseEntity &ent) : BaseObservableComponent(ent) {}
void CObservableComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CObservableComponent::ReceiveData(NetPacket &packet)
{
	constexpr auto numTypes = umath::to_integral(CameraType::Count);
	for(auto i = 0u; i < numTypes; ++i) {
		auto &data = GetCameraData(static_cast<CameraType>(i));
		*data.enabled = packet->Read<bool>();
		*data.localOrigin = packet->Read<Vector3>();
		*data.offset = packet->Read<Vector3>();
		data.rotateWithObservee = packet->Read<bool>();
		auto hasLimits = packet->Read<bool>();
		if(hasLimits) {
			auto minLimits = packet->Read<EulerAngles>();
			auto maxLimits = packet->Read<EulerAngles>();
			data.angleLimits = {minLimits, maxLimits};
		}
	}
}

Bool CObservableComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netSetObserverOrigin) {
		auto camType = packet->Read<CameraType>();
		auto origin = packet->Read<Vector3>();
		SetLocalCameraOrigin(camType, origin);
	}
	else if(eventId == m_netSetObserverOffset) {
		auto camType = packet->Read<CameraType>();
		auto offset = packet->Read<Vector3>();
		SetLocalCameraOffset(camType, offset);
	}
	else if(eventId == m_netSetViewOffset) {
		auto offset = packet->Read<Vector3>();
		SetViewOffset(offset);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

void CObservableComponent::SetLocalCameraOrigin(CameraType type, const Vector3 &offset) { BaseObservableComponent::SetLocalCameraOrigin(type, offset); }
