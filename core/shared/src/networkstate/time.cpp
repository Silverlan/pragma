/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/engine.h>
extern DLLNETWORK Engine *engine;
double &NetworkState::RealTime() { return m_tReal; }
double &NetworkState::DeltaTime() { return m_tDelta; }
double &NetworkState::LastThink() { return m_tLast; }
