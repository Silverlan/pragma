#include "stdafx_server.h"
#include "pragma/model/s_modelmanager.h"
#include <sharedutils/util_string.h>
#include "pragma/file_formats/wmd.h"
#include "pragma/file_formats/wmd_load.h"
#include <pragma/serverstate/serverstate.h>

extern DLLSERVER SGame *s_game;

std::shared_ptr<Model> ModelManager::GetModel(Game *game,const std::string &mdlName)
{
	return TModelManager<Model,ModelMesh,ModelSubMesh>::GetModel(game,mdlName);
}

std::shared_ptr<Model> ModelManager::Load(Game *game,const std::string &path,bool bReload,bool *newModel)
{
	auto r = TModelManager<Model,ModelMesh,ModelSubMesh>::Load(game,path,bReload,newModel);
	if(s_game != nullptr)
		s_game->RegisterGameResource("models\\" +GetCanonicalizedName(path));
	return r;
}

std::shared_ptr<Model> ModelManager::Create(Game *game,const std::string &path)
{
	auto r = TModelManager<Model,ModelMesh,ModelSubMesh>::Create(game,path);
	if(s_game != nullptr)
		s_game->RegisterGameResource("models\\" +GetCanonicalizedName(path));
	return r;
}

std::shared_ptr<Model> ModelManager::Create(Game *game,bool bAddReference)
{
	return TModelManager<Model,ModelMesh,ModelSubMesh>::CreateModel(game,bAddReference);
}

std::shared_ptr<Model> ModelManager::CreateFromBrushMeshes(Game *game,std::vector<std::shared_ptr<BrushMesh>> &meshes,const std::vector<SurfaceMaterial> &surfaceMaterials)
{
	return TModelManager<Model,ModelMesh,ModelSubMesh>::CreateFromBrushMeshes(game,meshes,surfaceMaterials);
}
