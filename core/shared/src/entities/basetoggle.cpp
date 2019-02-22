#include "stdafx_shared.h"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/baseentity.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include <algorithm>

using namespace pragma;

ComponentEventId BaseToggleComponent::EVENT_ON_TURN_ON = INVALID_COMPONENT_ID;
ComponentEventId BaseToggleComponent::EVENT_ON_TURN_OFF = INVALID_COMPONENT_ID;
void BaseToggleComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_TURN_ON = componentManager.RegisterEvent("ON_TURN_ON");
	EVENT_ON_TURN_OFF = componentManager.RegisterEvent("ON_TURN_OFF");
}
BaseToggleComponent::BaseToggleComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_bTurnedOn(util::BoolProperty::Create(false))
{}
void BaseToggleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"startdisabled",false))
			m_bStartDisabled = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"enable",false) || ustring::compare(inputData.input,"turnon",false))
			TurnOn();
		else if(ustring::compare(inputData.input,"disable",false) || ustring::compare(inputData.input,"turnoff",false))
			TurnOff();
		else if(ustring::compare(inputData.input,"toggle",false))
			Toggle();
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_netEvToggleState = SetupNetEvent("set_toggle_state");
	GetEntity().AddComponent("io");
}

void BaseToggleComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_bStartDisabled == false && (GetEntity().GetSpawnFlags() &SF_STARTON) == SF_STARTON)
		TurnOn();
}

bool BaseToggleComponent::ToggleInput(std::string input,BaseEntity*,BaseEntity*,std::string data)
{
	if(input == "turnon")
		TurnOn();
	else if(input == "turnoff")
		TurnOff();
	else if(input == "toggle")
		Toggle();
	else
		return false;
	return true;
}

bool BaseToggleComponent::IsTurnedOn() const {return *m_bTurnedOn;}
void BaseToggleComponent::TurnOn()
{
	*m_bTurnedOn = true;

	auto *pIoComponent = static_cast<pragma::BaseIOComponent*>(GetEntity().FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("OnActivate",&GetEntity());

	BroadcastEvent(EVENT_ON_TURN_ON);
}
void BaseToggleComponent::TurnOff()
{
	*m_bTurnedOn = false;

	auto *pIoComponent = static_cast<pragma::BaseIOComponent*>(GetEntity().FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("OnDeactivate",&GetEntity());

	BroadcastEvent(EVENT_ON_TURN_OFF);

}
void BaseToggleComponent::Toggle()
{
	if(*m_bTurnedOn == true)
	{
		TurnOff();
		return;
	}
	TurnOn();
}
const util::PBoolProperty &BaseToggleComponent::GetTurnedOnProperty() const {return m_bTurnedOn;}
void BaseToggleComponent::SetTurnedOn(bool b)
{
	if(*m_bTurnedOn == b)
		return;
	if(b == true)
		TurnOn();
	else
		TurnOff();
}

