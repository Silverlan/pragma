// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.input_movement_controller;

using namespace pragma;

InputMovementControllerComponent::InputMovementControllerComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void InputMovementControllerComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(movementComponent::EVENT_ON_UPDATE_MOVEMENT, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateMovementProperties(); });
}
void InputMovementControllerComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
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
