// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"

import pragma.client;

ConVarHandle GetClientConVar(std::string scmd) { return pragma::get_client_state()->GetConVarHandle(scmd); }
