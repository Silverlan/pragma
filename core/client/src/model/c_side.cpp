// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/model/c_side.h"
#include <mathutil/umath.h>

CSide::CSide() : Side() {}

DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const CSide side) { return os << (Side *)&side; }

DLLCLIENT std::ostream &operator<<(std::ostream &os, const CSide side) { return os << (Side *)&side; }
