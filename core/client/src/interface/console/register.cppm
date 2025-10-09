// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "sharedutils/util.h"
#include <string>

export module pragma.client:console.register_commands;

export import pragma.shared;

export namespace pragma::console {
    namespace client {
        using DummyReturnValue = bool;
        template<typename T>
            std::shared_ptr<ConVar> register_variable(const std::string &scmd, const T &value, ConVarFlags flags, const std::string &help = "", const std::optional<std::string> &usageHelp = {}, void(*autoCompleteFunction)(const std::string &, std::vector<std::string> &, bool) = nullptr);
        
        DummyReturnValue register_command(const std::string &scmd, void(*fc)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags, const std::string &help = "",
            void(*autoCompleteCallback)(const std::string &, std::vector<std::string> &, bool) = nullptr);
        DummyReturnValue register_toggle_command(const std::string &scmd, void(*fcIn)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), void(*fcOut)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags, const std::string &help = "",
            void(*autoCompleteCallback)(const std::string &, std::vector<std::string> &, bool) = nullptr);

        template<typename T> requires(util::is_of_type<T, int, float, bool, std::string>)
            DummyReturnValue register_variable_listener(const std::string &scvar, void(*function)(NetworkState *, const ConVar &, T, T));
    }
};
