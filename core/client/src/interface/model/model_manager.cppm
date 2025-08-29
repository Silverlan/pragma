// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/model/modelmanager.h>

export module pragma.client.model:model_manager;

export namespace pragma::asset {
	class DLLCLIENT CModelManager : public ModelManager {
	  public:
		using ModelManager::ModelManager;

		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones, const std::string &mdlName) override;
	  private:
	};
};
