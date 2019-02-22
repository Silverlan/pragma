#include "stdafx_shared.h"
#include "pragma/entities/basebot.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/basenpc.h"
#include "pragma/entities/components/base_model_component.hpp"

using namespace pragma;

void BaseBotComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto *pMdlComponent = static_cast<BaseModelComponent*>(ent.AddComponent("model").get());
	if(pMdlComponent != nullptr)
		pMdlComponent->SetModel("player/soldier.wmd");
}
