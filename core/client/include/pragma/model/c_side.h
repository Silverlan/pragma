/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SIDE_H__
#define __C_SIDE_H__
#include "pragma/clientdefinitions.h"
#include <pragma/model/side.h>
#include "material.h"


class DLLCLIENT CSide : public Side {
  public:
	CSide();
  public:
};

DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const CSide side);
DLLCLIENT std::ostream &operator<<(std::ostream &os, const CSide side);

#endif
