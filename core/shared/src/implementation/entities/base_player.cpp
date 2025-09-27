// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/baseplayer.hpp"

module pragma.shared;

import :entities.base_player;

util::WeakHandle<pragma::BasePlayerComponent> BasePlayer::GetBasePlayerComponent() { return m_basePlayerComponent; }
