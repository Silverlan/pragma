// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:model_manager;

export import pragma.shared;

export namespace pragma::asset {
	class DLLSERVER SModelManager : public ModelManager {
	  public:
		using ModelManager::ModelManager;
	  private:
		//virtual std::shared_ptr<Model> LoadModel(const std::string &mdlName,bool bReload=false,bool *outIsNewModel=nullptr) override;
	};
};
