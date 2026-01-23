// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_constraint_slider;
using namespace pragma;

void CPointConstraintSliderComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = networking::read_vector(packet);

	m_kvLimitLinLow = packet->Read<float>();
	m_kvLimitLinHigh = packet->Read<float>();
}
void CPointConstraintSliderComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CPointConstraintSlider::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintSliderComponent>();
}
