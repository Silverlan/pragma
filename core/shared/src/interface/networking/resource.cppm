// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.resource;

export import std;

export namespace pragma::networking {
    DLLNETWORK bool is_valid_resource(std::string res);
}
