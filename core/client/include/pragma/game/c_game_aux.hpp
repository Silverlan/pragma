/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_GAME_AUX_HPP__
#define __C_GAME_AUX_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/game/c_game.h"

extern DLLCENGINE CEngine *c_engine;

template<class TEfxProperties>
	std::shared_ptr<al::Effect> CGame::CreateAuxEffect(const std::string &name,const TEfxProperties &props)
{
	return c_engine->CreateAuxEffect(name,props);
}

#endif
