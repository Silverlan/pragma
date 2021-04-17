/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include <prosper_window.hpp>

extern DLLCLIENT CEngine *c_engine;

REGISTER_CONVAR_CALLBACK_CL(cl_render_present_mode,[](NetworkState *state,ConVar*,int32_t,int32_t val) {
	c_engine->GetWindow().SetPresentMode(static_cast<prosper::PresentModeKHR>(val));
});
