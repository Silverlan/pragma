// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :console.register_commands;

pragma::console::DummyReturnValue pragma::console::client::register_command(const std::string &scmd, void (*fc)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), pragma::console::ConVarFlags flags, const std::string &help)
{
	register_concommand(scmd, fc, flags, help);
	return {};
}
pragma::console::DummyReturnValue pragma::console::client::register_command(const std::string &scmd, void (*fc)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), pragma::console::ConVarFlags flags, const std::string &help)
{
	register_concommand(scmd, fc, flags, help);
	return {};
}
pragma::console::DummyReturnValue pragma::console::client::register_toggle_command(const std::string &scmd, void (*fcIn)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float),
  void (*fcOut)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), pragma::console::ConVarFlags flags, const std::string &help)
{
	register_concommand("+" + scmd, fcIn, flags, help);
	register_concommand("-" + scmd, fcOut, flags, help);
	return {};
}
pragma::console::DummyReturnValue pragma::console::client::register_toggle_command(const std::string &scmd, void (*fcIn)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), void (*fcOut)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &),
  pragma::console::ConVarFlags flags, const std::string &help)
{
	register_concommand("+" + scmd, fcIn, flags, help);
	register_concommand("-" + scmd, fcOut, flags, help);
	return {};
}
