// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/engine.h>

double &NetworkState::RealTime() { return m_tReal; }
double &NetworkState::DeltaTime() { return m_tDelta; }
double &NetworkState::LastThink() { return m_tLast; }
