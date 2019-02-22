#include "stdafx_client.h"
/*Listener *ClientState::GetListener() {return m_entListener;}

Listener *ClientState::CreateListener()
{
	Listener *listener = GetListener();
	if(listener != NULL)
		return listener;
	m_entListener = CreateEntity<Listener>();
	return m_entListener;
}

std::string Engine::GetLocalPlayerName()
{
	Player *pl = GetLocalPlayer();
	if(pl == NULL)
		return "";
	return pl->GetUserName();
}

void Engine::RemoveSprite(Sprite *spr)
{
	for(int i=0;i<m_sprites.size();i++)
	{
		if(m_sprites[i] == spr)
		{
			delete m_sprites[i];
			m_sprites.erase(m_sprites.begin() +i);
			break;
		}
	}
}

Player *Engine::GetLocalPlayer() {return m_plLocal;}*/