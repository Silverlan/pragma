// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_io;

using namespace pragma;

void BaseIOComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseIOComponent::EVENT_HANDLE_INPUT = registerEvent("HANDLE_INPUT", ComponentEventInfo::Type::Broadcast); }

BaseIOComponent::BaseIOComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseIOComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(ecs::baseEntity::EVENT_ON_REMOVE, [this](std::reference_wrapper<ComponentEvent> evData) { TriggerOutput("OnRemove", &GetEntity()); });
	BindEventUnhandled(ecs::baseEntity::EVENT_ON_SPAWN, [this](std::reference_wrapper<ComponentEvent> evData) { TriggerOutput("OnSpawn", &GetEntity()); });
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

void BaseIOComponent::Input(const std::string input, ecs::BaseEntity *activator, ecs::BaseEntity *caller) { Input(input, activator, caller, ""); }

void BaseIOComponent::StoreOutput(std::string name, std::string info)
{
	string::to_lower(name);
	std::vector<std::string> data;
	string::explode(info, ",", data);
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
	string::to_lower(name);
	std::unordered_map<std::string, std::vector<Output>>::iterator it = m_outputs.find(name);
	if(it == m_outputs.end())
		it = m_outputs.insert(std::unordered_map<std::string, std::vector<Output>>::value_type(name, std::vector<Output>())).first;
	it->second.push_back(Output(entities, input, param, delay, times));
}

bool BaseIOComponent::FireSingleOutput(Output &output, ecs::BaseEntity *activator, IoFlags flags)
{
	if(output.times == 0)
		return false;
	auto &entThis = GetEntity();
	auto *state = entThis.GetNetworkState();
	Game *game = state->GetGameState();
	std::vector<ecs::BaseEntity *> ents;
	if(output.entities == "!activator")
		ents.push_back(activator);
	else if(output.entities == "!self")
		ents.push_back(&entThis);
	else if(output.entities == "!player")
		game->GetPlayers(&ents);
	else {
		std::string className = output.entities;
		string::to_lower(className);
		ecs::EntityIterator entIt {*game};
		entIt.AttachFilter<EntityIteratorFilterUser>([&className, &output](ecs::BaseEntity &ent, std::size_t index) -> bool {
			if(string::compare(ent.GetClass().c_str(), className.c_str(), false))
				return true;
			auto pNameComponent = static_cast<BaseNameComponent *>(ent.FindComponent("name").get());
			return pNameComponent != nullptr && string::compare(pNameComponent->GetName(), output.entities, false);
		});
		for(auto *ent : entIt)
			ents.push_back(ent);
	}
	EntityHandle hThis = entThis.GetHandle();
	for(auto *ent : ents) {
		auto *pIoComponent = static_cast<BaseIOComponent *>(ent->FindComponent("io").get());
		if(pIoComponent == nullptr)
			continue;
		if(output.delay <= 0.f && !math::is_flag_set(flags, IoFlags::ForceDelayedFire)) {
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
				      ecs::BaseEntity *ent = hEnt.get();
				      auto *pIoComponent = static_cast<BaseIOComponent *>(ent->FindComponent("io").get());
				      if(pIoComponent == nullptr)
					      return;
				      ecs::BaseEntity *entActivator = nullptr;
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

void BaseIOComponent::Input(std::string input, ecs::BaseEntity *activator, ecs::BaseEntity *caller, std::string data)
{
	string::to_lower(input);

	CEInputData inputData {input, activator, caller, data};
	if(BroadcastEvent(baseIOComponent::EVENT_HANDLE_INPUT, inputData) == util::EventReply::Handled)
		return;
	auto &entThis = GetEntity();
	if(input == "addoutput") {
		size_t sp = data.find_first_of(string::WHITESPACE);
		if(sp != string::NOT_FOUND) {
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
		Con::COUT << "WARNING: Unhandled input '" << input << "' for entity '" << entThis.GetClass() << "'!" << Con::endl;
}

void BaseIOComponent::TriggerOutput(std::string name, ecs::BaseEntity *activator, IoFlags flags)
{
	string::to_lower(name);
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

CEInputData::CEInputData(const std::string &input, ecs::BaseEntity *activator, ecs::BaseEntity *caller, const std::string &data) : input(input), activator(activator), caller(caller), data(data) {}
void CEInputData::PushArguments(lua::State *l)
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
