// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :model_manager;

import :server_state;

/*std::shared_ptr<Model> pragma::asset::SModelManager::LoadModel(const std::string &mdlName,bool bReload,bool *outIsNewModel)
{
	auto mdl = ModelManager::LoadModel(mdlName,bReload,outIsNewModel);
	if(mdl == nullptr)
		static_cast<SGame&>(*m_nw.GetGameState()).RegisterGameResource("models/" +ToCacheIdentifier(mdlName));
	return mdl;
}*/
