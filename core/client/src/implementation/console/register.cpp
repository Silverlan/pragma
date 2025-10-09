// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :console.register_commands;

std::shared_ptr<ConCommand> pragma::console::client::register_command(const std::string &scmd, void(*fc)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags, const std::string &help,
    void(*autoCompleteCallback)(const std::string &, std::vector<std::string> &))
{

}
std::shared_ptr<ConCommand> pragma::console::client::register_command(const std::string &scmd, void(*fc)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags, const std::string &help,
    void(*autoCompleteCallback)(const std::string &, std::vector<std::string> &, bool))
{
    
}
std::shared_ptr<ConCommand> pragma::console::client::register_command(const ConCommandCreateInfo &createInfo)
{
    
}

CallbackHandle pragma::console::client::register_variable_listener(const std::string &scvar, void(*function)(NetworkState *, const ConVar &, int, int))
{
    
}
CallbackHandle pragma::console::client::register_variable_listener(const std::string &scvar, void(*function)(NetworkState *, const ConVar &, std::string, std::string))
{
    
}
CallbackHandle pragma::console::client::register_variable_listener(const std::string &scvar, void(*function)(NetworkState *, const ConVar &, float, float))
{
    
}
CallbackHandle pragma::console::client::register_variable_listener(const std::string &scvar, void(*function)(NetworkState *, const ConVar &, bool, bool))
{
    
}
