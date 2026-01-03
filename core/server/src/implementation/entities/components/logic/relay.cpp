// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.logic.relay;

import :entities.components;

using namespace pragma;

void SLogicRelayComponent::Initialize()
{
	BaseLogicRelayComponent::Initialize();

	BindEvent(sIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "trigger", false))
			Trigger(inputData.activator);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("io");
}

void SLogicRelayComponent::Trigger(ecs::BaseEntity *activator)
{
	auto &ent = GetEntity();
	auto ptrToggleComponent = ent.GetComponent<SToggleComponent>();
	if(ptrToggleComponent.valid() && ptrToggleComponent->IsTurnedOn() == false)
		return;
	auto pIoComponent = ent.GetComponent<SIOComponent>();
	if(pIoComponent.valid())
		pIoComponent->TriggerOutput("OnTrigger", activator);
}

//////////////

void SLogicRelayComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void LogicRelay::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLogicRelayComponent>();
}
