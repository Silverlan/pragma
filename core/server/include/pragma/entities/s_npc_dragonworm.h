/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_NPC_DRAGONWORM_H__
#define __S_NPC_DRAGONWORM_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/basenpcdragonworm.h"

namespace pragma
{
	class DLLSERVER SDragonWormComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	};
};

class DLLSERVER NPCDragonWorm
	: public SBaseEntity
{
public:
};

#endif