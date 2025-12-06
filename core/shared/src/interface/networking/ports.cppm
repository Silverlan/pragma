// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.shared:networking.ports;

export import std;

export namespace pragma::networking {
	DLLNETWORK const std::string DEFAULT_PORT_TCP = "29150";
	DLLNETWORK const std::string DEFAULT_PORT_UDP = "29150";
};
