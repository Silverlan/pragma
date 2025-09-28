// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <sharedutils/util_string.h>
#include "sharedutils/netpacket.hpp"

module pragma.server.entities.components.point.constraints.slider;

using namespace pragma;

void SPointConstraintSliderComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	nwm::write_vector(packet, m_posTarget);

	packet->Write<float>(m_kvLimitLinLow);
	packet->Write<float>(m_kvLimitLinHigh);
}

void SPointConstraintSliderComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointConstraintSlider::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintSliderComponent>();
}
