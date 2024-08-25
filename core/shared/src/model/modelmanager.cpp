/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/modelmanager.h"
#include "pragma/util/util_game.hpp"
#include "pragma/file_formats/wmd_load.h"
#include "pragma/asset/util_asset.hpp"
#include <sharedutils/util_path.hpp>
#include "pragma/model/animation/skeleton.hpp"
#include "pragma/model/animation/bone.hpp"
#include <fsys/ifile.hpp>
#include <pragma/debug/intel_vtune.hpp>

extern DLLNETWORK Engine *engine;

pragma::asset::IModelFormatHandler::IModelFormatHandler(util::IAssetManager &assetManager) : util::IAssetFormatHandler {assetManager} {}
bool pragma::asset::IModelFormatHandler::LoadData(ModelProcessor &processor, ModelLoadInfo &info)
{
	if(model) {
		model->SetName(processor.identifier);
		model->PrecacheMaterials();
		auto &mdlManager = static_cast<ModelManager &>(GetAssetManager());
		for(auto &inc : model->GetMetaInfo().includes)
			mdlManager.PreloadAsset(inc);
	}
	return model != nullptr;
}

///////////

pragma::asset::PmdlFormatHandler::PmdlFormatHandler(util::IAssetManager &assetManager) : IModelFormatHandler {assetManager} {}
bool pragma::asset::PmdlFormatHandler::LoadData(ModelProcessor &processor, ModelLoadInfo &info)
{
	auto &mdlManager = static_cast<ModelManager &>(GetAssetManager());
	auto &nw = mdlManager.GetNetworkState();
	auto &game = *nw.GetGameState();

	auto udm = util::load_udm_asset(std::move(m_file));
	if(udm == nullptr)
		return false;
	std::string err;
	auto mdl = mdlManager.CreateModel(0u, std::string {""});
	if(!mdl) {
		m_error = err;
		return false;
	}
	if(!Model::Load(*mdl, nw, udm->GetAssetData(), err)) {
		m_error = err;
		return false;
	}
	model = mdl;
	return IModelFormatHandler::LoadData(processor, info);
}

///////////

std::unique_ptr<util::IAssetProcessor> pragma::asset::ModelLoader::CreateAssetProcessor(const std::string &identifier, const std::string &ext, std::unique_ptr<util::IAssetFormatHandler> &&formatHandler)
{
	auto processor = util::TAssetFormatLoader<ModelProcessor>::CreateAssetProcessor(identifier, ext, std::move(formatHandler));
	auto &mdlProcessor = static_cast<ModelProcessor &>(*processor);
	mdlProcessor.identifier = identifier;
	mdlProcessor.formatExtension = ext;
	return processor;
}

///////////

pragma::asset::ModelLoadInfo::ModelLoadInfo(util::AssetLoadFlags flags) : util::AssetLoadInfo {flags} {}
pragma::asset::ModelProcessor::ModelProcessor(util::AssetFormatLoader &loader, std::unique_ptr<util::IAssetFormatHandler> &&handler) : util::FileAssetProcessor {loader, std::move(handler)} {}
bool pragma::asset::ModelProcessor::Load()
{
	auto &mdlHandler = static_cast<IModelFormatHandler &>(*handler);
	auto r = mdlHandler.LoadData(*this, static_cast<ModelLoadInfo &>(*loadInfo));
	if(!r)
		return false;
	model = mdlHandler.model;
	model->Update(ModelUpdateFlags::Initialize);
	return true;
}
bool pragma::asset::ModelProcessor::Finalize()
{
	// TODO: Move buffer allocation to Load() to make better use of multi-threading.
	// Data copying has to be performed on main thread due to the use of a primary
	// command buffer.
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("load_model_update_buffers");
#endif

	auto &assetManager = static_cast<ModelManager &>(handler->GetAssetManager());
	auto &includes = model->GetMetaInfo().includes;
	if(!includes.empty()) {
		for(auto &inc : includes) {
			auto asset = assetManager.LoadAsset(inc);
			if(!asset) {
				Con::cwar << "Model '" << model->GetName() << "' has include reference to model '" << inc << "', but that model could not be loaded! Ignoring..." << Con::endl;
				continue;
			}
			model->Merge(*asset);
		}
		model->Update(); // Need to update again
	}

	model->Update(ModelUpdateFlags::UpdateBuffers | ModelUpdateFlags::UpdateChildren);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return true;
}

