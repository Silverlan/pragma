// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>
#include <vector>

export module pragma.shared:networking.resource;

export DLLNETWORK bool IsValidResource(std::string res);
