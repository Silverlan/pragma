#include "stdafx_client.h"
#include "pragma/entities/components/c_entity_character_component_model.hpp"

using namespace pragma;

CEntityCharacterComponentModel::CEntityCharacterComponentModel(CBaseEntity &ent)
	: BaseEntityComponent(ent)
{}
// COMPONENT TODO
//void CEntityCharacterComponentModel::ApplyAnimationBlending(AnimationSlotInfo &animInfo,double tDelta) {}

void CEntityCharacterComponentModel::Initialize()
{
	BaseEntityComponent::Initialize();
}
