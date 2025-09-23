// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <mathutil/umath.h>

module pragma.client;

import :model.side;

CSide::CSide() : Side() {}

Con::c_cout &operator<<(Con::c_cout &os, const CSide side) { return os << (Side *)&side; }

std::ostream &operator<<(std::ostream &os, const CSide side) { return os << (Side *)&side; }
