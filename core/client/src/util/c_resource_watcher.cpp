/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/util/c_resource_watcher.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/console/c_cvar.h"
#include <texture_load_flags.hpp>
#include <sharedutils/util_file.h>
#include <cmaterialmanager.h>
#include <textureinfo.h>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/asset/util_asset.hpp>
#include <prosper_glsl.hpp>
#include <cmaterial_manager2.hpp>
#include <cmaterial.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

decltype(ECResourceWatcherCallbackType::Shader) ECResourceWatcherCallbackType::Shader = ECResourceWatcherCallbackType {umath::to_integral(E::Shader)};
decltype(ECResourceWatcherCallbackType::ParticleSystem) ECResourceWatcherCallbackType::ParticleSystem = ECResourceWatcherCallbackType {umath::to_integral(E::ParticleSystem)};
decltype(ECResourceWatcherCallbackType::Count) ECResourceWatcherCallbackType::Count = ECResourceWatcherCallbackType {umath::to_integral(E::Count)};
static auto cvMatStreaming = GetClientConVar("cl_material_streaming_enabled");
void CResourceWatcherManager::ReloadTexture(const std::string &path)
{
	auto *nw = m_networkState;
	auto &matManager = static_cast<msys::CMaterialManager &>(nw->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto *asset = texManager.FindCachedAsset(path);
	if(asset == nullptr)
		return;
	texManager.RemoveFromCache(path);
	auto loadInfo = std::make_unique<msys::TextureLoadInfo>();
	loadInfo->onLoaded = [path, nw](util::Asset &asset) {
		if(nw == nullptr)
			return;
		auto &matManager = static_cast<msys::CMaterialManager &>(nw->GetMaterialManager());
		auto &texManager = matManager.GetTextureManager();

		auto pathNoExt = path;
		std::string ext;
		auto bExt = ufile::get_extension(path, &ext);
		if(bExt)
			ext = '.' + ext;
		ufile::remove_extension_from_filename(pathNoExt);
		std::function<void(ds::Block &, CMaterial &)> fLookForTextureAndUpdate = nullptr;
		fLookForTextureAndUpdate = [&fLookForTextureAndUpdate, &pathNoExt, &ext](ds::Block &dataBlock, CMaterial &mat) {
			auto *data = dataBlock.GetData();
			if(data == nullptr)
				return;
			for(auto &pair : *data) {
				auto &dataVal = pair.second;
				if(dataVal == nullptr)
					continue;
				if(dataVal->IsBlock()) {
					fLookForTextureAndUpdate(static_cast<ds::Block &>(*dataVal), mat);
					continue;
				}
				auto *dsTex = dynamic_cast<ds::Texture *>(dataVal.get());
				if(dsTex != nullptr) {
					auto &texInfo = dsTex->GetValue();
					if(texInfo.texture == nullptr)
						continue;
					auto texName = texInfo.name;
					ufile::remove_extension_from_filename(texName);
					if(FileManager::ComparePath(texName, pathNoExt) == false)
						continue;
					auto identifier = pair.first;
					auto &texture = texInfo.texture;
					mat.CallOnLoaded([&mat, identifier, texture]() {
						auto &tex = *static_cast<Texture *>(texture.get());
						mat.SetTexture(identifier, &tex);
					});
				}
			}
		};

		// Iterate all materials and update the ones that use this texture
		for(auto &pair : matManager.GetCache()) {
			auto asset = matManager.GetAsset(pair.second);
			if(!asset)
				continue;
			auto hMat = msys::CMaterialManager::GetAssetObject(*asset);
			if(!hMat)
				continue;
			auto &data = hMat.get()->GetDataBlock();
			if(data != nullptr)
				fLookForTextureAndUpdate(*data, static_cast<CMaterial &>(*hMat.get()));
		}
	};
	texManager.LoadAsset(path, std::move(loadInfo));
}

void CResourceWatcherManager::OnMaterialReloaded(const std::string &path, const std::unordered_set<Model *> &modelMap)
{
	ResourceWatcherManager::OnMaterialReloaded(path, modelMap);
	if(c_game == nullptr)
		return;
	EntityIterator entIt {*c_game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CModelComponent>>();
	for(auto *ent : entIt) {
		auto &mdl = ent->GetModel();
		if(mdl == nullptr)
			continue;
		auto it = modelMap.find(mdl.get());
		if(it == modelMap.end())
			continue;
		auto mdlC = static_cast<pragma::CModelComponent *>(ent->GetModelComponent());
		mdlC->SetRenderMeshesDirty();
		mdlC->UpdateRenderMeshes();
	}
}

void CResourceWatcherManager::GetWatchPaths(std::vector<std::string> &paths)
{
	ResourceWatcherManager::GetWatchPaths(paths);
	paths.reserve(paths.size() + 2);
	paths.push_back("shaders");
	paths.push_back("particles");
}

void CResourceWatcherManager::OnResourceChanged(const util::Path &rootPath, const util::Path &path, const std::string &ext)
{
	ResourceWatcherManager::OnResourceChanged(rootPath, path, ext);
	auto &strPath = path.GetString();
	auto assetType = pragma::asset::determine_type_from_extension(ext);
	if(assetType.has_value()) {
		if(*assetType == pragma::asset::Type::ParticleSystem) {
			if(pragma::CParticleSystemComponent::IsParticleFilePrecached(strPath) == false)
				return;
#if RESOURCE_WATCHER_VERBOSE > 0
			auto ptPath = "particles\\" + strPath;
			Con::cout << "[ResourceWatcher] Particle has changed: " << ptPath << ". Attempting to reload..." << Con::endl;
#endif
			pragma::CParticleSystemComponent::Precache(strPath, true);
			CallChangeCallbacks(ECResourceWatcherCallbackType::ParticleSystem, strPath, ext);
		}
	}
	else if(prosper::glsl::is_glsl_file_extension(ext) || ext == "hlsl") {
#if RESOURCE_WATCHER_VERBOSE > 0
		auto shaderPath = "shaders\\" + strPath;
		Con::cout << "[ResourceWatcher] Shader has changed: " << shaderPath << ". Attempting to reload..." << Con::endl;
#endif
		auto canonShader = FileManager::GetCanonicalizedPath(strPath);
		ufile::remove_extension_from_filename(canonShader);
		ustring::to_lower(canonShader);
		auto &shaderManager = c_engine->GetShaderManager();
		std::vector<std::string> reloadShaders;
		for(auto &pair : shaderManager.GetShaderNameToIndexTable()) {
			auto *shader = shaderManager.GetShader(pair.second);
			if(shader == nullptr)
				continue;
			for(auto &src : shader->GetSourceFilePaths()) {
				auto fname = FileManager::GetCanonicalizedPath(src);
				ufile::remove_extension_from_filename(fname);
				ustring::to_lower(fname);
				if(canonShader == fname)
					reloadShaders.push_back(pair.first);
			}
		}
		for(auto &name : reloadShaders) {
#if RESOURCE_WATCHER_VERBOSE > 0
			Con::cout << "[ResourceWatcher] Reloading shader '" << name << "'..." << Con::endl;
#endif
			c_engine->ReloadShader(name);
		}
		CallChangeCallbacks(ECResourceWatcherCallbackType::Shader, strPath, ext);
	}
}
