/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __ACTION_INPUT_CONTROLLER_COMPONENT_HPP__
#define __ACTION_INPUT_CONTROLLER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/input/inkeys.h"

namespace pragma {
	class DLLNETWORK ActionInputControllerComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_HANDLE_ACTION_INPUT;
		static ComponentEventId EVENT_ON_ACTION_INPUT_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ActionInputControllerComponent(BaseEntity &ent);
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
		virtual void InitializeLuaObject(lua_State *lua) override;

		// Inputs
		Action m_actionInputs = Action::None;
		Action m_rawInputs = Action::None;
		std::unordered_map<Action, float> m_inputAxes;
	};
	struct DLLNETWORK CEHandleActionInput : public ComponentEvent {
		CEHandleActionInput(Action action, bool pressed, float magnitude);
		virtual void PushArguments(lua_State *l) override;
		Action action;
		bool pressed;
		float magnitude;
	};
	struct DLLNETWORK CEOnActionInputChanged : public ComponentEvent {
		CEOnActionInputChanged(Action action, bool b);
		virtual void PushArguments(lua_State *l) override;
		Action action;
		bool pressed;
	};
};

#endif
