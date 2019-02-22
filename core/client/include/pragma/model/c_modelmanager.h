#ifndef __C_MODELMANAGER_H__
#define __C_MODELMANAGER_H__

#include "pragma/clientdefinitions.h"
#include "pragma/model/modelmanager.h"

class CModel;
class CModelMesh;
class CModelSubMesh;
class DLLCLIENT CModelManager
	: public TModelManager<CModel,CModelMesh,CModelSubMesh>
{
public:
	static std::shared_ptr<Model> Load(Game *game,const std::string &path,bool bReload=false,bool *newModel=nullptr);
	static std::shared_ptr<Model> GetModel(Game *game,const std::string &mdlName);
	static std::shared_ptr<Model> Create(Game *game,const std::string &path);
	static std::shared_ptr<Model> Create(Game *game,bool bAddReference=true);
	static std::shared_ptr<Model> CreateFromBrushMeshes(std::vector<std::shared_ptr<BrushMesh>> &meshes);
};

#endif