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