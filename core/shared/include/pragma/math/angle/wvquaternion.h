/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WVQUATERNION_H__
#define __WVQUATERNION_H__
#include "pragma/definitions.h"
#include <mathutil/uquat.h>
#include "pragma/console/conout.h"

DLLNETWORK Con::c_cout& operator<<(Con::c_cout &os,const Quat &q);

#endif