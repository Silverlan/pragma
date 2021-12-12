/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/file_formats/wmd.h"
#include "pragma/file_formats/wmd_load.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/debug/intel_vtune.hpp>

std::shared_ptr<Model> pragma::asset::CModelManager::Load(
	const std::string &mdlName,std::unique_ptr<ufile::IFile> &&f,const std::string &ext,const std::function<std::shared_ptr<Model>(const std::string&)> &loadModel
)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("cl_load_model_core");
#endif
	auto &game = *m_nw.GetGameState();
	FWMD wmd {&game};
	auto mdl = wmd.Load<CModel,CModelMesh,CModelSubMesh>(&game,mdlName,std::move(f),ext,
		[&game](const std::string &mdlName) -> std::shared_ptr<Model> {
			return game.LoadModel(mdlName);
	});
	if(mdl)
		mdl->Update(ModelUpdateFlags::AllData & ~ModelUpdateFlags::UpdateCollisionShapes);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return mdl;
}
/*std::shared_ptr<Model> pragma::asset::CModelManager::LoadModel(const std::string &mdlName,bool bReload,bool *outIsNewModel)
{
	auto mdl = ModelManager::LoadModel(mdlName,bReload,outIsNewModel);
	if(mdl == nullptr)
		static_cast<CGame&>(*m_nw.GetGameState()).RequestResource("models/" +ToCacheIdentifier(mdlName) +"." +std::string{pragma::asset::FORMAT_MODEL_BINARY});
	return mdl;
}

std::shared_ptr<Model> pragma::asset::CModelManager::LoadModel(FWMD &wmd,const std::string &mdlName) const
{
	return std::shared_ptr<Model>{wmd.Load<CModel,CModelMesh,CModelSubMesh>(m_nw.GetGameState(),mdlName,[this](const std::string &mdlName) -> std::shared_ptr<Model> {
		return m_nw.GetGameState()->LoadModel(mdlName);
	})};
}
std::shared_ptr<Model> pragma::asset::CModelManager::CreateModel(uint32_t numBones,const std::string &mdlName)
{
	return Model::Create<CModel>(&m_nw,numBones,mdlName);
}*/
