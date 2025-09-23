// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include <prosper_window.hpp>

import pragma.client;


REGISTER_CONVAR_CALLBACK_CL(cl_render_present_mode, [](NetworkState *state, const ConVar &, int32_t, int32_t val) { pragma::get_cengine()->GetWindow().SetPresentMode(static_cast<prosper::PresentModeKHR>(val)); });
