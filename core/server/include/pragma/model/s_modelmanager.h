// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_MODELMANAGER_H__
#define __S_MODELMANAGER_H__

#include "pragma/serverdefinitions.h"
#include <pragma/model/modelmanager.h>

namespace pragma::asset {
	class DLLSERVER SModelManager : public ModelManager {
	  public:
		using ModelManager::ModelManager;
	  private:
		//virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr) override;
	};
};

#endif
