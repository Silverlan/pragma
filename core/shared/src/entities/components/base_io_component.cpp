/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/lua/libraries/ltimer.h"
#include <pragma/game/game.h>
#include <udm.hpp>

using namespace pragma;

ComponentEventId BaseIOComponent::EVENT_HANDLE_INPUT = pragma::INVALID_COMPONENT_ID;
void BaseIOComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_HANDLE_INPUT = registerEvent("HANDLE_INPUT", ComponentEventInfo::Type::Broadcast); }

BaseIOComponent::BaseIOComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseIOComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BaseEntity::EVENT_ON_REMOVE, [this](std::reference_wrapper<ComponentEvent> evData) { TriggerOutput("OnRemove", &GetEntity()); });
	BindEventUnhandled(BaseEntity::EVENT_ON_SPAWN, [this](std::reference_wrapper<ComponentEvent> evData) { TriggerOutput("OnSpawn", &GetEntity()); });
}

void BaseIOComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	auto udmOutputs = udm["outputs"];
	for(auto &pair : m_outputs) {
		auto &outputs = pair.second;
		auto udmOutputList = udmOutputs.AddArray(pair.first, outputs.size());
		for(auto i = decltype(outputs.size()) {0u}; i < outputs.size(); ++i) {
			auto &output = outputs[i];
			auto udmOutput = udmOutputList[i];
			udmOutput["entities"] = output.entities;
			udmOutput["input"] = output.input;
			udmOutput["param"] = output.param;
			udmOutput["delay"] = output.delay;
			udmOutput["times"] = output.times;
		}
	}
}
void BaseIOComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto udmOutputs = udm["outputs"];
	auto numOutputs = udmOutputs.GetChildCount();
	for(auto udmOutputList : udmOutputs.ElIt()) {
		auto &outputs = m_outputs[std::string {udmOutputList.key}] = {};
		auto udmOutputs = udmOutputList.property;
		outputs.resize(udmOutputs.GetSize());
		for(auto i = decltype(outputs.size()) {0u}; i < outputs.size(); ++i) {
			auto &output = outputs[i];
			auto udmOutput = udmOutputs[i];
			udmOutput["entities"](output.entities);
			udmOutput["input"](output.input);
			udmOutput["param"](output.param);
			udmOutput["delay"](output.delay);
			udmOutput["times"](output.times);
		}
	}
}

void BaseIOComponent::Input(const std::string input, BaseEntity *activator, BaseEntity *caller) { Input(input, activator, caller, ""); }

void BaseIOComponent::StoreOutput(std::string name, std::string info)
{
	StringToLower(name);
	std::vector<std::string> data;
	ustring::explode(info, ",", data);
	auto numData = data.size();
	if(numData < 2)
		return;
	Output output;
	output.entities = data[0];
	output.input = data[1];
	if(numData > 2) {
		output.param = data[2];
		if(numData > 3) {
			output.delay = util::to_float(data[3]);
			if(numData > 4)
				output.times = util::to_int(data[4]);
		}
	}
	std::unordered_map<std::string, std::vector<Output>>::iterator it = m_outputs.find(name);
	if(it == m_outputs.end())
		it = m_outputs.insert(std::unordered_map<std::string, std::vector<Output>>::value_type(name, std::vector<Output>())).first;
	it->second.push_back(output);
}

void BaseIOComponent::StoreOutput(std::string name, std::string entities, std::string input, std::string param, float delay, int times)
{
	StringToLower(name);
	std::unordered_map<std::string, std::vector<Output>>::iterator it = m_outputs.find(name);
	if(it == m_outputs.end())
		it = m_outputs.insert(std::unordered_map<std::string, std::vector<Output>>::value_type(name, std::vector<Output>())).first;
	it->second.push_back(Output(entities, input, param, delay, times));
}

