// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :locale;
import :util.resource_watcher;

decltype(pragma::util::eResourceWatcherCallbackType::Model) pragma::util::eResourceWatcherCallbackType::Model = EResourceWatcherCallbackType::createFromEnum(EResourceWatcherCallbackType::E::Model);
decltype(pragma::util::eResourceWatcherCallbackType::Material) pragma::util::eResourceWatcherCallbackType::Material = EResourceWatcherCallbackType::createFromEnum(EResourceWatcherCallbackType::E::Material);
decltype(pragma::util::eResourceWatcherCallbackType::Texture) pragma::util::eResourceWatcherCallbackType::Texture = EResourceWatcherCallbackType::createFromEnum(EResourceWatcherCallbackType::E::Texture);
decltype(pragma::util::eResourceWatcherCallbackType::Map) pragma::util::eResourceWatcherCallbackType::Map = EResourceWatcherCallbackType::createFromEnum(EResourceWatcherCallbackType::E::Map);
decltype(pragma::util::eResourceWatcherCallbackType::SoundScript) pragma::util::eResourceWatcherCallbackType::SoundScript = EResourceWatcherCallbackType::createFromEnum(EResourceWatcherCallbackType::E::SoundScript);
decltype(pragma::util::eResourceWatcherCallbackType::Sound) pragma::util::eResourceWatcherCallbackType::Sound = EResourceWatcherCallbackType::createFromEnum(EResourceWatcherCallbackType::E::Sound);
decltype(pragma::util::eResourceWatcherCallbackType::Count) pragma::util::eResourceWatcherCallbackType::Count = EResourceWatcherCallbackType::createFromEnum(EResourceWatcherCallbackType::E::Count);
pragma::util::ResourceWatcherManager::ResourceWatcherManager(NetworkState *nw) : m_networkState(nw), m_watcherManager {fs::create_directory_watcher_manager()} {}

void pragma::util::ResourceWatcherManager::Poll()
{
	std::scoped_lock lock {m_watcherMutex};
	for(auto &watcher : m_watchers)
		watcher->Poll();
}

void pragma::util::ResourceWatcherManager::Lock()
{
	std::scoped_lock lock {m_watcherMutex};
	if(m_lockedCount++ > 0)
		return;
	for(auto &dirWatcher : m_watchers)
		dirWatcher->SetEnabled(false);
}
void pragma::util::ResourceWatcherManager::Unlock()
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
pragma::util::ScopeGuard pragma::util::ResourceWatcherManager::ScopeLock()
{
	Lock();
	return ScopeGuard {[this]() { Unlock(); }};
}

bool pragma::util::ResourceWatcherManager::IsLocked() const { return m_lockedCount > 0; }

void pragma::util::ResourceWatcherManager::RegisterTypeHandler(const std::string &ext, const TypeHandler &handler) { m_typeHandlers[ext] = handler; }

