// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.action_input_controller;

export import :entities.components.base;
export import :input.enums;

export namespace pragma {
	namespace actionInputControllerComponent {
		REGISTER_COMPONENT_EVENT(EVENT_HANDLE_ACTION_INPUT)
		REGISTER_COMPONENT_EVENT(EVENT_ON_ACTION_INPUT_CHANGED)
	}
	class DLLNETWORK ActionInputControllerComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ActionInputControllerComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		// Inputs
		void SetActionInput(Action action, bool b, bool bKeepMagnitude);
		void SetActionInput(Action action, bool b, float magnitude = 1.f);
		bool GetActionInput(Action action) const;
		bool GetRawActionInput(Action action) const;
		float GetActionInputAxisMagnitude(Action action) const;
		const std::unordered_map<Action, float> &GetActionInputAxisMagnitudes() const;
		Action GetActionInputs() const;
		Action GetRawActionInputs() const;
		void SetActionInputs(Action action, bool bKeepMagnitudes = false);
		void SetActionInputAxisMagnitude(Action action, float magnitude);
	  private:
		virtual void InitializeLuaObject(lua::State *lua) override;

		// Inputs
		Action m_actionInputs = Action::None;
		Action m_rawInputs = Action::None;
		std::unordered_map<Action, float> m_inputAxes;
	};
	struct DLLNETWORK CEHandleActionInput : public ComponentEvent {
		CEHandleActionInput(Action action, bool pressed, float magnitude);
		virtual void PushArguments(lua::State *l) override;
		Action action;
		bool pressed;
		float magnitude;
	};
	struct DLLNETWORK CEOnActionInputChanged : public ComponentEvent {
		CEOnActionInputChanged(Action action, bool b);
		virtual void PushArguments(lua::State *l) override;
		Action action;
		bool pressed;
	};
};
