// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.observable;

using namespace pragma;

CObservableComponent::CObservableComponent(ecs::BaseEntity &ent) : BaseObservableComponent(ent) {}
void CObservableComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CObservableComponent::ReceiveData(NetPacket &packet)
{
	constexpr auto numTypes = math::to_integral(CameraType::Count);
	for(auto i = 0u; i < numTypes; ++i) {
		auto &data = GetCameraData(static_cast<CameraType>(i));
		*data.enabled = packet->Read<bool>();

		auto hasLocalOrigin = packet->Read<bool>();
		if(hasLocalOrigin)
			data.localOrigin = packet->Read<Vector3>();
		else
			data.localOrigin = {};

		*data.offset = packet->Read<Vector3>();
		data.rotateWithObservee = packet->Read<bool>();
		auto hasLimits = packet->Read<bool>();
		if(hasLimits) {
			auto minLimits = packet->Read<EulerAngles>();
			auto maxLimits = packet->Read<EulerAngles>();
			data.angleLimits = {minLimits, maxLimits};
		}
		else
			data.angleLimits = {};
	}
}

Bool CObservableComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
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
