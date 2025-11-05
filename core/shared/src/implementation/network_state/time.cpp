// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :network_state;

double &NetworkState::RealTime() { return m_tReal; }
double &NetworkState::DeltaTime() { return m_tDelta; }
double &NetworkState::LastThink() { return m_tLast; }
