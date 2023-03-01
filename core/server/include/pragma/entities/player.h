/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/baseplayer.hpp>

class WVServerClient;
class DLLSERVER Player : public SBaseEntity, public BasePlayer {
  public:
	virtual void Initialize() override;
};

#endif
