/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_GAME_CALLBACK_H__
#define __C_GAME_CALLBACK_H__

#include "pragma/clientdefinitions.h"
#include "pragma/game/game_callback.h"

DECLARE_STATIC_GAME_CALLBACKS(DLLCLIENT,client);
#define ADD_STATIC_CLIENT_GAME_CALLBACK(name,callback) \
	ADD_STATIC_GAME_CALLBACK(name,callback,client);

#endif