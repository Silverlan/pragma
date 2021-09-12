/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_flex_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/model/model.h"
#include <udm_types.hpp>

using namespace pragma;

BaseFlexComponent::BaseFlexComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void BaseFlexComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &changeData = static_cast<CEOnModelChanged&>(evData.get());
		OnModelChanged(changeData.model);
	});

	auto &mdl = GetEntity().GetModel();
	if(mdl)
		OnModelChanged(mdl);
}
void BaseFlexComponent::OnModelChanged(const std::shared_ptr<Model> &model)
{
	util::ScopeGuard sg {[this]() {OnMembersChanged();}};
	ClearMembers();
	if(!model)
		return;
	auto &flexControllers = model->GetFlexControllers();
	ReserveMembers(flexControllers.size());
	for(uint32_t idx = 0; auto &flexController : flexControllers)
	{
		const auto &name = flexController.name;
		auto lname = name;
		ustring::to_lower(lname);
		auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();
		memberInfo.SetName("flex/" +lname);
		memberInfo.type = udm::Type::Float;
		memberInfo.userIndex = idx++;
		memberInfo.SetGetterFunction<BaseFlexComponent,float,static_cast<void(*)(const pragma::ComponentMemberInfo&,BaseFlexComponent&,float&)>(
			[](const pragma::ComponentMemberInfo &memberInfo,BaseFlexComponent &component,float &outValue) {
			outValue = component.GetFlexController(memberInfo.userIndex);
		})>();
		memberInfo.SetSetterFunction<BaseFlexComponent,float,static_cast<void(*)(const pragma::ComponentMemberInfo&,BaseFlexComponent&,const float&)>(
			[](const pragma::ComponentMemberInfo &memberInfo,BaseFlexComponent &component,const float &value) {
			component.SetFlexController(memberInfo.userIndex,value);
		})>();
		RegisterMember(std::move(memberInfo));
	}
}
const ComponentMemberInfo *BaseFlexComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx -numStatic);
}
std::optional<ComponentMemberIndex> BaseFlexComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex>{};
}
void BaseFlexComponent::SetFlexController(const std::string &name,float val,float duration,bool clampToLimits)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto flexId = 0u;
	if(!mdlComponent || mdlComponent->LookupFlexController(name,flexId) == false)
		return;
	SetFlexController(flexId,val,duration,clampToLimits);
}
float BaseFlexComponent::GetFlexController(uint32_t flexId) const
{
	auto r = 0.f;
	GetFlexController(flexId,r);
	return r;
}
bool BaseFlexComponent::GetScaledFlexController(uint32_t flexId,float &val) const
{
	if(GetFlexController(flexId,val) == false)
		return false;
	val *= GetFlexControllerScale();
	return true;
}

void BaseFlexComponent::SetFlexControllerScale(float scale) {m_flexControllerScale = scale;}
float BaseFlexComponent::GetFlexControllerScale() const {return m_flexControllerScale;}
float BaseFlexComponent::GetFlexController(const std::string &flexController) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto flexId = 0u;
	if(!mdlComponent || mdlComponent->LookupFlexController(flexController,flexId) == false)
		return 0.f;
	return GetFlexController(flexId);
}
