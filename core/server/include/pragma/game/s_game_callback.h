/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_GAME_CALLBACK_H__
#define __S_GAME_CALLBACK_H__

#include "pragma/serverdefinitions.h"
#include "pragma/game/game_callback.h"

DECLARE_STATIC_GAME_CALLBACKS(DLLSERVER,server);
#define ADD_STATIC_CLIENT_GAME_CALLBACK(name,callback) \
	ADD_STATIC_GAME_CALLBACK(name,callback,server);

#endif