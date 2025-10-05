// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/serverdefinitions.h"

import pragma.shared;

DLLSERVER void CMD_startserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv) { Engine::Get()->StartServer(false); }
DLLSERVER void CMD_closeserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { Engine::Get()->CloseServer(); }
