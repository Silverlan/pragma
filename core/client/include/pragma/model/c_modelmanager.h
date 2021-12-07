/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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

		virtual std::shared_ptr<Model> Load(const std::string &mdlName,std::unique_ptr<ufile::IFile> &&f,const std::string &ext,const std::function<std::shared_ptr<Model>(const std::string&)> &loadModel) override;
		//virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr) override;
	private:
		//virtual std::shared_ptr<Model> LoadModel(FWMD &wmd,const std::string &mdlName) const override;
		//virtual std::shared_ptr<Model> CreateModel(uint32_t numBones,const std::string &mdlName) override;
	};
};

#endif
