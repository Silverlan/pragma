// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
