// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.action_input_controller;

using namespace pragma;

ComponentEventId actionInputControllerComponent::EVENT_HANDLE_ACTION_INPUT = pragma::INVALID_COMPONENT_ID;
ComponentEventId actionInputControllerComponent::EVENT_ON_ACTION_INPUT_CHANGED = pragma::INVALID_COMPONENT_ID;
void ActionInputControllerComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	actionInputControllerComponent::EVENT_HANDLE_ACTION_INPUT = registerEvent("HANDLE_ACTION_INPUT", ComponentEventInfo::Type::Explicit);
	actionInputControllerComponent::EVENT_ON_ACTION_INPUT_CHANGED = registerEvent("ON_ACTION_INPUT_CHANGED", ComponentEventInfo::Type::Explicit);
}

ActionInputControllerComponent::ActionInputControllerComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ActionInputControllerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ActionInputControllerComponent::InitializeLuaObject(lua::State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ActionInputControllerComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

bool ActionInputControllerComponent::GetActionInput(pragma::Action action) const { return ((m_actionInputs & action) != pragma::Action::None) ? true : false; }
bool ActionInputControllerComponent::GetRawActionInput(pragma::Action action) const { return ((m_rawInputs & action) != pragma::Action::None) ? true : false; }
const std::unordered_map<pragma::Action, float> &ActionInputControllerComponent::GetActionInputAxisMagnitudes() const { return m_inputAxes; }
float ActionInputControllerComponent::GetActionInputAxisMagnitude(pragma::Action action) const
{
	auto it = m_inputAxes.find(action);
	if(it == m_inputAxes.end())
		return 0.f;
	return it->second;
}
void ActionInputControllerComponent::SetActionInputAxisMagnitude(pragma::Action action, float magnitude) { m_inputAxes[action] = magnitude; }

pragma::Action ActionInputControllerComponent::GetActionInputs() const { return m_actionInputs; }
pragma::Action ActionInputControllerComponent::GetRawActionInputs() const { return m_rawInputs; }
void ActionInputControllerComponent::SetActionInputs(pragma::Action actions, bool bKeepMagnitudes)
{
	auto rawInputs = m_rawInputs;
	auto valuesOld = umath::get_power_of_2_values(umath::to_integral(rawInputs));
	for(auto v : valuesOld) {
		if((actions & static_cast<pragma::Action>(v)) == pragma::Action::None) // Action has been unpressed
			SetActionInput(static_cast<pragma::Action>(v), false);
	}
	actions &= ~rawInputs;
	auto values = umath::get_power_of_2_values(umath::to_integral(actions));
	for(auto v : values)
		SetActionInput(static_cast<pragma::Action>(v), true, bKeepMagnitudes);
}
void ActionInputControllerComponent::SetActionInput(pragma::Action action, bool b, bool bKeepMagnitude) { SetActionInput(action, b, (bKeepMagnitude == true) ? GetActionInputAxisMagnitude(action) : ((b == true) ? 1.f : 0.f)); }
void ActionInputControllerComponent::SetActionInput(pragma::Action action, bool b, float magnitude)
{
	SetActionInputAxisMagnitude(action, (b == true) ? magnitude : 0.f);
	if(((m_rawInputs & action) != pragma::Action::None) == b)
		return;
	if(b == false)
		m_rawInputs &= ~action;
	else
		m_rawInputs |= action;

	CEHandleActionInput evData {action, b, magnitude};
	if(InvokeEventCallbacks(actionInputControllerComponent::EVENT_HANDLE_ACTION_INPUT, evData) == util::EventReply::Handled)
		return;
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto r = false;
	if(GetGame().CallCallbacks<bool, ActionInputControllerComponent *, pragma::Action, bool>("OnActionInput", &r, this, action, b) == CallbackReturnType::HasReturnValue) {
		if(r == false)
			return;
	}
	if(b == false) {
		if(GetActionInput(action) == true) {
			m_actionInputs &= ~action;
			InvokeEventCallbacks(actionInputControllerComponent::EVENT_ON_ACTION_INPUT_CHANGED, CEOnActionInputChanged {action, b});
		}
		return;
	}
	if(GetActionInput(action))
		return;
	m_actionInputs |= action;
	InvokeEventCallbacks(actionInputControllerComponent::EVENT_ON_ACTION_INPUT_CHANGED, CEOnActionInputChanged {action, b});
}

//////////////////

CEHandleActionInput::CEHandleActionInput(pragma::Action action, bool pressed, float magnitude) : action {action}, pressed {pressed}, magnitude {magnitude} {}
void CEHandleActionInput::PushArguments(lua::State *l)
{
	Lua::PushInt(l, umath::to_integral(action));
	Lua::PushBool(l, pressed);
	Lua::PushNumber(l, magnitude);
}

//////////////////

CEOnActionInputChanged::CEOnActionInputChanged(pragma::Action action, bool b) : action {action}, pressed {b} {}
void CEOnActionInputChanged::PushArguments(lua::State *l)
{
	Lua::PushInt(l, umath::to_integral(action));
	Lua::PushBool(l, pressed);
}
