/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_observable_component.hpp"
#include "pragma/entities/components/s_observer_component.hpp"
#include "pragma/entities/components/s_generic_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/action_input_controller_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

SAIComponent::ControlInfo::ControlInfo() : hController {}, hCbOnRemove {}, hCbOnKilled {}, hCbOnActionInput {} {}

void SAIComponent::ControlInfo::Clear()
{
	hController = {};
	if(hCbOnRemove.IsValid())
		hCbOnRemove.Remove();
	if(hCbOnKilled.IsValid())
		hCbOnKilled.Remove();
	if(hCbOnActionInput.IsValid())
		hCbOnActionInput.Remove();
}

void SAIComponent::SelectControllerSchedule() { BroadcastEvent(EVENT_SELECT_CONTROLLER_SCHEDULE); }

bool SAIComponent::IsControllable() const { return m_bControllable; }
void SAIComponent::SetControllable(bool b)
{
	m_bControllable = b;
	if(b == false)
		EndControl();
}
void SAIComponent::StartControl(pragma::SPlayerComponent &pl)
{
	if(IsControllable() == false || IsControlled() == true)
		return;
	auto &plEnt = pl.GetEntity();
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(plEnt.GetCharacterComponent().get());
	if(charComponent != nullptr)
		charComponent->SetNoTarget(true);
	CancelSchedule();
	m_controlInfo.actions = Action::None; // We want to overwrite the player's controls, so we need to keep track of them ourselves
	auto pGenericComponent = plEnt.GetComponent<SGenericComponent>();
	if(pGenericComponent.valid()) {
		m_controlInfo.hCbOnRemove = pGenericComponent->BindEventUnhandled(BaseEntity::EVENT_ON_REMOVE, std::bind(&SAIComponent::EndControl, this));
		m_controlInfo.hCbOnKilled = pGenericComponent->BindEventUnhandled(SCharacterComponent::EVENT_ON_DEATH, std::bind(&SAIComponent::EndControl, this));
	}
	auto *actionInputC = pl.GetActionInputController();
	if(actionInputC) {
		m_controlInfo.hCbOnActionInput = actionInputC->BindEvent(ActionInputControllerComponent::EVENT_HANDLE_ACTION_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &actionInputData = static_cast<CEHandleActionInput &>(evData.get());
			if(actionInputData.pressed == true) {
				if((m_controlInfo.actions & actionInputData.action) != Action::None)
					return util::EventReply::Handled;
				m_controlInfo.actions |= actionInputData.action;
			}
			else {
				if((m_controlInfo.actions & actionInputData.action) == Action::None)
					return util::EventReply::Handled;
				m_controlInfo.actions &= ~actionInputData.action;
			}
			OnControllerActionInput(actionInputData.action, actionInputData.pressed);
			return util::EventReply::Handled;
		});
	}
	m_controlInfo.hController = util::WeakHandle<pragma::SPlayerComponent>(std::static_pointer_cast<pragma::SPlayerComponent>(pl.shared_from_this()));

	auto pObsComponent = GetEntity().GetComponent<pragma::SObservableComponent>();
	if(pObsComponent.valid()) {
		auto observerC = pl.GetEntity().GetComponent<pragma::SObserverComponent>();
		if(observerC.valid()) {
			observerC->SetObserverMode(ObserverMode::ThirdPerson);
			observerC->SetObserverTarget(pObsComponent.get());
		}
	}
	DisableAI();
	OnStartControl(pl);
}
Action SAIComponent::GetControllerActionInput() const { return m_controlInfo.actions; }
void SAIComponent::OnControllerActionInput(Action action, bool pressed)
{
	CEOnControllerActionInput evData {action, pressed};
	BroadcastEvent(EVENT_ON_CONTROLLER_ACTION_INPUT);
}
void SAIComponent::OnStartControl(pragma::SPlayerComponent &pl)
{
	CEOnStartControl evData {pl};
	BroadcastEvent(EVENT_ON_START_CONTROL, evData);
}
void SAIComponent::OnEndControl() { BroadcastEvent(EVENT_ON_END_CONTROL); }
void SAIComponent::EndControl()
{
	if(IsControlled() == false)
		return;
	auto *pl = m_controlInfo.hController.get();
	if(pl != nullptr) {
		auto *charComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
		if(charComponent != nullptr)
			charComponent->SetNoTarget(false);
		auto pObservableComponent = pl->GetEntity().GetComponent<SObservableComponent>();
		if(pObservableComponent.valid()) {
			auto observerC = pl->GetEntity().GetComponent<pragma::SObserverComponent>();
			if(observerC.valid()) {
				observerC->SetObserverMode(ObserverMode::FirstPerson);
				observerC->SetObserverTarget(pObservableComponent.get());
			}
		}
	}
	OnEndControl();
	EnableAI();
	m_controlInfo.Clear();
}
bool SAIComponent::IsControlled() const { return m_controlInfo.hController.expired() == false; }
pragma::SPlayerComponent *SAIComponent::GetController() const { return m_controlInfo.hController.get(); }
