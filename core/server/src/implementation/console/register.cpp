// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;

import :console.register_commands;

pragma::console::DummyReturnValue pragma::console::server::register_command(const std::string &scmd, void (*fc)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), pragma::console::ConVarFlags flags, const std::string &help)
{
	console_system::server::register_concommand(scmd, fc, flags, help);
	return {};
}
pragma::console::DummyReturnValue pragma::console::server::register_command(const std::string &scmd, void (*fc)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), pragma::console::ConVarFlags flags, const std::string &help)
{
	console_system::server::register_concommand(scmd, fc, flags, help);
	return {};
}
pragma::console::DummyReturnValue pragma::console::server::register_toggle_command(const std::string &scmd, void (*fcIn)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float),
  void (*fcOut)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), pragma::console::ConVarFlags flags, const std::string &help)
{
	console_system::server::register_concommand("+" + scmd, fcIn, flags, help);
	console_system::server::register_concommand("-" + scmd, fcOut, flags, help);
	return {};
}
pragma::console::DummyReturnValue pragma::console::server::register_toggle_command(const std::string &scmd, void (*fcIn)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), void (*fcOut)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &),
  pragma::console::ConVarFlags flags, const std::string &help)
{
	console_system::server::register_concommand("+" + scmd, fcIn, flags, help);
	console_system::server::register_concommand("-" + scmd, fcOut, flags, help);
	return {};
}
