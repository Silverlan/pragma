// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.surface;

import :entities.base;
import :server_state;

using namespace pragma;

void SSurfaceComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<Vector4>(GetPlane().ToVector4()); }
void SSurfaceComponent::SetPlane(const math::Plane &plane)
{
	BaseSurfaceComponent::SetPlane(plane);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write<Vector4>(plane.ToVector4());
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetPlane, p, networking::Protocol::SlowReliable);
}
void SSurfaceComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
