#ifndef __C_MODELMANAGER_H__
#define __C_MODELMANAGER_H__

#include "pragma/clientdefinitions.h"
#include <pragma/model/modelmanager.h>

class CModel;
class CModelMesh;
class CModelSubMesh;
namespace pragma::asset
{
	class DLLCLIENT CModelManager
		: public ModelManager
	{
	public:
		using ModelManager::ModelManager;
		virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr) override;
	private:
		virtual std::shared_ptr<Model> LoadModel(FWMD &wmd,const std::string &mdlName) const override;
		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones,const std::string &mdlName) override;
	};
};

#endif
