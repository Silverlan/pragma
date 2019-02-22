#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/file_formats/wmd.h"
#include "pragma/file_formats/wmd_load.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"

extern DLLCLIENT CGame *c_game;

std::shared_ptr<Model> CModelManager::GetModel(Game *game,const std::string &mdlName)
{
	return TModelManager<CModel,CModelMesh,CModelSubMesh>::GetModel(game,mdlName);
}

std::shared_ptr<Model> CModelManager::Load(Game *game,const std::string &path,bool bReload,bool *newModel)
{
	auto r = TModelManager<CModel,CModelMesh,CModelSubMesh>::Load(game,path,bReload,newModel);
	if(c_game != nullptr && r == nullptr)
		c_game->RequestResource("models\\" +GetCanonicalizedName(path));
	return r;
}

std::shared_ptr<Model> CModelManager::Create(Game *game,const std::string &path)
{
	auto r = TModelManager<CModel,CModelMesh,CModelSubMesh>::Create(game,path);
	if(c_game != nullptr && r == nullptr)
		c_game->RequestResource("models\\" +GetCanonicalizedName(path));
	return r;
}

std::shared_ptr<Model> CModelManager::Create(Game *game,bool bAddReference)
{
	return TModelManager<CModel,CModelMesh,CModelSubMesh>::CreateModel(game,bAddReference);
}

std::shared_ptr<Model> CModelManager::CreateFromBrushMeshes(std::vector<std::shared_ptr<BrushMesh>> &meshes)
{
	return TModelManager<CModel,CModelMesh,CModelSubMesh>::CreateFromBrushMeshes(c_game,meshes,c_game->GetSurfaceMaterials());
}