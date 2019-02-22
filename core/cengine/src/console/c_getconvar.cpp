#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <pragma/serverstate/serverstate.h>
#include <pragma/physics/physobj.h>
#include "pragma/entities/components/c_player_component.hpp"
#include <pragma/console/convars.h>

ConConf *CEngine::GetConVar(const std::string &cv)
{
	auto *cvar = Engine::GetConVar(cv);
	if(cvar != nullptr)
		return cvar;
	auto *stateCl = GetClientState();
	return (stateCl != nullptr) ? stateCl->GetConVar(cv) : nullptr;
}

bool CEngine::RunConsoleCommand(std::string cmd,std::vector<std::string> &argv,KeyState pressState,float magnitude,const std::function<bool(ConConf*,float&)> &callback)
{
	std::transform(cmd.begin(),cmd.end(),cmd.begin(),::tolower);
	ClientState *stateCl = static_cast<ClientState*>(GetClientState());
	pragma::BasePlayerComponent *pl = NULL;
	if(stateCl != NULL)
	{
		CGame *game = stateCl->GetGameState();
		if(game != NULL)
			pl = game->GetLocalPlayer();
	}
	if(stateCl == nullptr)
		return RunEngineConsoleCommand(cmd,argv,pressState,magnitude,callback);
	if(stateCl == NULL || !stateCl->RunConsoleCommand(cmd,argv,pl,pressState,magnitude,callback))
	{
		Con::cwar<<"WARNING: Unknown console command '"<<cmd<<"'!"<<Con::endl;
		auto similar = (stateCl != nullptr) ? stateCl->FindSimilarConVars(cmd) : FindSimilarConVars(cmd);
		if(similar.empty() == true)
			Con::cout<<"No similar matches found!"<<Con::endl;
		else
		{
			Con::cout<<"Were you looking for one of the following?"<<Con::endl;
			for(auto &sim : similar)
				Con::cout<<"- "<<sim<<Con::endl;
		}
		return false;
	}
	return true;
}
