#ifndef __GAME_CLEAR_RESOURCES_H__
#define __GAME_CLEAR_RESOURCES_H__

#include <pragma/game/game.h>
#include <materialmanager.h>

template<class TModelManager>
	void Game::ClearResources()
{
	TModelManager::ClearMarkedModels();
	GetNetworkState()->GetMaterialManager().ClearUnused();
}

#endif
