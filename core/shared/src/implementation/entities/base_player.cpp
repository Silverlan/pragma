// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.base_player;

pragma::util::WeakHandle<pragma::BasePlayerComponent> BasePlayer::GetBasePlayerComponent() { return m_basePlayerComponent; }
