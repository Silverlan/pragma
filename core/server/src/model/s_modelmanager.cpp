#include "stdafx_server.h"
#include "pragma/model/s_modelmanager.h"
#include <sharedutils/util_string.h>
#include "pragma/file_formats/wmd.h"
#include "pragma/file_formats/wmd_load.h"
#include <pragma/serverstate/serverstate.h>

extern DLLSERVER SGame *s_game;

std::shared_ptr<Model> pragma::asset::SModelManager::LoadModel(const std::string &mdlName,bool bReload,bool *outIsNewModel)
{
	auto mdl = ModelManager::LoadModel(mdlName,bReload,outIsNewModel);
	if(mdl == nullptr)
		static_cast<SGame&>(*m_nw.GetGameState()).RegisterGameResource("models/" +GetNormalizedModelName(mdlName));
	return mdl;
}
