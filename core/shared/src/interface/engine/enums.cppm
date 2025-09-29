// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>

export module pragma.shared:engine.enums;

export {
	enum class NwStateType : uint8_t { Client = 0, Server, Count };
};
