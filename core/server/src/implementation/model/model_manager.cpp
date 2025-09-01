// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <sharedutils/util_string.h>
#include <pragma/serverstate/serverstate.h>

module pragma.server.model_manager;

/*std::shared_ptr<Model> pragma::asset::SModelManager::LoadModel(const std::string &mdlName,bool bReload,bool *outIsNewModel)
{
	auto mdl = ModelManager::LoadModel(mdlName,bReload,outIsNewModel);
	if(mdl == nullptr)
		static_cast<SGame&>(*m_nw.GetGameState()).RegisterGameResource("models/" +ToCacheIdentifier(mdlName));
	return mdl;
}*/
