// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <sstream>
#include <string>

export module pragma.shared:string.format;

export std::string DLLNETWORK FormatTime(double dtm);
