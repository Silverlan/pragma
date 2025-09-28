// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_GAME_CALLBACK_H__
#define __C_GAME_CALLBACK_H__


DECLARE_STATIC_GAME_CALLBACKS(DLLCLIENT, client);
#define ADD_STATIC_CLIENT_GAME_CALLBACK(name, callback) ADD_STATIC_GAME_CALLBACK(name, callback, client);

#endif
