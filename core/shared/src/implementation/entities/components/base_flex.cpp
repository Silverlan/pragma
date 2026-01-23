// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_flex;

using namespace pragma;

void BaseFlexComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseEntityComponent::RegisterEvents(componentManager, registerEvent);
	baseFlexComponent::EVENT_ON_FLEX_CONTROLLER_CHANGED = registerEvent("ON_FLEX_CONTROLLER_CHANGED", ComponentEventInfo::Type::Explicit);
}
void BaseFlexComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseFlexComponent;
	{
		using TEnableFlexControllerLimitsEnabled = bool;
		auto memberInfo = create_component_member_info<T, TEnableFlexControllerLimitsEnabled, &T::SetFlexControllerLimitsEnabled, &T::AreFlexControllerLimitsEnabled>("flexControllerLimitsEnabled", true);
		registerMember(std::move(memberInfo));
	}

	{
		using TFlexControllerScale = float;
		auto memberInfo = create_component_member_info<T, TFlexControllerScale, static_cast<void (T::*)(TFlexControllerScale)>(&T::SetFlexControllerScale), static_cast<TFlexControllerScale (T::*)() const>(&T::GetFlexControllerScale)>("flexControllerScale", 1.f);
		memberInfo.SetMin(-5.f);
		memberInfo.SetMax(5.f);
		registerMember(std::move(memberInfo));
	}
}
BaseFlexComponent::BaseFlexComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseFlexComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &changeData = static_cast<CEOnModelChanged &>(evData.get());
		OnModelChanged(changeData.model);
	});

	auto &mdl = GetEntity().GetModel();
	if(mdl)
		OnModelChanged(mdl);
}
void BaseFlexComponent::SetFlexControllerUpdateListenersEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::EnableFlexControllerUpdateListeners, enabled); }
bool BaseFlexComponent::AreFlexControllerUpdateListenersEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::EnableFlexControllerUpdateListeners); }
void BaseFlexComponent::OnModelChanged(const std::shared_ptr<asset::Model> &model)
{
	util::ScopeGuard sg {[this]() { OnMembersChanged(); }};
	ClearMembers();
	if(!model)
		return;
	auto &flexControllers = model->GetFlexControllers();
	ReserveMembers(flexControllers.size());
	for(uint32_t idx = 0; auto &flexController : flexControllers) {
		const auto &name = flexController.name;
		auto lname = name;
		// pragma::string::to_lower(lname);
		auto memberInfo = ComponentMemberInfo::CreateDummy();
		memberInfo.SetName("flexController/" + lname);
		memberInfo.type = ents::EntityMemberType::Float;
		memberInfo.SetMin(flexController.min);
		memberInfo.SetMax(flexController.max);
		memberInfo.userIndex = idx++;
		memberInfo.SetGetterFunction<BaseFlexComponent, float,
		  static_cast<void (*)(const ComponentMemberInfo &, BaseFlexComponent &, float &)>([](const ComponentMemberInfo &memberInfo, BaseFlexComponent &component, float &outValue) { outValue = component.GetFlexController(memberInfo.userIndex); })>();
		memberInfo.SetSetterFunction<BaseFlexComponent, float, static_cast<void (*)(const ComponentMemberInfo &, BaseFlexComponent &, const float &)>([](const ComponentMemberInfo &memberInfo, BaseFlexComponent &component, const float &value) {
			component.SetFlexController(memberInfo.userIndex, value, 0.f, component.AreFlexControllerLimitsEnabled());
		})>();
		RegisterMember(std::move(memberInfo));
	}
}
const ComponentMemberInfo *BaseFlexComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx);
}
std::optional<ComponentMemberIndex> BaseFlexComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx;
	return std::optional<ComponentMemberIndex> {};
}
void BaseFlexComponent::SetFlexController(const std::string &name, float val, float duration, bool clampToLimits)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto flexId = 0u;
	if(!mdlComponent || mdlComponent->LookupFlexController(name, flexId) == false)
		return;
	SetFlexController(flexId, val, duration, clampToLimits);
}
float BaseFlexComponent::GetFlexController(uint32_t flexId) const
{
	auto r = 0.f;
	GetFlexController(flexId, r);
	return r;
}
bool BaseFlexComponent::GetScaledFlexController(uint32_t flexId, float &val) const
{
	if(GetFlexController(flexId, val) == false)
		return false;
	val *= GetFlexControllerScale();
	return true;
}

void BaseFlexComponent::SetFlexControllerLimitsEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::EnableFlexControllerLimits, enabled); }
bool BaseFlexComponent::AreFlexControllerLimitsEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::EnableFlexControllerLimits); }

void BaseFlexComponent::SetFlexControllerScale(float scale) { m_flexControllerScale = scale; }
float BaseFlexComponent::GetFlexControllerScale() const { return m_flexControllerScale; }
float BaseFlexComponent::GetFlexController(const std::string &flexController) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto flexId = 0u;
	if(!mdlComponent || mdlComponent->LookupFlexController(flexController, flexId) == false)
		return 0.f;
	return GetFlexController(flexId);
}

/////////////////

CEOnFlexControllerChanged::CEOnFlexControllerChanged(animation::FlexControllerId flexControllerId, float value) : flexControllerId {flexControllerId}, value {value} {}
void CEOnFlexControllerChanged::PushArguments(lua::State *l)
{
	Lua::PushInt(l, flexControllerId);
	Lua::PushNumber(l, value);
}
