#ifndef __S_GAME_CALLBACK_H__
#define __S_GAME_CALLBACK_H__

#include "pragma/serverdefinitions.h"
#include "pragma/game/game_callback.h"

DECLARE_STATIC_GAME_CALLBACKS(DLLSERVER,server);
#define ADD_STATIC_CLIENT_GAME_CALLBACK(name,callback) \
	ADD_STATIC_GAME_CALLBACK(name,callback,server);

#endif