bool BaseIOComponent::FireSingleOutput(Output &output, BaseEntity *activator, IoFlags flags)
{
	if(output.times == 0)
		return false;
	auto &entThis = GetEntity();
	NetworkState *state = entThis.GetNetworkState();
	Game *game = state->GetGameState();
	std::vector<BaseEntity *> ents;
	if(output.entities == "!activator")
		ents.push_back(activator);
	else if(output.entities == "!self")
		ents.push_back(&entThis);
	else if(output.entities == "!player")
		game->GetPlayers(&ents);
	else {
		std::string className = output.entities;
		ustring::to_lower(className);
		EntityIterator entIt {*game};
		entIt.AttachFilter<EntityIteratorFilterUser>([&className, &output](BaseEntity &ent, std::size_t index) -> bool {
			if(ustring::compare(ent.GetClass().c_str(), className.c_str(), false))
				return true;
			auto pNameComponent = static_cast<pragma::BaseNameComponent *>(ent.FindComponent("name").get());
			return pNameComponent != nullptr && ustring::compare(pNameComponent->GetName(), output.entities, false);
		});
		for(auto *ent : entIt)
			ents.push_back(ent);
	}
	EntityHandle hThis = entThis.GetHandle();
	for(auto *ent : ents) {
		auto *pIoComponent = static_cast<BaseIOComponent *>(ent->FindComponent("io").get());
		if(pIoComponent == nullptr)
			continue;
		if(output.delay <= 0.f && !umath::is_flag_set(flags, IoFlags::ForceDelayedFire)) {
			pIoComponent->Input(output.input, activator, &entThis, output.param);
			if(!hThis.valid())
				return false;
		}
		else {
			game
			  ->CreateTimer(output.delay, 1,
			    FunctionCallback<>::Create(std::bind(
			      [](EntityHandle &hEnt, EntityHandle activator, Output output) {
				      if(!hEnt.valid())
					      return;
				      BaseEntity *ent = hEnt.get();
				      auto *pIoComponent = static_cast<BaseIOComponent *>(ent->FindComponent("io").get());
				      if(pIoComponent == nullptr)
					      return;
				      BaseEntity *entActivator = NULL;
				      if(activator.valid())
					      entActivator = activator.get();
				      pIoComponent->Input(output.input, entActivator, ent, output.param);
			      },
			      ent->GetHandle(), activator->GetHandle(), output)))
			  ->Start(game);
		}
	}
	if(output.times != -1)
		output.times -= 1;
	return ((output.times > 0) || (output.times == -1)) ? true : false;
}

void BaseIOComponent::Input(std::string input, BaseEntity *activator, BaseEntity *caller, std::string data)
{
	StringToLower(input);

	pragma::CEInputData inputData {input, activator, caller, data};
	if(BroadcastEvent(EVENT_HANDLE_INPUT, inputData) == util::EventReply::Handled)
		return;
	auto &entThis = GetEntity();
	if(input == "addoutput") {
		size_t sp = data.find_first_of(ustring::WHITESPACE);
		if(sp != ustring::NOT_FOUND) {
			std::string output = data.substr(0, sp);
			std::string info = data.substr(sp + 1, data.length());
			StoreOutput(output, info);
		}
	}
	else if(input == "remove")
		entThis.RemoveSafely();
	else if(input == "fireuser1")
		TriggerOutput("onuser1", activator);
	else if(input == "fireuser2")
		TriggerOutput("onuser2", activator);
	else if(input == "fireuser3")
		TriggerOutput("onuser3", activator);
	else if(input == "fireuser4")
		TriggerOutput("onuser4", activator);
	else
		Con::cout << "WARNING: Unhandled input '" << input << "' for entity '" << entThis.GetClass() << "'!" << Con::endl;
}

void BaseIOComponent::TriggerOutput(std::string name, BaseEntity *activator, IoFlags flags)
{
	StringToLower(name);
	std::unordered_map<std::string, std::vector<Output>>::iterator it = m_outputs.find(name);
	if(it == m_outputs.end())
		return;
	std::vector<Output> &outputs = it->second;
	EntityHandle hActivator = activator->GetHandle();
	for(auto i = outputs.size() - 1; i != size_t(-1); i--) {
		Output &output = outputs[i];
		if(!FireSingleOutput(output, activator, flags)) {
			if(!hActivator.valid())
				return;
			outputs.erase(outputs.begin() + i);
		}
		else if(!hActivator.valid())
			return;
	}
	if(outputs.empty())
		m_outputs.erase(it);
}

/////////////////

CEInputData::CEInputData(const std::string &input, BaseEntity *activator, BaseEntity *caller, const std::string &data) : input(input), activator(activator), caller(caller), data(data) {}
void CEInputData::PushArguments(lua_State *l)
{
	Lua::PushString(l, input);
	if(activator != nullptr)
		activator->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
	if(caller != nullptr)
		caller->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
	Lua::PushString(l, data);
}
