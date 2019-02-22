#include "stdafx_shared.h"
#include "pragma/util/resource_watcher.h"
#include "pragma/lua/lua_script_watcher.h"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/model/model.h"
#include <sharedutils/util_file.h>

decltype(EResourceWatcherCallbackType::Model) EResourceWatcherCallbackType::Model = EResourceWatcherCallbackType{umath::to_integral(E::Model)};
decltype(EResourceWatcherCallbackType::Material) EResourceWatcherCallbackType::Material = EResourceWatcherCallbackType{umath::to_integral(E::Material)};
decltype(EResourceWatcherCallbackType::Texture) EResourceWatcherCallbackType::Texture = EResourceWatcherCallbackType{umath::to_integral(E::Texture)};
decltype(EResourceWatcherCallbackType::Map) EResourceWatcherCallbackType::Map = EResourceWatcherCallbackType{umath::to_integral(E::Map)};
decltype(EResourceWatcherCallbackType::SoundScript) EResourceWatcherCallbackType::SoundScript = EResourceWatcherCallbackType{umath::to_integral(E::SoundScript)};
decltype(EResourceWatcherCallbackType::Sound) EResourceWatcherCallbackType::Sound = EResourceWatcherCallbackType{umath::to_integral(E::Sound)};
decltype(EResourceWatcherCallbackType::Count) EResourceWatcherCallbackType::Count = EResourceWatcherCallbackType{umath::to_integral(E::Count)};
ResourceWatcherManager::ResourceWatcherManager(NetworkState *nw)
	: m_networkState(nw)
{}

void ResourceWatcherManager::Poll()
{
	for(auto &watcher : m_watchers)
		watcher->Poll();
}

void ResourceWatcherManager::ReloadMaterial(const std::string &path)
{
	auto *nw = m_networkState;
	auto *game = m_networkState->GetGameState();
	auto &matManager = nw->GetMaterialManager();
	//if(matManager.FindMaterial(path) == nullptr)
	//	return; // Don't reload the material if it was never requested in the first place
	if(nw->LoadMaterial(path,true) != nullptr)
	{
#if RESOURCE_WATCHER_VERBOSE > 0
		Con::cout<<"[ResourceWatcher] Material 'materials\\"<<path<<"' has been reloaded!"<<Con::endl;
#endif
		if(game != nullptr)
		{
			auto fname = ufile::get_file_from_filename(path);
			ufile::remove_extension_from_filename(fname);
			auto &models = game->GetModels();
			for(auto &pair : models)
			{
				auto &mdl = pair.second;
				auto &textures = mdl->GetTextures();
				for(auto it=textures.begin();it!=textures.end();++it)
				{
					auto tex = *it;
					ustring::to_lower(tex);
					if(tex == fname)
					{
						mdl->PrecacheTexture(it -textures.begin());
						goto loopDone;
					}
				}
			}
			loopDone:;
		}
	}
}

void ResourceWatcherManager::ReloadTexture(const std::string &path) {}

CallbackHandle ResourceWatcherManager::AddChangeCallback(EResourceWatcherCallbackType type,const std::function<void(std::reference_wrapper<const std::string>,std::reference_wrapper<const std::string>)> &fcallback)
{
	auto it = m_callbacks.find(type);
	if(it == m_callbacks.end())
		it = m_callbacks.insert(std::make_pair(type,std::vector<CallbackHandle>{})).first;
	it->second.push_back(FunctionCallback<void,std::reference_wrapper<const std::string>,std::reference_wrapper<const std::string>>::Create(fcallback));
	return it->second.back();
}

void ResourceWatcherManager::CallChangeCallbacks(EResourceWatcherCallbackType type,const std::string &path,const std::string &ext)
{
	auto it = m_callbacks.find(type);
	if(it == m_callbacks.end())
		return;
	for(auto itCb=it->second.begin();itCb!=it->second.end();)
	{
		auto &hCb = *itCb;
		if(hCb.IsValid() == true)
		{
			hCb(std::ref(path),std::ref(ext));
			++itCb;
		}
		else
			it->second.erase(itCb);
	}
	if(it->second.empty())
		m_callbacks.erase(it);
}

