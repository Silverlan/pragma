#ifndef __BASE_MODEL_COMPONENT_SETMODEL_HPP__
#define __BASE_MODEL_COMPONENT_SETMODEL_HPP__

#include "pragma/entities/components/base_model_component.hpp"

template<class TModel,class TModelMesh,class TModelLoader>
	void pragma::BaseModelComponent::SetModel(const std::string &mdl)
{
	if(mdl.empty() == true)
	{
		SetModel(std::shared_ptr<Model>(nullptr));
		return;
	}
	m_modelName = std::make_unique<std::string>(TModelLoader::GetCanonicalizedName(mdl));
	auto *nw = GetEntity().GetNetworkState();
	auto *game = nw->GetGameState();
	auto prevMdl = GetModel();
	auto model = game->LoadModel(mdl);
	if(model == nullptr)
	{
		model = game->LoadModel("error.wmd");
		if(model == nullptr)
		{
			if(GetModel() == prevMdl) // Model might have been changed during TModelLoader::Load-call in single player (on the client)
				SetModel(std::shared_ptr<Model>(nullptr));
			return;
		}
	}

	// Deprecated?
	/*auto model = TModelLoader::Load(nw->GetGameState(),mdl);
	if(model == nullptr)
	{
		model = TModelLoader::Load(nw->GetGameState(),"error.wmd");
		if(model == nullptr)
		{
			if(GetModel() == prevMdl) // Model might have been changed during TModelLoader::Load-call in single player (on the client)
				SetModel(std::shared_ptr<Model>(nullptr));
			return;
		}
	}*/

	SetModel(model);
	ustring::to_lower(*m_modelName);
}

#endif
