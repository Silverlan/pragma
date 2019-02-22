#include "stdafx_server.h"
#include "pragma/lua/classes/s_logic_relay.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/components/s_toggle_component.hpp"
#include "pragma/entities/components/s_io_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(logic_relay,LogicRelay);

void SLogicRelayComponent::Initialize()
{
	BaseLogicRelayComponent::Initialize();

	BindEvent(SIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"trigger",false))
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
		pIoComponent->TriggerOutput("OnTrigger",activator);
}

//////////////

luabind::object SLogicRelayComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SLogicRelayComponentHandleWrapper>(l);}

void LogicRelay::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLogicRelayComponent>();
}
