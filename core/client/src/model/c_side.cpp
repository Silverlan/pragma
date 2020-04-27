/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/c_side.h"
#include <mathutil/umath.h>

CSide::CSide()
	: Side(),NormalMesh()
{
}

DLLCLIENT Con::c_cout& operator<<(Con::c_cout &os,const CSide side)
{
	return os<<(Side*)&side;
}

DLLCLIENT std::ostream& operator<<(std::ostream &os,const CSide side)
{
	return os<<(Side*)&side;
}
