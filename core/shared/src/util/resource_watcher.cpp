/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <material_manager2.hpp>
#include "pragma/util/resource_watcher.h"
#include "pragma/lua/lua_script_watcher.h"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/model/modelmanager.h"
#include "pragma/model/model.h"
#include <material_property_block_view.hpp>
#include <materialmanager.h>
#include <sharedutils/util_file.h>
#include <pragma/asset/util_asset.hpp>

import pragma.locale;

decltype(EResourceWatcherCallbackType::Model) EResourceWatcherCallbackType::Model = EResourceWatcherCallbackType {umath::to_integral(E::Model)};
decltype(EResourceWatcherCallbackType::Material) EResourceWatcherCallbackType::Material = EResourceWatcherCallbackType {umath::to_integral(E::Material)};
decltype(EResourceWatcherCallbackType::Texture) EResourceWatcherCallbackType::Texture = EResourceWatcherCallbackType {umath::to_integral(E::Texture)};
decltype(EResourceWatcherCallbackType::Map) EResourceWatcherCallbackType::Map = EResourceWatcherCallbackType {umath::to_integral(E::Map)};
decltype(EResourceWatcherCallbackType::SoundScript) EResourceWatcherCallbackType::SoundScript = EResourceWatcherCallbackType {umath::to_integral(E::SoundScript)};
decltype(EResourceWatcherCallbackType::Sound) EResourceWatcherCallbackType::Sound = EResourceWatcherCallbackType {umath::to_integral(E::Sound)};
decltype(EResourceWatcherCallbackType::Count) EResourceWatcherCallbackType::Count = EResourceWatcherCallbackType {umath::to_integral(E::Count)};
ResourceWatcherManager::ResourceWatcherManager(NetworkState *nw) : m_networkState(nw) {}

void ResourceWatcherManager::Poll()
{
	std::scoped_lock lock {m_watcherMutex};
	for(auto &watcher : m_watchers)
		watcher->Poll();
}

void ResourceWatcherManager::Lock()
{
	std::scoped_lock lock {m_watcherMutex};
	if(m_lockedCount++ > 0)
		return;
	for(auto &dirWatcher : m_watchers)
		dirWatcher->SetEnabled(false);
}
void ResourceWatcherManager::Unlock()
{
	std::scoped_lock lock {m_watcherMutex};
	assert(m_lockedCount > 0);
	if(m_lockedCount == 0)
		throw std::logic_error {"Attempted to unlock resource watcher more times than it has been locked!"};
	if(--m_lockedCount > 0)
		return;
	for(auto &dirWatcher : m_watchers)
		dirWatcher->SetEnabled(true);
}
util::ScopeGuard ResourceWatcherManager::ScopeLock()
{
	Lock();
	return util::ScopeGuard {[this]() { Unlock(); }};
}

bool ResourceWatcherManager::IsLocked() const { return m_lockedCount > 0; }

void ResourceWatcherManager::RegisterTypeHandler(const std::string &ext, const TypeHandler &handler) { m_typeHandlers[ext] = handler; }

void ResourceWatcherManager::ReloadMaterial(const std::string &path)
{
	auto *nw = m_networkState;
	auto *game = m_networkState->GetGameState();
	auto &matManager = nw->GetMaterialManager();
	//if(matManager.FindMaterial(path) == nullptr)
	//	return; // Don't reload the material if it was never requested in the first place
	std::string internalPath;
	if(nw->LoadMaterial(path, true) != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
		Con::cout << "[ResourceWatcher] Material 'materials\\" << path << "' has been reloaded!" << Con::endl;
#endif
		if(game != nullptr) {
			auto fname = ufile::get_file_from_filename(path);
			ufile::remove_extension_from_filename(fname);
			auto &mdlManager = m_networkState->GetModelManager();
			auto &models = mdlManager.GetCache();
			std::unordered_set<Model *> modelMap;
			for(auto &pair : models) {
				auto asset = mdlManager.GetAsset(pair.second);
				if(!asset)
					continue;
				auto mdl = pragma::asset::ModelManager::GetAssetObject(*asset);
				auto &textures = mdl->GetTextures();
				for(auto it = textures.begin(); it != textures.end(); ++it) {
					auto tex = *it;
					ustring::to_lower(tex);
					if(tex == fname) {
						modelMap.insert(mdl.get());
						mdl->PrecacheTexture(it - textures.begin());
						goto loopDone;
					}
				}
			}
		loopDone:;

			OnMaterialReloaded(path, modelMap);
		}
	}
}

void ResourceWatcherManager::ReloadTexture(const std::string &path) {}

