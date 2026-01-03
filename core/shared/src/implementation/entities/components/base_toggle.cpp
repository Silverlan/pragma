// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_toggle;

using namespace pragma;

void BaseToggleComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseToggleComponent::EVENT_ON_TURN_ON = registerEvent("ON_TURN_ON", ComponentEventInfo::Type::Broadcast);
	baseToggleComponent::EVENT_ON_TURN_OFF = registerEvent("ON_TURN_OFF", ComponentEventInfo::Type::Broadcast);
}
void BaseToggleComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseToggleComponent;

	{
		using TEnabled = bool;
		auto memberInfo = create_component_member_info<T, TEnabled, static_cast<void (T::*)(TEnabled)>(&T::SetTurnedOn), static_cast<TEnabled (T::*)() const>(&T::IsTurnedOn)>("enabled", false);
		registerMember(std::move(memberInfo));
	}
}
BaseToggleComponent::BaseToggleComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_bTurnedOn(util::BoolProperty::Create(false)) {}
void BaseToggleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "startdisabled", false))
			m_bStartDisabled = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "enable", false) || pragma::string::compare<std::string>(inputData.input, "turnon", false))
			TurnOn();
		else if(pragma::string::compare<std::string>(inputData.input, "disable", false) || pragma::string::compare<std::string>(inputData.input, "turnoff", false))
			TurnOff();
		else if(pragma::string::compare<std::string>(inputData.input, "toggle", false))
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
	if(m_bStartDisabled == false && (GetEntity().GetSpawnFlags() & SF_STARTON) == SF_STARTON)
		TurnOn();
}

void BaseToggleComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["startDisabled"] = m_bStartDisabled;
	udm["isTurnedOn"] = **m_bTurnedOn;
}

void BaseToggleComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	udm["startDisabled"](m_bStartDisabled);
	auto isTurnedOn = IsTurnedOn();
	udm["isTurnedOn"](isTurnedOn);
	SetTurnedOn(isTurnedOn);
}

bool BaseToggleComponent::ToggleInput(std::string input, ecs::BaseEntity *, ecs::BaseEntity *, std::string data)
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

bool BaseToggleComponent::IsTurnedOn() const { return *m_bTurnedOn; }
void BaseToggleComponent::TurnOn()
{
	*m_bTurnedOn = true;

	auto *pIoComponent = static_cast<BaseIOComponent *>(GetEntity().FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("OnActivate", &GetEntity());

	BroadcastEvent(baseToggleComponent::EVENT_ON_TURN_ON);
}
void BaseToggleComponent::TurnOff()
{
	*m_bTurnedOn = false;

	auto *pIoComponent = static_cast<BaseIOComponent *>(GetEntity().FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("OnDeactivate", &GetEntity());

	BroadcastEvent(baseToggleComponent::EVENT_ON_TURN_OFF);
}
void BaseToggleComponent::Toggle()
{
	if(*m_bTurnedOn == true) {
		TurnOff();
		return;
	}
	TurnOn();
}
const util::PBoolProperty &BaseToggleComponent::GetTurnedOnProperty() const { return m_bTurnedOn; }
void BaseToggleComponent::SetTurnedOn(bool b)
{
	if(*m_bTurnedOn == b)
		return;
	if(b == true)
		TurnOn();
	else
		TurnOff();
}
