// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.model_manager;

export import pragma.shared;

export namespace pragma::asset {
	class DLLCLIENT CModelManager : public ModelManager {
	  public:
		using ModelManager::ModelManager;

		virtual std::shared_ptr<Model> CreateModel(uint32_t numBones, const std::string &mdlName) override;
	  private:
	};
};
