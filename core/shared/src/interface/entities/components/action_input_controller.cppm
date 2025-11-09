// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.action_input_controller;

export import :entities.components.base;
export import :input.enums;

export namespace pragma {
	namespace actionInputControllerComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_HANDLE_ACTION_INPUT;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_ACTION_INPUT_CHANGED;
	}
	class DLLNETWORK ActionInputControllerComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ActionInputControllerComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		// Inputs
		void SetActionInput(pragma::Action action, bool b, bool bKeepMagnitude);
		void SetActionInput(pragma::Action action, bool b, float magnitude = 1.f);
		bool GetActionInput(pragma::Action action) const;
		bool GetRawActionInput(pragma::Action action) const;
		float GetActionInputAxisMagnitude(pragma::Action action) const;
		const std::unordered_map<pragma::Action, float> &GetActionInputAxisMagnitudes() const;
		pragma::Action GetActionInputs() const;
		pragma::Action GetRawActionInputs() const;
		void SetActionInputs(pragma::Action action, bool bKeepMagnitudes = false);
		void SetActionInputAxisMagnitude(pragma::Action action, float magnitude);
	  private:
		virtual void InitializeLuaObject(lua::State *lua) override;

		// Inputs
		pragma::Action m_actionInputs = pragma::Action::None;
		pragma::Action m_rawInputs = pragma::Action::None;
		std::unordered_map<pragma::Action, float> m_inputAxes;
	};
	struct DLLNETWORK CEHandleActionInput : public ComponentEvent {
		CEHandleActionInput(pragma::Action action, bool pressed, float magnitude);
		virtual void PushArguments(lua::State *l) override;
		pragma::Action action;
		bool pressed;
		float magnitude;
	};
	struct DLLNETWORK CEOnActionInputChanged : public ComponentEvent {
		CEOnActionInputChanged(pragma::Action action, bool b);
		virtual void PushArguments(lua::State *l) override;
		pragma::Action action;
		bool pressed;
	};
};
