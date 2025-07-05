// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