CallbackHandle ResourceWatcherManager::AddChangeCallback(EResourceWatcherCallbackType type, const std::function<void(std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>)> &fcallback)
{
	auto it = m_callbacks.find(type);
	if(it == m_callbacks.end())
		it = m_callbacks.insert(std::make_pair(type, std::vector<CallbackHandle> {})).first;
	it->second.push_back(FunctionCallback<void, std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>::Create(fcallback));
	return it->second.back();
}

void ResourceWatcherManager::CallChangeCallbacks(EResourceWatcherCallbackType type, const std::string &path, const std::string &ext)
{
	auto it = m_callbacks.find(type);
	if(it == m_callbacks.end())
		return;
	for(auto itCb = it->second.begin(); itCb != it->second.end();) {
		auto &hCb = *itCb;
		if(hCb.IsValid() == true) {
			hCb(std::ref(path), std::ref(ext));
			++itCb;
		}
		else
			it->second.erase(itCb);
	}
	if(it->second.empty())
		m_callbacks.erase(it);
}

static bool is_image_format(const std::string &ext)
{
	auto &supportedFormats = MaterialManager::get_supported_image_formats();
	return std::find_if(supportedFormats.begin(), supportedFormats.end(), [&ext](const MaterialManager::ImageFormat &format) { return ustring::compare(format.extension, ext, false); }) != supportedFormats.end();
}
void ResourceWatcherManager::OnResourceChanged(const util::Path &rootPath, const util::Path &path, const std::string &ext)
{
	auto &strPath = path.GetString();
	auto *nw = m_networkState;
	auto *game = nw->GetGameState();
	auto it = m_typeHandlers.find(ext);
	if(it != m_typeHandlers.end()) {
		it->second(path, ext);
		return;
	}
	auto assetType = pragma::asset::determine_type_from_extension(ext);
	if(assetType.has_value()) {
		if(*assetType == pragma::asset::Type::Model) {
			if(game != nullptr) {
				auto *asset = m_networkState->GetModelManager().FindCachedAsset(strPath);
				if(asset != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
					auto mdlPath = "models\\" + strPath;
					Con::cout << "[ResourceWatcher] Model has changed: " << mdlPath << ". Attempting to reload..." << Con::endl;
#endif
					auto mdl = game->LoadModel(strPath, true);
					if(mdl != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
						Con::cout << "[ResourceWatcher] Model has been reloaded, reloading entities..." << Con::endl;
#endif
						EntityIterator entIt {*game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
						entIt.AttachFilter<EntityIteratorFilterComponent>("model");
						for(auto *ent : entIt) {
							auto mdlComponent = ent->GetModelComponent();
							if(!mdlComponent || FileManager::ComparePath(mdlComponent->GetModelName(), strPath) == false)
								continue;
#if RESOURCE_WATCHER_VERBOSE > 0
							Con::cout << "[ResourceWatcher] Reloading model for entity " << ent->GetClass() << "..." << Con::endl;
#endif
							mdlComponent->SetModel(std::shared_ptr<Model>(nullptr));
							mdlComponent->SetModel(strPath);
						}
					}
				}
			}
			CallChangeCallbacks(EResourceWatcherCallbackType::Model, strPath, ext);
		}
		else if(*assetType == pragma::asset::Type::Material) {
#if RESOURCE_WATCHER_VERBOSE > 0
			auto matPath = "materials\\" + strPath;
			Con::cout << "[ResourceWatcher] Material has changed: " << matPath << ". Attempting to reload..." << Con::endl;
#endif
			ReloadMaterial(strPath);
			CallChangeCallbacks(EResourceWatcherCallbackType::Material, strPath, ext);
		}
		else if(*assetType == pragma::asset::Type::Map)
			CallChangeCallbacks(EResourceWatcherCallbackType::Map, strPath, ext);
		else if(*assetType == pragma::asset::Type::Texture) {
#if RESOURCE_WATCHER_VERBOSE > 0
			auto texPath = "materials\\" + strPath;
			Con::cout << "[ResourceWatcher] Texture has changed: " << texPath << ". Attempting to reload..." << Con::endl;
#endif
			ReloadTexture(strPath);
			auto &matManager = nw->GetMaterialManager();
			for(auto &pair : matManager.GetCache()) // Find all materials which use this texture
			{
				auto asset = matManager.GetAsset(pair.second);
				if(!asset)
					continue;
				auto hMat = msys::MaterialManager::GetAssetObject(*asset);
				if(!hMat)
					continue;
				auto *mat = hMat.get();

				auto canonName = FileManager::GetCanonicalizedPath(strPath);
				ustring::to_lower(canonName);
				ufile::remove_extension_from_filename(canonName);

				std::function<bool(const util::Path &path)> fHasTexture = nullptr;
				fHasTexture = [mat, &canonName, &fHasTexture](const util::Path &path) -> bool {
					for(auto &name : msys::MaterialPropertyBlockView {*mat, path}) {
						auto propType = mat->GetPropertyType(name);
						switch(propType) {
						case msys::PropertyType::Block:
							{
								if(fHasTexture(util::FilePath(path, name)))
									return true;
								break;
							}
						case msys::PropertyType::Texture:
							{
								std::string texName;
								if(mat->GetProperty(util::FilePath(path, name).GetString(), &texName)) {
									texName = FileManager::GetCanonicalizedPath(texName);
									ustring::to_lower(texName);
									ufile::remove_extension_from_filename(texName);
									if(canonName == texName)
										return true;
								}
								break;
							}
						}
					}
					return false;
				};
				if(fHasTexture({}) == true) // Material has texture, reload it
				{
					auto matName = mat->GetName();
					// A new material with a different extension may have just been
					// moved into the game files. Remove the extension and let the material
					// system decide which one to load.
					ufile::remove_extension_from_filename(matName);
					ReloadMaterial(matName);
				}
			}
			CallChangeCallbacks(EResourceWatcherCallbackType::Texture, strPath, ext);
		}
		else if(*assetType == pragma::asset::Type::Sound) {
			if(game != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
				auto sndPath = "sounds\\" + strPath;
				Con::cout << "[ResourceWatcher] Sound has changed: " << sndPath << ". Attempting to reload..." << Con::endl;
#endif
				// TODO: Reload sounds if they had been loaded previously
				game->GetNetworkState()->PrecacheSound(strPath, ALChannel::Both); // TODO: Only precache whatever's been requested before?
			}
			CallChangeCallbacks(EResourceWatcherCallbackType::Sound, strPath, ext);
		}
	}
	else if(rootPath == "scripts/sounds/") {
		if(ustring::compare<std::string>(ext, "udm", false)) {
			if(game != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
				auto scriptPath = "scripts\\" + strPath;
				Con::cout << "[ResourceWatcher] Sound-script has changed: " << scriptPath << ". Attempting to reload..." << Con::endl;
#endif
				// TODO: Reload sound-scripts if they had been loaded previously
				game->LoadSoundScripts(strPath.c_str()); // TODO: Only reload if they have been requested before?
			}
			CallChangeCallbacks(EResourceWatcherCallbackType::SoundScript, strPath, ext);
		}
	}
	else if(rootPath == "scripts/localization/")
		pragma::locale::reload_files();
}

void ResourceWatcherManager::OnResourceChanged(const util::Path &rootPath, const util::Path &path)
{
	filemanager::update_file_index_cache((rootPath + path).GetString());
	/*std::string absPath;
	if(FileManager::FindAbsolutePath(rootPath +'/' +path,absPath))
	{
		auto path = util::Path::CreateFile(absPath);
		path.MakeRelative(util::get_program_path());
		absPath = path.GetString();
		filemanager::update_file_index_cache(absPath);
	}*/
	auto ext = path.GetFileExtension();
	if(!ext)
		return;
#if RESOURCE_WATCHER_VERBOSE > 0
	Con::cout << "[ResourceWatcher] File changed: " << path << " (" << ext << ")" << Con::endl;
#endif
	OnResourceChanged(rootPath, path, *ext);
}

void ResourceWatcherManager::GetWatchPaths(std::vector<std::string> &paths) { paths = {"models", "materials", "sounds", "scripts/sounds", "scripts/localization", "maps"}; }

bool ResourceWatcherManager::MountDirectory(const std::string &path, bool bAbsolutePath)
{
	std::vector<std::string> watchPaths;
	GetWatchPaths(watchPaths);
	for(auto &path : watchPaths)
		path = FileManager::GetCanonicalizedPath(path);
	try {
		auto watchFlags = DirectoryWatcherCallback::WatchFlags::WatchSubDirectories;
		if(bAbsolutePath)
			watchFlags |= DirectoryWatcherCallback::WatchFlags::AbsolutePath;
		if(m_lockedCount > 0)
			watchFlags |= DirectoryWatcherCallback::WatchFlags::StartDisabled;
		m_watcherMutex.lock();
		m_watchers.push_back(std::make_shared<DirectoryWatcherCallback>(
		  path,
		  [this, watchPaths](const std::string &fName) {
			  for(auto &resPath : watchPaths) {
				  if(ustring::substr(fName, 0, resPath.length()) == resPath) {
					  OnResourceChanged(util::Path::CreatePath(resPath), util::Path::CreateFile(ustring::substr(fName, resPath.length() + 1)));
					  break;
				  }
			  }
		  },
		  watchFlags));
		m_watcherMutex.unlock();
	}
	catch(const DirectoryWatcher::ConstructException &e) {
#if RESOURCE_WATCHER_VERBOSE > 1
		Con::cwar << "[ResourceWatcher] Unable to mount directory '" << path << "': " << e.what() << Con::endl;
#endif
	}
	return true;
}
