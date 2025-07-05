// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/baseplayer.hpp"

util::WeakHandle<pragma::BasePlayerComponent> BasePlayer::GetBasePlayerComponent() { return m_basePlayerComponent; }
