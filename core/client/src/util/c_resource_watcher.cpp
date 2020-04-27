/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/util/c_resource_watcher.hpp"
#include "pragma/console/c_cvar.h"
#include <texture_load_flags.hpp>
#include <sharedutils/util_file.h>
#include <cmaterialmanager.h>
#include <textureinfo.h>

extern DLLCENGINE CEngine *c_engine;

decltype(ECResourceWatcherCallbackType::Shader) ECResourceWatcherCallbackType::Shader = ECResourceWatcherCallbackType{umath::to_integral(E::Shader)};
decltype(ECResourceWatcherCallbackType::ParticleSystem) ECResourceWatcherCallbackType::ParticleSystem = ECResourceWatcherCallbackType{umath::to_integral(E::ParticleSystem)};
decltype(ECResourceWatcherCallbackType::Count) ECResourceWatcherCallbackType::Count = ECResourceWatcherCallbackType{umath::to_integral(E::Count)};
static auto cvMatStreaming = GetClientConVar("cl_material_streaming_enabled");
void CResourceWatcherManager::ReloadTexture(const std::string &path)
{
	auto *nw = m_networkState;
	auto &matManager = static_cast<CMaterialManager&>(nw->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto tex = texManager.GetTexture(path);
	if(tex == nullptr)
		return;
	TextureManager::LoadInfo loadInfo {};
	if(cvMatStreaming->GetBool() == false)
		loadInfo.flags |= TextureLoadFlags::LoadInstantly;
	loadInfo.flags |= TextureLoadFlags::Reload;
	loadInfo.onLoadCallback = [path,nw](std::shared_ptr<Texture> tex) {
		if(nw == nullptr)
			return;
		auto &matManager = static_cast<CMaterialManager&>(nw->GetMaterialManager());
		auto &texManager = matManager.GetTextureManager();

		auto pathNoExt = path;
		std::string ext;
		auto bExt = ufile::get_extension(path,&ext);
		if(bExt)
			ext = '.' +ext;
		ufile::remove_extension_from_filename(pathNoExt);
		std::function<void(ds::Block&,CMaterial&)> fLookForTextureAndUpdate = nullptr;
		fLookForTextureAndUpdate = [&fLookForTextureAndUpdate,&pathNoExt,&ext](ds::Block &dataBlock,CMaterial &mat) {
			auto *data = dataBlock.GetData();
			if(data == nullptr)
				return;
			for(auto &pair : *data)
			{
				auto &dataVal = pair.second;
				if(dataVal == nullptr)
					continue;
				if(dataVal->IsBlock())
				{
					fLookForTextureAndUpdate(static_cast<ds::Block&>(*dataVal),mat);
					continue;
				}
				auto *dsTex = dynamic_cast<ds::Texture*>(dataVal.get());
				if(dsTex != nullptr)
				{
					auto &texInfo = dsTex->GetValue();
					if(texInfo.texture == nullptr)
						continue;
					auto texName = texInfo.name;
					ufile::remove_extension_from_filename(texName);
					if(FileManager::ComparePath(texName,pathNoExt) == false)
						continue;
					auto identifier = pair.first;
					auto &texture = texInfo.texture;
					mat.CallOnLoaded([&mat,identifier,texture]() {
						auto &tex = *static_cast<Texture*>(texture.get());
						mat.SetTexture(identifier,&tex);
					});
				}
			}
		};

		// Iterate all materials and update the ones that use this texture
		for(auto &pair : matManager.GetMaterials())
		{
			if(pair.second.IsValid() == false)
				continue;
			auto &data = pair.second.get()->GetDataBlock();
			if(data != nullptr)
				fLookForTextureAndUpdate(*data,static_cast<CMaterial&>(*pair.second.get()));
		}
	};
	texManager.ReloadTexture(path,loadInfo);
}

void CResourceWatcherManager::GetWatchPaths(std::vector<std::string> &paths)
{
	ResourceWatcherManager::GetWatchPaths(paths);
	paths.reserve(paths.size() +2);
	paths.push_back("shaders");
	paths.push_back("particles");
}

void CResourceWatcherManager::OnResourceChanged(const std::string &path,const std::string &ext)
{
	ResourceWatcherManager::OnResourceChanged(path,ext);
	if(ext == "gls" || ext == "hls")
	{
#if RESOURCE_WATCHER_VERBOSE > 0
		auto shaderPath = "shaders\\" +path;
		Con::cout<<"[ResourceWatcher] Shader has changed: "<<shaderPath<<". Attempting to reload..."<<Con::endl;
#endif
		auto canonShader = FileManager::GetCanonicalizedPath(path);
		ufile::remove_extension_from_filename(canonShader);
		ustring::to_lower(canonShader);
		auto &shaderManager = c_engine->GetShaderManager();
		std::vector<std::string> reloadShaders;
		for(auto &pair : shaderManager.GetShaders())
		{
			for(auto &src : pair.second->GetSourceFilePaths())
			{
				auto fname = FileManager::GetCanonicalizedPath(src);
				ufile::remove_extension_from_filename(fname);
				ustring::to_lower(fname);
				if(canonShader == fname)
					reloadShaders.push_back(pair.first);
			}
		}
		for(auto &name : reloadShaders)
		{
#if RESOURCE_WATCHER_VERBOSE > 0
			Con::cout<<"[ResourceWatcher] Reloading shader '"<<name<<"'..."<<Con::endl;
#endif
			c_engine->ReloadShader(name);
		}
		CallChangeCallbacks(ECResourceWatcherCallbackType::Shader,path,ext);
	}
	else if(ext == "wpt")
	{
		if(pragma::CParticleSystemComponent::IsParticleFilePrecached(path) == false)
			return;
#if RESOURCE_WATCHER_VERBOSE > 0
		auto ptPath = "particles\\" +path;
		Con::cout<<"[ResourceWatcher] Particle has changed: "<<ptPath<<". Attempting to reload..."<<Con::endl;
#endif
		pragma::CParticleSystemComponent::Precache(path,true);
		CallChangeCallbacks(ECResourceWatcherCallbackType::ParticleSystem,path,ext);
	}
}