void pragma::util::ResourceWatcherManager::ReloadMaterial(const std::string &path)
{
	auto *nw = m_networkState;
	auto *game = m_networkState->GetGameState();
	auto &matManager = nw->GetMaterialManager();
	//if(matManager.FindMaterial(path) == nullptr)
	//	return; // Don't reload the material if it was never requested in the first place
	std::string internalPath;
	if(nw->LoadMaterial(path, true) != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
		Con::COUT << "[ResourceWatcher] Material 'materials\\" << path << "' has been reloaded!" << Con::endl;
#endif
		if(game != nullptr) {
			auto fname = ufile::get_file_from_filename(path);
			ufile::remove_extension_from_filename(fname);
			auto &mdlManager = m_networkState->GetModelManager();
			auto &models = mdlManager.GetCache();
			std::unordered_set<asset::Model *> modelMap;
			for(auto &pair : models) {
				auto asset = mdlManager.GetAsset(pair.second);
				if(!asset)
					continue;
				auto mdl = asset::ModelManager::GetAssetObject(*asset);
				auto &textures = mdl->GetTextures();
				for(auto it = textures.begin(); it != textures.end(); ++it) {
					auto tex = *it;
					string::to_lower(tex);
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

void pragma::util::ResourceWatcherManager::ReloadTexture(const std::string &path) {}

CallbackHandle pragma::util::ResourceWatcherManager::AddChangeCallback(EResourceWatcherCallbackType type, const std::function<void(std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>)> &fcallback)
{
	auto it = m_callbacks.find(type);
	if(it == m_callbacks.end())
		it = m_callbacks.insert(std::make_pair(type, std::vector<CallbackHandle> {})).first;
	it->second.push_back(FunctionCallback<void, std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>::Create(fcallback));
	return it->second.back();
}

void pragma::util::ResourceWatcherManager::CallChangeCallbacks(EResourceWatcherCallbackType type, const std::string &path, const std::string &ext)
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
	return std::find_if(supportedFormats.begin(), supportedFormats.end(), [&ext](const MaterialManager::ImageFormat &format) { return pragma::string::compare(format.extension, ext, false); }) != supportedFormats.end();
}
void pragma::util::ResourceWatcherManager::OnResourceChanged(const Path &rootPath, const Path &path, const std::string &ext)
{
	auto &strPath = path.GetString();
	auto *nw = m_networkState;
	auto *game = nw->GetGameState();
	auto it = m_typeHandlers.find(ext);
	if(it != m_typeHandlers.end()) {
		it->second(path, ext);
		return;
	}
	auto assetType = asset::determine_type_from_extension(ext);
	if(assetType.has_value()) {
		if(*assetType == asset::Type::Model) {
			if(game != nullptr) {
				auto *asset = m_networkState->GetModelManager().FindCachedAsset(strPath);
				if(asset != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
					auto mdlPath = "models\\" + strPath;
					Con::COUT << "[ResourceWatcher] Model has changed: " << mdlPath << ". Attempting to reload..." << Con::endl;
#endif
					auto mdl = game->LoadModel(strPath, true);
					if(mdl != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
						Con::COUT << "[ResourceWatcher] Model has been reloaded, reloading entities..." << Con::endl;
#endif
						ecs::EntityIterator entIt {*game, ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
						entIt.AttachFilter<EntityIteratorFilterComponent>("model");
						for(auto *ent : entIt) {
							auto mdlComponent = ent->GetModelComponent();
							if(!mdlComponent || fs::compare_path(mdlComponent->GetModelName(), strPath) == false)
								continue;
#if RESOURCE_WATCHER_VERBOSE > 0
							Con::COUT << "[ResourceWatcher] Reloading model for entity " << ent->GetClass() << "..." << Con::endl;
#endif
							mdlComponent->SetModel(std::shared_ptr<asset::Model>(nullptr));
							mdlComponent->SetModel(strPath);
						}
					}
				}
			}
			CallChangeCallbacks(eResourceWatcherCallbackType::Model, strPath, ext);
		}
		else if(*assetType == asset::Type::Material) {
#if RESOURCE_WATCHER_VERBOSE > 0
			auto matPath = "materials\\" + strPath;
			Con::COUT << "[ResourceWatcher] Material has changed: " << matPath << ". Attempting to reload..." << Con::endl;
#endif
			ReloadMaterial(strPath);
			CallChangeCallbacks(eResourceWatcherCallbackType::Material, strPath, ext);
		}
		else if(*assetType == asset::Type::Map)
			CallChangeCallbacks(eResourceWatcherCallbackType::Map, strPath, ext);
		else if(*assetType == asset::Type::Texture) {
#if RESOURCE_WATCHER_VERBOSE > 0
			auto texPath = "materials\\" + strPath;
			Con::COUT << "[ResourceWatcher] Texture has changed: " << texPath << ". Attempting to reload..." << Con::endl;
#endif
			ReloadTexture(strPath);
			auto &matManager = nw->GetMaterialManager();
			for(auto &pair : matManager.GetCache()) // Find all materials which use this texture
			{
				auto asset = matManager.GetAsset(pair.second);
				if(!asset)
					continue;
				auto hMat = material::MaterialManager::GetAssetObject(*asset);
				if(!hMat)
					continue;
				auto *mat = hMat.get();

				auto canonName = fs::get_canonicalized_path(strPath);
				string::to_lower(canonName);
				ufile::remove_extension_from_filename(canonName);

				std::function<bool(const Path &path)> fHasTexture = nullptr;
				fHasTexture = [mat, &canonName, &fHasTexture](const Path &path) -> bool {
					for(auto &name : material::MaterialPropertyBlockView {*mat, path}) {
						auto propType = mat->GetPropertyType(name);
						switch(propType) {
						case material::PropertyType::Block:
							{
								if(fHasTexture(FilePath(path, name)))
									return true;
								break;
							}
						case material::PropertyType::Texture:
							{
								std::string texName;
								if(mat->GetProperty(FilePath(path, name).GetString(), &texName)) {
									texName = fs::get_canonicalized_path(texName);
									string::to_lower(texName);
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
			CallChangeCallbacks(eResourceWatcherCallbackType::Texture, strPath, ext);
		}
		else if(*assetType == asset::Type::Sound) {
			if(game != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
				auto sndPath = "sounds\\" + strPath;
				Con::COUT << "[ResourceWatcher] Sound has changed: " << sndPath << ". Attempting to reload..." << Con::endl;
#endif
				// TODO: Reload sounds if they had been loaded previously
				game->GetNetworkState()->PrecacheSound(strPath, audio::ALChannel::Both); // TODO: Only precache whatever's been requested before?
			}
			CallChangeCallbacks(eResourceWatcherCallbackType::Sound, strPath, ext);
		}
	}
	else if(rootPath == "scripts/sounds/") {
		if(pragma::string::compare<std::string>(ext, "udm", false)) {
			if(game != nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
				auto scriptPath = "scripts\\" + strPath;
				Con::COUT << "[ResourceWatcher] Sound-script has changed: " << scriptPath << ". Attempting to reload..." << Con::endl;
#endif
				// TODO: Reload sound-scripts if they had been loaded previously
				game->LoadSoundScripts(strPath.c_str()); // TODO: Only reload if they have been requested before?
			}
			CallChangeCallbacks(eResourceWatcherCallbackType::SoundScript, strPath, ext);
		}
	}
	else if(rootPath == "scripts/localization/")
		locale::reload_files();
}

void pragma::util::ResourceWatcherManager::OnResourceChanged(const Path &rootPath, const Path &path)
{
	fs::update_file_index_cache((rootPath + path).GetString());
	/*std::string absPath;
	if(fs::find_absolute_path(rootPath +'/' +path,absPath))
	{
		std::string path;
		if(fs::find_relative_path(absPath, path)) {
			absPath = path;
			fs::update_file_index_cache(absPath);
		}
	}*/
	auto ext = path.GetFileExtension();
	if(!ext)
		return;
#if RESOURCE_WATCHER_VERBOSE > 0
	Con::COUT << "[ResourceWatcher] File changed: " << path << " (" << ext << ")" << Con::endl;
#endif
	OnResourceChanged(rootPath, path, *ext);
}

void pragma::util::ResourceWatcherManager::GetWatchPaths(std::vector<std::string> &paths) { paths = {"models", "materials", "sounds", "scripts/sounds", "scripts/localization", "maps"}; }

bool pragma::util::ResourceWatcherManager::MountDirectory(const std::string &path, bool bAbsolutePath)
{
	std::vector<std::string> watchPaths;
	GetWatchPaths(watchPaths);
	for(auto &path : watchPaths)
		path = fs::get_canonicalized_path(path);
	try {
		auto watchFlags = fs::DirectoryWatcherCallback::WatchFlags::WatchSubDirectories;
		if(bAbsolutePath)
			watchFlags |= fs::DirectoryWatcherCallback::WatchFlags::AbsolutePath;
		if(m_lockedCount > 0)
			watchFlags |= fs::DirectoryWatcherCallback::WatchFlags::StartDisabled;
		m_watcherMutex.lock();
		m_watchers.reserve(m_watchers.size() + watchPaths.size());
		for(auto &watchPath : watchPaths) {
			auto pwatchPath = DirPath(watchPath);
			m_watchers.push_back(pragma::util::make_shared<fs::DirectoryWatcherCallback>(DirPath(path, pwatchPath).GetString(), [this, pwatchPath = std::move(pwatchPath)](const std::string &fName) { OnResourceChanged(pwatchPath, fName); }, watchFlags, m_watcherManager.get()));
		}
		m_watcherMutex.unlock();
	}
	catch(const fs::DirectoryWatcher::ConstructException &e) {
#if RESOURCE_WATCHER_VERBOSE > 1
		Con::CWAR << "[ResourceWatcher] Unable to mount directory '" << path << "': " << e.what() << Con::endl;
#endif
	}
	return true;
}
