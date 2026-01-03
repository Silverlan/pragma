// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.action_input_controller;

using namespace pragma;

void ActionInputControllerComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	actionInputControllerComponent::EVENT_HANDLE_ACTION_INPUT = registerEvent("HANDLE_ACTION_INPUT", ComponentEventInfo::Type::Explicit);
	actionInputControllerComponent::EVENT_ON_ACTION_INPUT_CHANGED = registerEvent("ON_ACTION_INPUT_CHANGED", ComponentEventInfo::Type::Explicit);
}

ActionInputControllerComponent::ActionInputControllerComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ActionInputControllerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ActionInputControllerComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ActionInputControllerComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

bool ActionInputControllerComponent::GetActionInput(Action action) const { return ((m_actionInputs & action) != Action::None) ? true : false; }
bool ActionInputControllerComponent::GetRawActionInput(Action action) const { return ((m_rawInputs & action) != Action::None) ? true : false; }
const std::unordered_map<Action, float> &ActionInputControllerComponent::GetActionInputAxisMagnitudes() const { return m_inputAxes; }
float ActionInputControllerComponent::GetActionInputAxisMagnitude(Action action) const
{
	auto it = m_inputAxes.find(action);
	if(it == m_inputAxes.end())
		return 0.f;
	return it->second;
}
void ActionInputControllerComponent::SetActionInputAxisMagnitude(Action action, float magnitude) { m_inputAxes[action] = magnitude; }

Action ActionInputControllerComponent::GetActionInputs() const { return m_actionInputs; }
Action ActionInputControllerComponent::GetRawActionInputs() const { return m_rawInputs; }
void ActionInputControllerComponent::SetActionInputs(Action actions, bool bKeepMagnitudes)
{
	auto rawInputs = m_rawInputs;
	auto valuesOld = math::get_power_of_2_values(math::to_integral(rawInputs));
	for(auto v : valuesOld) {
		if((actions & static_cast<Action>(v)) == Action::None) // Action has been unpressed
			SetActionInput(static_cast<Action>(v), false);
	}
	actions &= ~rawInputs;
	auto values = math::get_power_of_2_values(math::to_integral(actions));
	for(auto v : values)
		SetActionInput(static_cast<Action>(v), true, bKeepMagnitudes);
}
void ActionInputControllerComponent::SetActionInput(Action action, bool b, bool bKeepMagnitude) { SetActionInput(action, b, (bKeepMagnitude == true) ? GetActionInputAxisMagnitude(action) : ((b == true) ? 1.f : 0.f)); }
void ActionInputControllerComponent::SetActionInput(Action action, bool b, float magnitude)
{
	SetActionInputAxisMagnitude(action, (b == true) ? magnitude : 0.f);
	if(((m_rawInputs & action) != Action::None) == b)
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
	if(GetGame().CallCallbacks<bool, ActionInputControllerComponent *, Action, bool>("OnActionInput", &r, this, action, b) == CallbackReturnType::HasReturnValue) {
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

CEHandleActionInput::CEHandleActionInput(Action action, bool pressed, float magnitude) : action {action}, pressed {pressed}, magnitude {magnitude} {}
void CEHandleActionInput::PushArguments(lua::State *l)
{
	Lua::PushInt(l, math::to_integral(action));
	Lua::PushBool(l, pressed);
	Lua::PushNumber(l, magnitude);
}

//////////////////

CEOnActionInputChanged::CEOnActionInputChanged(Action action, bool b) : action {action}, pressed {b} {}
void CEOnActionInputChanged::PushArguments(lua::State *l)
{
	Lua::PushInt(l, math::to_integral(action));
	Lua::PushBool(l, pressed);
}
