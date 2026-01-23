// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:string.format;

export import std;

export namespace pragma::string {
    std::string DLLNETWORK format_time(double dtm);
}
