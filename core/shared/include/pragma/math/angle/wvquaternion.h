// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WVQUATERNION_H__
#define __WVQUATERNION_H__
#include "pragma/definitions.h"
#include <mathutil/uquat.h>
#include "pragma/console/conout.h"

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Quat &q);

#endif
