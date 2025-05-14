/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/model/s_modelmanager.h"
#include <sharedutils/util_string.h>
#include <pragma/serverstate/serverstate.h>

extern DLLSERVER SGame *s_game;

/*std::shared_ptr<Model> pragma::asset::SModelManager::LoadModel(const std::string &mdlName,bool bReload,bool *outIsNewModel)
{
	auto mdl = ModelManager::LoadModel(mdlName,bReload,outIsNewModel);
	if(mdl == nullptr)
		static_cast<SGame&>(*m_nw.GetGameState()).RegisterGameResource("models/" +ToCacheIdentifier(mdlName));
	return mdl;
}*/
