/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_MODELMANAGER_H__
#define __S_MODELMANAGER_H__

#include "pragma/serverdefinitions.h"
#include <pragma/model/modelmanager.h>

namespace pragma::asset
{
	class DLLSERVER SModelManager
		: public ModelManager
	{
	public:
		using ModelManager::ModelManager;
	private:
		virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr) override;
	};
};

#endif
