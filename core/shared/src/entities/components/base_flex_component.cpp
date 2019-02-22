#include "stdafx_shared.h"
#include "pragma/entities/components/base_flex_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"

using namespace pragma;

BaseFlexComponent::BaseFlexComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void BaseFlexComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}
void BaseFlexComponent::SetFlexController(const std::string &name,float val,float duration)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto flexId = 0u;
	if(mdlComponent.expired() || mdlComponent->LookupFlexController(name,flexId) == false)
		return;
	SetFlexController(flexId,val,duration);
}
float BaseFlexComponent::GetFlexController(uint32_t flexId) const
{
	auto r = 0.f;
	GetFlexController(flexId,r);
	return r;
}
float BaseFlexComponent::GetFlexController(const std::string &flexController) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto flexId = 0u;
	if(mdlComponent.expired() || mdlComponent->LookupFlexController(flexController,flexId) == false)
		return 0.f;
	return GetFlexController(flexId);
}
