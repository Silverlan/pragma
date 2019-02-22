#ifndef __S_MODELMANAGER_H__
#define __S_MODELMANAGER_H__

#include "pragma/serverdefinitions.h"
#include <pragma/model/modelmanager.h>
#include <pragma/math/surfacematerial.h>

class Game;
class DLLSERVER ModelManager
	: public TModelManager<Model,ModelMesh,ModelSubMesh>
{
public:
	static std::shared_ptr<Model> Load(Game *game,const std::string &path,bool bReload=false,bool *newModel=nullptr);
	static std::shared_ptr<Model> GetModel(Game *game,const std::string &mdlName);
	static std::shared_ptr<Model> Create(Game *game,const std::string &path);
	static std::shared_ptr<Model> Create(Game *game,bool bAddReference=true);
	static std::shared_ptr<Model> CreateFromBrushMeshes(Game *game,std::vector<std::shared_ptr<BrushMesh>> &meshes,const std::vector<SurfaceMaterial> &materials);
};

#endif
