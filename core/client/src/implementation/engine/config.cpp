// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :engine;
import pragma.string.unicode;

void CEngine::SaveClientConfig()
{
	FileManager::CreatePath("cfg");
	std::string path = "cfg\\client.cfg";
	auto f = FileManager::OpenFile<VFilePtrReal>(path.c_str(), "w");
	if(f == nullptr) {
		spdlog::warn("Unable to save client.cfg");
		return;
	}
	WriteClientConfig(f);
}

void CEngine::WriteClientConfig(VFilePtrReal f)
{
	f->WriteString("unbindall\n");
	auto inputLayer = GetCoreInputBindingLayer();
	if(inputLayer) {
		for(auto &pair : inputLayer->GetKeyMappings()) {
			auto &kb = pair.second;
			if(kb.GetType() == KeyBind::Type::Regular) {
				std::string key;
				if(KeyToString(pair.first, &key)) {
					std::string l = "bind \"" + key + "\" \"" + kb.GetBind() + "\"\n";
					f->WriteString(l.c_str());
				}
			}
			// Deprecated (Replaced by "toggle" console command")
			/*else if(kb.GetType() == KeyBind::Type::Toggle)
			{
				std::string key;
				if(KeyToString(pair.first,&key))
				{
					std::string l = "bindtoggle \"" +key +"\" \"" +kb.GetBind() +"\"\n";
					f->WriteString(l.c_str());
				}
			}*/
		}
	}
	auto *stateSv = GetServerNetworkState();
	auto *stateCl = GetClientState();
	if(stateCl != nullptr) {
		auto &cvars = stateCl->GetConVars();

		auto &cfg = GetConVarConfig(pragma::NwStateType::Client);
		if(cfg)
			RestoreConVarsForUnknownCommands(f, *cfg, cvars);

		for(auto &pair : cvars) {
			if(stateSv == nullptr || stateSv->GetConVar(pair.first) == nullptr) {
				auto &cf = pair.second;
				if(cf->GetType() == ConType::Var) {
					auto *cv = static_cast<ConVar *>(cf.get());
					if((cv->GetFlags() & pragma::console::ConVarFlags::Archive) == pragma::console::ConVarFlags::Archive && cv->GetString() != cv->GetDefault()) {
						std::string l = pair.first + " \"" + cv->GetString() + "\"\n";
						f->WriteString(l.c_str());
					}
				}
			}
		}
	}
}

void CEngine::LoadConfig()
{
	pragma::Engine::LoadConfig();
	PreloadConfig(pragma::NwStateType::Client, "client.cfg");
}

void CEngine::LoadClientConfig()
{
	auto &cfg = GetConVarConfig(pragma::NwStateType::Client);
	if(cfg)
		ExecCommands(*cfg);
}

void CEngine::PreloadConfig(pragma::NwStateType type, const std::string &configName)
{
	pragma::Engine::PreloadConfig(type, configName);
	if(type != pragma::NwStateType::Client)
		return;
	auto &cfg = GetConVarConfig(pragma::NwStateType::Client);
	if(!cfg)
		return;
	std::string lan = pragma::locale::determine_system_language();
	auto *args = cfg->Find("cl_language");
	if(args && !args->empty())
		lan = args->front();
	pragma::locale::set_language(lan);
	pragma::locale::load("inputs.txt");
	pragma::locale::load("menu.txt");
	pragma::locale::load("misc.txt");
	pragma::locale::load("components.txt");
	pragma::locale::load("prompts.txt");
	pragma::locale::load("shader_materials.txt");

	constexpr auto numBts = umath::to_integral(util::debug::MessageBoxButton::Count);
	std::array<std::string, numBts> labels;
	for(size_t i = 0; i < numBts; ++i) {
		auto identifier = ustring::to_snake_case(std::string {magic_enum::enum_name(static_cast<util::debug::MessageBoxButton>(i))});
		labels[i] = pragma::locale::get_text("prompt_button_" + identifier);
	}
	util::debug::set_button_labels(labels);
}
