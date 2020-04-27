/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/file_formats/wmd.h"
#include "pragma/file_formats/wmd_load.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"

std::shared_ptr<Model> pragma::asset::CModelManager::LoadModel(const std::string &mdlName,bool bReload,bool *outIsNewModel)
{
	auto mdl = ModelManager::LoadModel(mdlName,bReload,outIsNewModel);
	if(mdl == nullptr)
		static_cast<CGame&>(*m_nw.GetGameState()).RequestResource("models/" +GetNormalizedModelName(mdlName));
	return mdl;
}

std::shared_ptr<Model> pragma::asset::CModelManager::LoadModel(FWMD &wmd,const std::string &mdlName) const
{
	return std::shared_ptr<Model>{wmd.Load<CModel,CModelMesh,CModelSubMesh>(m_nw.GetGameState(),mdlName,[this](const std::string &mat,bool reload) -> Material* {
		return m_nw.LoadMaterial(mat,reload);
	},[this](const std::string &mdlName) -> std::shared_ptr<Model> {
		return m_nw.GetGameState()->LoadModel(mdlName);
	})};
}
std::shared_ptr<Model> pragma::asset::CModelManager::CreateModel(uint32_t numBones,const std::string &mdlName)
{
	return Model::Create<CModel>(&m_nw,numBones,mdlName);
}
