// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/baseplayer.hpp>

export module pragma.server.entities.player;

export class DLLSERVER Player : public SBaseEntity, public BasePlayer {
  public:
	virtual void Initialize() override;
};
