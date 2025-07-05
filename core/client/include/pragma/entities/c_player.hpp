// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PLAYER_HPP__
#define __C_PLAYER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseplayer.hpp>

class DLLCLIENT CPlayer : public CBaseEntity, public BasePlayer {
  public:
	virtual void Initialize() override;
};

#endif
