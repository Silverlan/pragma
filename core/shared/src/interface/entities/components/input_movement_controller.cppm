// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.input_movement_controller;

export import :entities.components.base;

export namespace pragma {
	class ActionInputControllerComponent;
	class MovementComponent;
	class DLLNETWORK InputMovementControllerComponent final : public BaseEntityComponent {
	  public:
		InputMovementControllerComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		void SetActionInputController(ActionInputControllerComponent *controller);
		ActionInputControllerComponent *GetActionInputController();
		const ActionInputControllerComponent *GetActionInputController() const { return const_cast<InputMovementControllerComponent *>(this)->GetActionInputController(); }
	  private:
		virtual void InitializeLuaObject(lua::State *lua) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		void UpdateMovementProperties();
		ComponentHandle<ActionInputControllerComponent> m_actionInputController {};
		MovementComponent *m_movementComponent = nullptr;
	};
};
