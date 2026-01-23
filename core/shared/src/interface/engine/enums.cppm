// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:engine.enums;

export import std.compat;

export namespace pragma {
	enum class NwStateType : uint8_t { Client = 0, Server, Count };
};
