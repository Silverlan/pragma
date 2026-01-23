// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;
import pragma.string.unicode;

void pragma::CEngine::SaveClientConfig()
{
	fs::create_path("cfg");
	std::string path = "cfg\\client.cfg";
	auto f = fs::open_file<fs::VFilePtrReal>(path, fs::FileMode::Write);
	if(f == nullptr) {
		spdlog::warn("Unable to save client.cfg");
		return;
	}
	WriteClientConfig(f);
}

void pragma::CEngine::WriteClientConfig(fs::VFilePtrReal f)
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

		auto &cfg = GetConVarConfig(NwStateType::Client);
		if(cfg)
			RestoreConVarsForUnknownCommands(f, *cfg, cvars);

		for(auto &pair : cvars) {
			if(stateSv == nullptr || stateSv->GetConVar(pair.first) == nullptr) {
				auto &cf = pair.second;
				if(cf->GetType() == console::ConType::Var) {
					auto *cv = static_cast<console::ConVar *>(cf.get());
					if((cv->GetFlags() & console::ConVarFlags::Archive) == console::ConVarFlags::Archive && cv->GetString() != cv->GetDefault()) {
						std::string l = pair.first + " \"" + cv->GetString() + "\"\n";
						f->WriteString(l.c_str());
					}
				}
			}
		}
	}
}

void pragma::CEngine::LoadConfig()
{
	Engine::LoadConfig();
	PreloadConfig(NwStateType::Client, "client.cfg");
}

void pragma::CEngine::LoadClientConfig()
{
	auto &cfg = GetConVarConfig(NwStateType::Client);
	if(cfg)
		ExecCommands(*cfg);
}

void pragma::CEngine::PreloadConfig(NwStateType type, const std::string &configName)
{
	Engine::PreloadConfig(type, configName);
	if(type != NwStateType::Client)
		return;
	auto &cfg = GetConVarConfig(NwStateType::Client);
	if(!cfg)
		return;
	std::string lan = locale::determine_system_language();
	auto *args = cfg->Find("cl_language");
	if(args && !args->empty())
		lan = args->front();
	locale::set_language(lan);
	locale::load("inputs.txt");
	locale::load("menu.txt");
	locale::load("misc.txt");
	locale::load("components.txt");
	locale::load("prompts.txt");
	locale::load("shader_materials.txt");

	constexpr auto numBts = math::to_integral(debug::MessageBoxButton::Count);
	std::array<std::string, numBts> labels;
	for(size_t i = 0; i < numBts; ++i) {
		auto identifier = string::to_snake_case(std::string {magic_enum::enum_name(static_cast<debug::MessageBoxButton>(i))});
		labels[i] = locale::get_text("prompt_button_" + identifier);
	}
	debug::set_button_labels(labels);
}
