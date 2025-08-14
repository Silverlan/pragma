// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_LUA_RUN_AUTOCOMPLETE_HPP__
#define __PRAGMA_LUA_RUN_AUTOCOMPLETE_HPP__

#include "pragma/networkdefinitions.h"

namespace pragma::console::impl {
    void lua_run_autocomplete(lua_State *l, const std::string &arg, std::vector<std::string> &autoCompleteOptions);
};

#endif
