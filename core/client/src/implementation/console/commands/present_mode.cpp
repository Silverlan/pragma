// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :console.commands;

namespace {
	auto _ = pragma::console::client::register_variable_listener<int32_t>("cl_render_present_mode", +[](pragma::NetworkState *state, const pragma::console::ConVar &, int32_t, int32_t val) { pragma::get_cengine()->GetWindow().SetPresentMode(static_cast<prosper::PresentModeKHR>(val)); });
}
