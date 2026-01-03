// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :core.input_binding_layer;
import :engine;

bool InputBindingLayer::Save(const std::vector<std::shared_ptr<InputBindingLayer>> &layers, udm::AssetDataArg outData, std::string &outErr)
{
	auto udmData = outData.GetData();
	for(auto &layer : layers) {
		if(layer->identifier.empty())
			continue;
		auto udmLayer = udmData[layer->identifier];
		for(auto &pair : layer->GetKeyMappings()) {
			if(pair.second.GetType() != KeyBind::Type::Regular)
				continue;
			std::string key;
			if(!KeyToString(pair.first, &key))
				continue;
			auto &bind = pair.second.GetBind();
			udmLayer[key] = bind;
		}
	}
	return true;
}
bool InputBindingLayer::Load(const udm::AssetData &data, std::vector<std::shared_ptr<InputBindingLayer>> &outLayers, std::string &outErr)
{
	for(auto &pair : data.GetData().ElIt()) {
		auto layer = pragma::util::make_shared<InputBindingLayer>();
		layer->identifier = pair.key;
		for(auto &pair : pair.property.ElIt()) {
			auto val = pair.property.ToValue<udm::String>();
			if(!val.has_value())
				continue;
			short c;
			if(!StringToKey(std::string {pair.key}, &c))
				continue;
			layer->AddKeyMapping(c, *val);
		}
		outLayers.push_back(layer);
	}
	return true;
}
void InputBindingLayer::MapKey(short c, std::string cmd)
{
	UnmapKey(c);
	if(cmd.empty())
		return;
	m_keybinds.insert(std::unordered_map<short, KeyBind>::value_type(c, KeyBind(cmd)));
}

void InputBindingLayer::MapKey(short c, luabind::function<> function)
{
	UnmapKey(c);
	m_keybinds.insert(std::unordered_map<short, KeyBind>::value_type(c, KeyBind(function)));
}

void InputBindingLayer::MapKey(short c, std::unordered_map<std::string, std::vector<std::string>> &binds)
{
	std::stringstream cmdNew;
	for(auto it = binds.begin(); it != binds.end(); ++it) {
		cmdNew << it->first;
		auto &argv = it->second;
		for(auto itArg = argv.begin(); itArg != argv.end(); ++itArg)
			cmdNew << " " << *itArg;
		cmdNew << ";";
	}
	MapKey(c, cmdNew.str());
}

void InputBindingLayer::AddKeyMapping(short c, std::string cmd)
{
	auto it = m_keybinds.find(c);
	if(it == m_keybinds.end()) {
		MapKey(c, cmd);
		return;
	}
	auto &keyBind = it->second;
	auto &bind = keyBind.GetBind();
	std::unordered_map<std::string, std::vector<std::string>> binds;
	auto callback = [&binds](std::string cmd, std::vector<std::string> &argv) {
		auto it = binds.find(cmd);
		if(it != binds.end())
			return;
		binds.insert(std::unordered_map<std::string, std::vector<std::string>>::value_type(cmd, argv)).first;
	};
	pragma::string::get_sequence_commands(cmd, callback);
	pragma::string::get_sequence_commands(bind, callback);
	MapKey(c, binds);
}
void InputBindingLayer::RemoveKeyMapping(short c, std::string cmd)
{
	auto it = m_keybinds.find(c);
	if(it == m_keybinds.end())
		return;
	auto &keyBind = it->second;
	auto &bind = keyBind.GetBind();
	std::unordered_map<std::string, std::vector<std::string>> binds;
	auto callback = [&binds, &cmd](std::string cmdBind, std::vector<std::string> &argv) {
		auto it = binds.find(cmdBind);
		if(it != binds.end() || cmdBind == cmd)
			return;
		binds.insert(std::unordered_map<std::string, std::vector<std::string>>::value_type(cmdBind, argv)).first;
	};
	pragma::string::get_sequence_commands(bind, callback);
	MapKey(c, binds);
}

void InputBindingLayer::UnmapKey(short c)
{
	auto i = m_keybinds.find(c);
	if(i == m_keybinds.end())
		return;
	m_keybinds.erase(i);
}

void InputBindingLayer::ClearLuaKeyMappings()
{
	std::unordered_map<short, KeyBind>::iterator i, j;
	for(i = m_keybinds.begin(); i != m_keybinds.end();) {
		if(i->second.GetType() == KeyBind::Type::Function) {
			j = i;
			++i;
			m_keybinds.erase(j);
		}
		else
			++i;
	}
}

std::unordered_map<short, KeyBind> &InputBindingLayer::GetKeyMappings() { return m_keybinds; }

void InputBindingLayer::ClearKeyMappings() { m_keybinds.clear(); }

std::unordered_map<short, KeyBind> &CoreInputBindingLayer::GetKeyMappings()
{
	pragma::get_cengine()->UpdateDirtyInputBindings();
	return InputBindingLayer::GetKeyMappings();
}
