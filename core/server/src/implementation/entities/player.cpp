// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server;
import :entities.player;

import :entities.components;

void Player::Initialize()
{
	SBaseEntity::Initialize();
	auto whPlComponent = AddComponent<pragma::SPlayerComponent>();
	if(whPlComponent.expired() == false)
		m_basePlayerComponent = util::WeakHandle<pragma::BasePlayerComponent>(std::static_pointer_cast<pragma::BasePlayerComponent>(whPlComponent.get()->shared_from_this()));
}
