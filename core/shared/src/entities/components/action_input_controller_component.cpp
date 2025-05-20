/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/action_input_controller_component.hpp"

using namespace pragma;

ComponentEventId ActionInputControllerComponent::EVENT_HANDLE_ACTION_INPUT = pragma::INVALID_COMPONENT_ID;
ComponentEventId ActionInputControllerComponent::EVENT_ON_ACTION_INPUT_CHANGED = pragma::INVALID_COMPONENT_ID;
void ActionInputControllerComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_HANDLE_ACTION_INPUT = registerEvent("HANDLE_ACTION_INPUT", ComponentEventInfo::Type::Explicit);
	EVENT_ON_ACTION_INPUT_CHANGED = registerEvent("ON_ACTION_INPUT_CHANGED", ComponentEventInfo::Type::Explicit);
}

ActionInputControllerComponent::ActionInputControllerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ActionInputControllerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void ActionInputControllerComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
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
	auto valuesOld = umath::get_power_of_2_values(umath::to_integral(rawInputs));
	for(auto v : valuesOld) {
		if((actions & static_cast<Action>(v)) == Action::None) // Action has been unpressed
			SetActionInput(static_cast<Action>(v), false);
	}
	actions &= ~rawInputs;
	auto values = umath::get_power_of_2_values(umath::to_integral(actions));
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
	if(InvokeEventCallbacks(EVENT_HANDLE_ACTION_INPUT, evData) == util::EventReply::Handled)
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
			InvokeEventCallbacks(EVENT_ON_ACTION_INPUT_CHANGED, CEOnActionInputChanged {action, b});
		}
		return;
	}
	if(GetActionInput(action))
		return;
	m_actionInputs |= action;
	InvokeEventCallbacks(EVENT_ON_ACTION_INPUT_CHANGED, CEOnActionInputChanged {action, b});
}

//////////////////

CEHandleActionInput::CEHandleActionInput(Action action, bool pressed, float magnitude) : action {action}, pressed {pressed}, magnitude {magnitude} {}
void CEHandleActionInput::PushArguments(lua_State *l)
{
	Lua::PushInt(l, umath::to_integral(action));
	Lua::PushBool(l, pressed);
	Lua::PushNumber(l, magnitude);
}

//////////////////

CEOnActionInputChanged::CEOnActionInputChanged(Action action, bool b) : action {action}, pressed {b} {}
void CEOnActionInputChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l, umath::to_integral(action));
	Lua::PushBool(l, pressed);
}
