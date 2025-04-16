/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/logic/s_logic_relay.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/components/s_toggle_component.hpp"
#include "pragma/entities/components/s_io_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(logic_relay, LogicRelay);

void SLogicRelayComponent::Initialize()
{
	BaseLogicRelayComponent::Initialize();

	BindEvent(SIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "trigger", false))
			Trigger(inputData.activator);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("io");
}

void SLogicRelayComponent::Trigger(BaseEntity *activator)
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

void SLogicRelayComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void LogicRelay::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLogicRelayComponent>();
}
