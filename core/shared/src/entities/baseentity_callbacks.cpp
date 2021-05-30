/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/base_generic_component.hpp"

CallbackHandle BaseEntity::CallOnRemove(const CallbackHandle &hCallback)
{
	auto *pComponent = static_cast<pragma::BaseGenericComponent*>(FindComponent("entity").get());
	if(pComponent != nullptr)
		pComponent->BindEventUnhandled(BaseEntity::EVENT_ON_REMOVE,hCallback);
	return hCallback;
}