// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.common;

extern pragma::Engine *engine;
pragma::console::ConVarHandle pragma::console::get_con_var(std::string_view scmd) { return Engine::GetConVarHandle(scmd); }
pragma::console::ConVarHandle pragma::console::get_engine_con_var(std::string_view scmd) { return Engine::GetConVarHandle(scmd); }

size_t pragma::console::get_con_var_value_bytes(CVarHandler &cvarHandler, std::string_view scmd, std::string_view defaultValue)
{
	auto bytes = util::parse_bytes(cvarHandler.GetConVarValueOr<udm::String>(scmd, std::string {defaultValue}, true));
	if(!bytes)
		bytes = util::parse_bytes(defaultValue);
	if(!bytes)
		throw std::runtime_error {"Invalid format for bytes input " + std::string {defaultValue}};
	return *bytes;
}
