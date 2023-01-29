/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/model/brush/brushmesh.h"

// TODO: Remove this file
bool Game::CollisionTest()
{
	if(!m_bCollisionsEnabled)
		return false;
	return false;
	//Player *pl = GetLocalPlayer();
	//if(pl == NULL) return false;
	//return CollisionTest(pl->GetPosition());
}

void Game::EnableCollisions(bool b) { m_bCollisionsEnabled = b; }

void Game::CollisionTest(BaseEntity *a, BaseEntity *b)
{
	// TODO: Remove me
}

bool Game::CollisionTest(pragma::BasePlayerComponent &, float *, Vector3 *)
{
	// TODO: Remove me
	return false;
}

bool Game::CollisionTest(Vector3 *)
{
	// TODO: Remove me
	return false;
}