void ResourceWatcherManager::OnResourceChanged(const std::string &path,const std::string &ext)
{
	auto *nw = m_networkState;
	auto *game = nw->GetGameState();
	if(ext == "wmd")
	{
		if(game != nullptr)
		{
			auto &mdls = game->GetModels();
			auto it = mdls.find(path);
			if(it != mdls.end())
			{
#if RESOURCE_WATCHER_VERBOSE > 0
				auto mdlPath = "models\\" +path;
				Con::cout<<"[ResourceWatcher] Model has changed: "<<mdlPath<<". Attempting to reload..."<<Con::endl;
#endif
				auto mdl = game->LoadModel(path,true);
				if(mdl != nullptr)
				{
#if RESOURCE_WATCHER_VERBOSE > 0
					Con::cout<<"[ResourceWatcher] Model has been reloaded, reloading entities..."<<Con::endl;
#endif
					EntityIterator entIt {*game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
					entIt.AttachFilter<EntityIteratorFilterComponent>("model");
					for(auto *ent : entIt)
					{
						auto mdlComponent = ent->GetModelComponent();
						if(mdlComponent.expired() || FileManager::ComparePath(mdlComponent->GetModelName(),path) == false)
							continue;
#if RESOURCE_WATCHER_VERBOSE > 0
						Con::cout<<"[ResourceWatcher] Reloading model for entity "<<ent->GetClass()<<"..."<<Con::endl;
#endif
						mdlComponent->SetModel(std::shared_ptr<Model>(nullptr));
						mdlComponent->SetModel(path);
					}
				}
			}
		}
		CallChangeCallbacks(EResourceWatcherCallbackType::Model,path,ext);
	}
	else if(ext == "wmi" || ext == "vmt")
	{
#if RESOURCE_WATCHER_VERBOSE > 0
		auto matPath = "materials\\" +path;
		Con::cout<<"[ResourceWatcher] Material has changed: "<<matPath<<". Attempting to reload..."<<Con::endl;
#endif
		ReloadMaterial(path);
		CallChangeCallbacks(EResourceWatcherCallbackType::Material,path,ext);
	}
	else if(ext == "dds" || ext == "vtf" || ext == "png" || ext == "tga" || ext == "ktx")
	{
#if RESOURCE_WATCHER_VERBOSE > 0
		auto texPath = "materials\\" +path;
		Con::cout<<"[ResourceWatcher] Texture has changed: "<<texPath<<". Attempting to reload..."<<Con::endl;
#endif
		ReloadTexture(path);
		auto &matManager = nw->GetMaterialManager();
		for(auto &pair : matManager.GetMaterials()) // Find all materials which use this texture
		{
			auto &hMat = pair.second;
			if(hMat.IsValid() == false)
				continue;
			auto *mat = hMat.get();
			auto &block = mat->GetDataBlock();
			if(block == nullptr)
				continue;
			auto canonName = FileManager::GetCanonicalizedPath(path);
			ustring::to_lower(canonName);
			ufile::remove_extension_from_filename(canonName);
			
			std::function<bool(const std::shared_ptr<ds::Block>&)> fHasTexture = nullptr;
			fHasTexture = [&fHasTexture,&canonName](const std::shared_ptr<ds::Block> &block) -> bool {
				auto *data = block->GetData();
				if(data != nullptr)
				{
					for(auto &pair : *data)
					{
						auto v = pair.second;
						if(v->IsBlock() == true)
						{
							if(fHasTexture(std::static_pointer_cast<ds::Block>(v)) == true)
								return true;
						}
						else
						{
							auto dataTex = std::dynamic_pointer_cast<ds::Texture>(v);
							if(dataTex != nullptr)
							{
								auto texName = FileManager::GetCanonicalizedPath(dataTex->GetString());
								ustring::to_lower(texName);
								ufile::remove_extension_from_filename(texName);
								if(canonName == texName)
									return true;
							}
						}
					}
				}
				return false;
			};
			if(fHasTexture(block) == true) // Material has texture, reload it
				ReloadMaterial(mat->GetName());
		}
		CallChangeCallbacks(EResourceWatcherCallbackType::Texture,path,ext);
	}
	else if(ext == "txt")
	{
		if(game != nullptr)
		{
#if RESOURCE_WATCHER_VERBOSE > 0
			auto scriptPath = "scripts\\" +path;
			Con::cout<<"[ResourceWatcher] Sound-script has changed: "<<scriptPath<<". Attempting to reload..."<<Con::endl;
#endif
			// TODO: Reload sound-scripts if they had been loaded previously
			game->LoadSoundScripts(path.c_str()); // TODO: Only reload if they have been requested before?
		}
		CallChangeCallbacks(EResourceWatcherCallbackType::SoundScript,path,ext);
	}
	else if(ext == "wld")
		CallChangeCallbacks(EResourceWatcherCallbackType::Map,path,ext);
	else
	{
		auto audioFormats = engine_info::get_supported_audio_formats();
		if(std::find(audioFormats.begin(),audioFormats.end(),ext) != audioFormats.end())
		{
			if(game != nullptr)
			{
#if RESOURCE_WATCHER_VERBOSE > 0
				auto sndPath = "sounds\\" +path;
				Con::cout<<"[ResourceWatcher] Sound has changed: "<<sndPath<<". Attempting to reload..."<<Con::endl;
#endif
				// TODO: Reload sounds if they had been loaded previously
				game->GetNetworkState()->PrecacheSound(path,ALChannel::Both); // TODO: Only precache whatever's been requested before?
			}
			CallChangeCallbacks(EResourceWatcherCallbackType::Sound,path,ext);
		}
	}
}

void ResourceWatcherManager::OnResourceChanged(const std::string &path)
{
	std::string ext;
	if(ufile::get_extension(path,&ext) == false)
		return;
#if RESOURCE_WATCHER_VERBOSE > 0
	Con::cout<<"[ResourceWatcher] File changed: "<<path<<" ("<<ext<<")"<<Con::endl;
#endif
	OnResourceChanged(path,ext);
}

void ResourceWatcherManager::GetWatchPaths(std::vector<std::string> &paths)
{
	paths = {
		"models",
		"materials",
		"sounds",
		"scripts/sounds",
		"maps"
	};
}

bool ResourceWatcherManager::MountDirectory(const std::string &path,bool bAbsolutePath)
{
	std::vector<std::string> watchPaths;
	GetWatchPaths(watchPaths);
	for(auto &path : watchPaths)
		path = FileManager::GetCanonicalizedPath(path);
	try
	{
		auto watchFlags = DirectoryWatcherCallback::WatchFlags::WatchSubDirectories;
		if(bAbsolutePath)
			watchFlags |= DirectoryWatcherCallback::WatchFlags::AbsolutePath;
		m_watchers.push_back(std::make_shared<DirectoryWatcherCallback>(path,[this,watchPaths](const std::string &fName) {
			for(auto &resPath : watchPaths)
			{
				if(ustring::substr(fName,0,resPath.length()) == resPath)
				{
					OnResourceChanged(ustring::substr(fName,resPath.length() +1));
					break;
				}
			}
		},watchFlags));
	}
	catch(const DirectoryWatcher::ConstructException &e)
	{
#if RESOURCE_WATCHER_VERBOSE > 1
		Con::cwar<<"WARNING: [ResourceWatcher] Unable to mount directory '"<<path<<"': "<<e.what()<<Con::endl;
#endif
	}
	return true;
}
