/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/point/s_point_rendertarget.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <sharedutils/netpacket.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

// LINK_ENTITY_TO_CLASS(point_rendertarget,PointRenderTarget);

void SRenderTargetComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvMaterial);
	packet->Write<float>(m_kvFOV);
	packet->Write<float>(m_kvRefreshRate);
	packet->Write<float>(m_kvRenderWidth);
	packet->Write<float>(m_kvRenderHeight);
	packet->Write<float>(m_kvNearZ);
	packet->Write<float>(m_kvFarZ);
	packet->Write<int>(m_kvRenderDepth);
}

void SRenderTargetComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointRenderTarget::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SRenderTargetComponent>();
}
