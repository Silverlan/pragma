/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/modelmanager.h"
#include "pragma/asset/util_asset.hpp"
#include <sharedutils/util_path.hpp>
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>

extern DLLNETWORK Engine *engine;

static const std::vector<std::string> &get_model_extensions()
{
	static std::vector<std::string> extensions {};
	if(extensions.empty())
	{
		extensions = pragma::asset::get_supported_extensions(pragma::asset::Type::Model,true);
		auto &assetManager = engine->GetAssetManager();
		auto numImporters = assetManager.GetImporterCount(pragma::asset::Type::Model);
		for(auto i=decltype(numImporters){0u};i<numImporters;++i)
		{
			auto *importerInfo = assetManager.GetImporterInfo(pragma::asset::Type::Model,i);
			if(importerInfo == nullptr)
				continue;
			extensions.reserve(extensions.size() +importerInfo->fileExtensions.size());
			for(auto &ext : importerInfo->fileExtensions)
				extensions.push_back(ext);
		}
	}
	return extensions;
}
bool pragma::asset::ModelAsset::IsInUse() const {return model.use_count() > 1;}

pragma::asset::ModelManager::ModelManager(NetworkState &nw)
	: m_nw{nw}
{
	// TODO: New extensions might be added after the model manager has been created
	for(auto &ext : get_model_extensions())
		RegisterFileExtension(ext);
}

std::shared_ptr<Model> pragma::asset::ModelManager::CreateModel(uint32_t numBones,const std::string &mdlName)
{
	return Model::Create<Model>(&m_nw,numBones,mdlName);
}
std::shared_ptr<Model> pragma::asset::ModelManager::CreateModel(const std::string &name,bool bAddReference,bool addToCache)
{
	uint32_t boneCount = (bAddReference == true) ? 1 : 0;
	auto mdl = CreateModel(boneCount,name);
	auto &skeleton = mdl->GetSkeleton();
	auto reference = pragma::animation::Animation::Create();

	if(bAddReference == true)
	{
		auto frame = Frame::Create(1);
		auto *root = new panima::Bone;
		root->name = "root";
		auto rootID = skeleton.AddBone(root);
		mdl->SetBindPoseBoneMatrix(0,glm::inverse(umat::identity()));
		auto &rootBones = skeleton.GetRootBones();
		rootBones[0] = skeleton.GetBone(rootID).lock();
		reference->AddBoneId(0);

		frame->SetBonePosition(0,Vector3(0.f,0.f,0.f));
		frame->SetBoneOrientation(0,uquat::identity());

		auto refFrame = Frame::Create(*frame);
		frame->Localize(*reference,skeleton);
		reference->AddFrame(frame);
		mdl->AddAnimation("reference",reference);
		mdl->SetReference(refFrame);

		auto &baseMeshes = mdl->GetBaseMeshes();
		baseMeshes.push_back(0);
		mdl->AddMeshGroup("reference");

		mdl->CreateTextureGroup();
	}

	if(addToCache)
		AddToCache(name,std::make_shared<ModelAsset>(mdl));
	return mdl;
}
std::shared_ptr<Model> pragma::asset::ModelManager::LoadModel(FWMD &wmd,const std::string &mdlName) const
{
	auto *game = m_nw.GetGameState();
	assert(game);
	return std::shared_ptr<Model>{wmd.Load<Model,ModelMesh,ModelSubMesh>(game,mdlName,[this](const std::string &mat,bool reload) -> Material* {
		return m_nw.LoadMaterial(mat,reload);
	},[this](const std::string &mdlName) -> std::shared_ptr<Model> {
		return m_nw.GetGameState()->LoadModel(mdlName);
	})};
}
std::shared_ptr<Model> pragma::asset::ModelManager::LoadModel(const std::string &mdlName,bool bReload,bool *outIsNewModel)
{
	if(outIsNewModel)
		*outIsNewModel = false;
	auto cacheIdentifier = ToCacheIdentifier(mdlName);
	if(bReload == false)
	{
		FlagForRemoval(mdlName,false);

		auto *asset = FindCachedAsset(mdlName);
		if(asset)
			return static_cast<ModelAsset*>(asset)->model;
	}

	assert(m_nw.GetGameState());
	FWMD wmdLoader {m_nw.GetGameState()};
	auto mdl = LoadModel(wmdLoader,ToCacheIdentifier(mdlName));
	if(mdl == nullptr)
		return nullptr;
	mdl->Update();
	AddToCache(mdlName,std::make_shared<ModelAsset>(mdl));
	if(outIsNewModel != nullptr)
		*outIsNewModel = true;
	return mdl;
}
void pragma::asset::ModelManager::FlagForRemoval(const Model &mdl,bool flag)
{
	auto *asset = FindCachedAsset(mdl.GetName());
	if(asset && static_cast<ModelAsset*>(asset)->model.get() == &mdl)
	{
		FlagForRemoval(mdl.GetName());
		return;
	}
	auto it = std::find_if(m_cache.begin(),m_cache.end(),[&mdl](const std::pair<size_t,AssetInfo> &pair) {
		auto &assetInfo = pair.second;
		return static_cast<ModelAsset*>(assetInfo.asset.get())->model.get() == &mdl;
	});
	if(it == m_cache.end())
		return;
	FlagForRemoval(it->first);
}
