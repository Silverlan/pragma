// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_MODELMANAGER_H__
#define __C_MODELMANAGER_H__

#include "pragma/clientdefinitions.h"
#include <pragma/model/modelmanager.h>

class CModel;
class CModelMesh;
class CModelSubMesh;
namespace pragma::asset {
	class DLLCLIENT CModelManager : public ModelManager {
	  public:
		using ModelManager::ModelManager;

		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones, const std::string &mdlName) override;
	  private:
	};
};

#endif
