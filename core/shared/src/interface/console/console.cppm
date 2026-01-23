// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:console;

export import :console.command_options;
export import :console.commands;
export import :console.common;
export import :console.convar;
export import :console.convar_handle;
export import :console.cvar_callback;
export import :console.cvar_handler;
export import :console.debug_console;
export import :console.enums;
export import :console.find_entities;
export import :console.line_noise;
export import :console.output;
export import :console.spdlog_anycolor_sink;

export namespace pragma::console {
    PR_EXPORT void register_shared_convars(ConVarMap &map);
}
