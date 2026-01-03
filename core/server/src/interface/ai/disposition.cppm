// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.disposition;

export import std.compat;

export {
	enum class DISPOSITION : uint32_t { HATE, FEAR, NEUTRAL, LIKE, COUNT };
};
