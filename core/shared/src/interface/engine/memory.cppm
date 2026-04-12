// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.shared:engine.memory;

export import std.compat;

export namespace pragma {
	DLLNETWORK size_t get_peak_rss();
	DLLNETWORK size_t get_current_rss();
};
