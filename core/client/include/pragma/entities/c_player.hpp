/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PLAYER_HPP__
#define __C_PLAYER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseplayer.hpp>

class DLLCLIENT CPlayer
	: public CBaseEntity,
	public BasePlayer
{
public:
	virtual void Initialize() override;
};

#endif
