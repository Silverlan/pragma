/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/input_movement_controller_component.hpp"
#include "pragma/entities/components/action_input_controller_component.hpp"
#include "pragma/entities/components/movement_component.hpp"

using namespace pragma;

InputMovementControllerComponent::InputMovementControllerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void InputMovementControllerComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(MovementComponent::EVENT_ON_UPDATE_MOVEMENT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateMovementProperties(); });
}
void InputMovementControllerComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void InputMovementControllerComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

void InputMovementControllerComponent::SetActionInputController(ActionInputControllerComponent *controller) { m_actionInputController = controller ? controller->GetHandle<ActionInputControllerComponent>() : ComponentHandle<ActionInputControllerComponent> {}; }
ActionInputControllerComponent *InputMovementControllerComponent::GetActionInputController() { return m_actionInputController.get(); }

void InputMovementControllerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(MovementComponent))
		m_movementComponent = &static_cast<MovementComponent &>(component);
}
void InputMovementControllerComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(MovementComponent))
		m_movementComponent = nullptr;
}

void InputMovementControllerComponent::UpdateMovementProperties()
{
	if(!IsActive() || m_actionInputController.expired() || !m_movementComponent)
		return;
	m_movementComponent->SetDirectionMagnitude(MovementComponent::MoveDirection::Forward, m_actionInputController->GetActionInputAxisMagnitude(Action::MoveForward));
	m_movementComponent->SetDirectionMagnitude(MovementComponent::MoveDirection::Backward, m_actionInputController->GetActionInputAxisMagnitude(Action::MoveBackward));
	m_movementComponent->SetDirectionMagnitude(MovementComponent::MoveDirection::Left, m_actionInputController->GetActionInputAxisMagnitude(Action::MoveLeft));
	m_movementComponent->SetDirectionMagnitude(MovementComponent::MoveDirection::Right, m_actionInputController->GetActionInputAxisMagnitude(Action::MoveRight));
}