static const std::vector<std::string> &get_model_extensions()
{
	static std::vector<std::string> extensions {};
	if(extensions.empty()) {
		extensions = pragma::asset::get_supported_extensions(pragma::asset::Type::Model, pragma::asset::FormatType::All);
		auto &assetManager = engine->GetAssetManager();
		auto numImporters = assetManager.GetImporterCount(pragma::asset::Type::Model);
		for(auto i = decltype(numImporters) {0u}; i < numImporters; ++i) {
			auto *importerInfo = assetManager.GetImporterInfo(pragma::asset::Type::Model, i);
			if(importerInfo == nullptr)
				continue;
			extensions.reserve(extensions.size() + importerInfo->fileExtensions.size());
			for(auto &extInfo : importerInfo->fileExtensions)
				extensions.push_back(extInfo.first);
		}
	}
	return extensions;
}

/*std::optional<util::AssetLoadJobId> pragma::asset::ModelLoader::AddJob(
	NetworkState &nw,const std::string &identifier,const std::string &ext,const std::unique_ptr<ufile::IFile> &file,util::AssetLoadJobPriority priority
)
{
	//ModelProcessor(util::AssetFormatLoader &loader,std::unique_ptr<util::IAssetFormatHandler> &&handler);
	auto processor = std::make_unique<ModelProcessor>(nw,*this,std::move(file));
	return IAssetLoader::AddJob(identifier,std::move(processor),priority);
}*/

