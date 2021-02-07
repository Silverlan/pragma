/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WVANGLE_H__
#define __WVANGLE_H__
#include "pragma/definitions.h"
#include <mathutil/eulerangles.h>

namespace Con
{
	class c_cout;
	class c_cwar;
	class c_cerr;
	class c_crit;
	class c_csv;
	class c_ccl;
};
DLLNETWORK Con::c_cout & operator<<(Con::c_cout &os,const EulerAngles &ang);

#endif