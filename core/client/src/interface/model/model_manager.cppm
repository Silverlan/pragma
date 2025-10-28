// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:model.model_manager;

export import pragma.shared;

export namespace pragma::asset {
	class DLLCLIENT CModelManager : public ModelManager {
	  public:
		using ModelManager::ModelManager;

		virtual std::shared_ptr<pragma::Model> CreateModel(uint32_t numBones, const std::string &mdlName) override;
	  private:
	};
};