pragma::asset::ModelManager::ModelManager(NetworkState &nw) : m_nw {nw}
{
	auto fileHandler = std::make_unique<util::AssetFileHandler>();
	fileHandler->open = [](const std::string &path, util::AssetFormatType formatType) -> std::unique_ptr<ufile::IFile> {
		auto openMode = filemanager::FileMode::Read;
		if(formatType == util::AssetFormatType::Binary)
			openMode |= filemanager::FileMode::Binary;
		auto f = filemanager::open_file(path, openMode);
		if(!f)
			return nullptr;
		return std::make_unique<fsys::File>(f);
	};
	fileHandler->exists = [](const std::string &path) -> bool { return filemanager::exists(path); };
	SetFileHandler(std::move(fileHandler));
	SetRootDirectory("models");
	m_loader = std::make_unique<ModelLoader>(*this);

	RegisterFormatHandler<PmdlFormatHandler>("pmdl_b");
	RegisterFormatHandler<PmdlFormatHandler>("pmdl");
	RegisterFormatHandler<WmdFormatHandler>("wmd"); // Legacy format

	// Import formats
	RegisterImportHandler<SourceMdlFormatHandler>("mdl");
	RegisterImportHandler<Source2VmdlFormatHandler>("vmdl_c");
	RegisterImportHandler<NifFormatHandler>("nif");
	RegisterImportHandler<PmxFormatHandler>("pmx");

	auto addBlenderFormatHandler = [this](std::string ext) { return RegisterImportHandler(ext, [ext](util::IAssetManager &assetManager) -> std::unique_ptr<util::IImportAssetFormatHandler> { return std::make_unique<BlenderFormatHandler>(assetManager, ext); }); };
	addBlenderFormatHandler("blend");
	addBlenderFormatHandler("fbx");
	addBlenderFormatHandler("dae");
	addBlenderFormatHandler("x3d");
	addBlenderFormatHandler("obj");
	addBlenderFormatHandler("abc");
	addBlenderFormatHandler("usd");

	auto &assetManager = pragma::get_engine()->GetAssetManager();
	auto numImporters = assetManager.GetImporterCount(pragma::asset::Type::Model);
	for(auto i = decltype(numImporters) {0u}; i < numImporters; ++i) {
		auto *importerInfo = assetManager.GetImporterInfo(pragma::asset::Type::Model, i);
		if(!importerInfo)
			continue;
		for(auto &extInfo : importerInfo->fileExtensions) {
			if(extInfo.first == "pmx")
				continue;
			RegisterImportHandler<AssetManagerFormatHandler>(extInfo.first, extInfo.second ? util::AssetFormatType::Binary : util::AssetFormatType::Text);
		}
	}
}
std::shared_ptr<Model> pragma::asset::ModelManager::CreateModel(uint32_t numBones, const std::string &mdlName) { return Model::Create<Model>(&m_nw, numBones, mdlName); }
std::shared_ptr<ModelMesh> pragma::asset::ModelManager::CreateMesh() { return m_nw.GetGameState()->CreateModelMesh(); }
std::shared_ptr<ModelSubMesh> pragma::asset::ModelManager::CreateSubMesh() { return m_nw.GetGameState()->CreateModelSubMesh(); }
std::shared_ptr<Model> pragma::asset::ModelManager::CreateModel(const std::string &name, bool bAddReference, bool addToCache)
{
	uint32_t boneCount = (bAddReference == true) ? 1 : 0;
	auto mdl = CreateModel(boneCount, name);
	auto &skeleton = mdl->GetSkeleton();
	auto reference = pragma::animation::Animation::Create();

	if(bAddReference == true) {
		auto frame = Frame::Create(1);
		auto *root = new pragma::animation::Bone;
		root->name = "root";
		auto rootID = skeleton.AddBone(root);
		mdl->SetBindPoseBoneMatrix(0, glm::inverse(umat::identity()));
		auto &rootBones = skeleton.GetRootBones();
		rootBones[0] = skeleton.GetBone(rootID).lock();
		reference->AddBoneId(0);

		frame->SetBonePosition(0, Vector3(0.f, 0.f, 0.f));
		frame->SetBoneOrientation(0, uquat::identity());

		auto refFrame = Frame::Create(*frame);
		frame->Localize(*reference, skeleton);
		reference->AddFrame(frame);
		mdl->AddAnimation("reference", reference);
		mdl->SetReference(refFrame);

		auto &baseMeshes = mdl->GetBaseMeshes();
		baseMeshes.push_back(0);
		mdl->AddMeshGroup("reference");

		mdl->CreateTextureGroup();
	}

	if(addToCache) {
		auto asset = std::make_shared<util::Asset>();
		asset->assetObject = mdl;
		AddToCache(name, asset);
	}
	return mdl;
}
void pragma::asset::ModelManager::InitializeProcessor(util::IAssetProcessor &processor) {}
util::AssetObject pragma::asset::ModelManager::InitializeAsset(const util::Asset &asset, const util::AssetLoadJob &job)
{
	auto &mdlProcessor = *static_cast<ModelProcessor *>(job.processor.get());
	return mdlProcessor.model;
}
#if 0
bool pragma::asset::ModelManager::PrecacheModel(const std::string &mdlName) const
{
	auto f = filemanager::open_file(mdlName,filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		return false;
	std::string ext;
	if(!ufile::get_extension(mdlName,&ext))
		return false;
	auto fp = std::make_unique<fsys::File>(f);
	auto jobId = m_loader->AddJob(m_nw,mdlName,ext,std::move(fp));
	return jobId.has_value();
}
std::shared_ptr<Model> pragma::asset::ModelManager::LoadModel(FWMD &wmd,const std::string &mdlName) const
{


}
std::shared_ptr<Model> pragma::asset::ModelManager::LoadModel(const std::string &cacheName,const std::shared_ptr<ufile::IFile> &file,const std::string &ext)
{
	auto *asset = FindCachedAsset(cacheName);
	if(asset)
		return GetAssetObject(*asset);
	auto fp = std::make_unique<fsys::File>(file);
	auto jobId = m_loader->AddJob(m_nw,cacheName,ext,std::move(fp));

	return jobId.has_value();

	mdl->Update();
	AddToCache(mdlName,std::make_shared<ModelAsset>(mdl));
	if(outIsNewModel != nullptr)
		*outIsNewModel = true;
	return mdl;
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
	//FWMD wmdLoader {m_nw.GetGameState()};
	//auto mdl = LoadModel(wmdLoader,ToCacheIdentifier(mdlName));

	std::string pathCache(pmodel);
	// std::transform(pathCache.begin(),pathCache.end(),pathCache.begin(),::tolower);

	auto model = pmodel;
	
	std::string ext;
	auto mdlPath = pragma::asset::find_file(pathCache,pragma::asset::Type::Model,&ext);
	if(mdlPath.has_value())
		model = *mdlPath;

	std::string path = "models\\";
	path += model;

	ustring::to_lower(ext);







	if(mdl == nullptr)
		return nullptr;
	mdl->Update();
	AddToCache(mdlName,std::make_shared<ModelAsset>(mdl));
	if(outIsNewModel != nullptr)
		*outIsNewModel = true;
	return mdl;
}
#endif
void pragma::asset::ModelManager::FlagForRemoval(const Model &mdl, bool flag)
{
	auto *asset = FindCachedAsset(mdl.GetName());
	if(asset) {
		auto mdlCache = GetAssetObject(*asset);
		if(mdlCache.get() == &mdl) {
			FlagForRemoval(mdl.GetName());
			return;
		}
	}
	auto it = std::find_if(m_cache.begin(), m_cache.end(), [this, &mdl](const std::pair<util::AssetIdentifierHash, util::AssetIndex> &pair) {
		auto mdlCache = GetAssetObject(*GetAsset(pair.second));
		return mdlCache.get() == &mdl;
	});
	if(it == m_cache.end())
		return;
	FlagForRemoval(it->first);
}
