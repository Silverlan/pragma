// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :util.resource_watcher;
import :engine;
import :entities.components.particle_system;
import :game;
import pragma.shadergraph;

decltype(pragma::util::ECResourceWatcherCallbackType::Shader) pragma::util::ECResourceWatcherCallbackType::Shader = ECResourceWatcherCallbackType {pragma::math::to_integral(E::Shader)};
decltype(pragma::util::ECResourceWatcherCallbackType::ParticleSystem) pragma::util::ECResourceWatcherCallbackType::ParticleSystem = ECResourceWatcherCallbackType {pragma::math::to_integral(E::ParticleSystem)};
decltype(pragma::util::ECResourceWatcherCallbackType::Count) pragma::util::ECResourceWatcherCallbackType::Count = ECResourceWatcherCallbackType {pragma::math::to_integral(E::Count)};
static auto cvMatStreaming = pragma::console::get_client_con_var("cl_material_streaming_enabled");

pragma::util::CResourceWatcherManager::CResourceWatcherManager(pragma::NetworkState *nw) : ResourceWatcherManager(nw) {}

void pragma::util::CResourceWatcherManager::ReloadTexture(const std::string &path)
{
	auto *nw = m_networkState;
	auto &matManager = static_cast<material::CMaterialManager &>(nw->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto *asset = texManager.FindCachedAsset(path);
	if(asset == nullptr)
		return;
	texManager.RemoveFromCache(path);
	auto loadInfo = std::make_unique<material::TextureLoadInfo>();
	loadInfo->onLoaded = [path, nw](pragma::util::Asset &asset) {
		if(nw == nullptr)
			return;
		auto &matManager = static_cast<material::CMaterialManager &>(nw->GetMaterialManager());
		auto &texManager = matManager.GetTextureManager();

		auto pathNoExt = path;
		std::string ext;
		auto bExt = ufile::get_extension(path, &ext);
		if(bExt)
			ext = '.' + ext;
		ufile::remove_extension_from_filename(pathNoExt);

		std::function<void(material::CMaterial &, const pragma::util::Path &path)> fLookForTextureAndUpdate = nullptr;
		fLookForTextureAndUpdate = [&fLookForTextureAndUpdate, &pathNoExt](material::CMaterial &mat, const pragma::util::Path &path) {
			for(auto &name : material::MaterialPropertyBlockView {mat, path}) {
				auto propType = mat.GetPropertyType(name);
				switch(propType) {
				case material::PropertyType::Block:
					fLookForTextureAndUpdate(mat, pragma::util::FilePath(path, name));
					break;
				case material::PropertyType::Texture:
					{
						std::string texName;
						if(!mat.GetProperty(pragma::util::FilePath(path, name).GetString(), &texName))
							continue;
						auto *texInfo = mat.GetTextureInfo(name);
						if(!texInfo)
							continue;
						ufile::remove_extension_from_filename(texName);
						if(fs::compare_path(texName, pathNoExt) == false)
							continue;
						auto &identifier = name;
						auto &texture = texInfo->texture;
						mat.CallOnLoaded([&mat, identifier, texture]() {
							auto &tex = *static_cast<material::Texture *>(texture.get());
							mat.SetTexture(std::string {identifier}, &tex);
						});
						break;
					}
				}
			}
		};

		// Iterate all materials and update the ones that use this texture
		for(auto &pair : matManager.GetCache()) {
			auto asset = matManager.GetAsset(pair.second);
			if(!asset)
				continue;
			auto hMat = material::CMaterialManager::GetAssetObject(*asset);
			if(!hMat)
				continue;
			fLookForTextureAndUpdate(static_cast<material::CMaterial &>(*hMat.get()), {});
		}
	};
	texManager.LoadAsset(path, std::move(loadInfo));
}

void pragma::util::CResourceWatcherManager::OnMaterialReloaded(const std::string &path, const std::unordered_set<pragma::asset::Model *> &modelMap)
{
	ResourceWatcherManager::OnMaterialReloaded(path, modelMap);
	if(pragma::get_cgame() == nullptr)
		return;
	pragma::ecs::EntityIterator entIt {*pragma::get_cgame(), pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
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

void pragma::util::CResourceWatcherManager::GetWatchPaths(std::vector<std::string> &paths)
{
	ResourceWatcherManager::GetWatchPaths(paths);
	paths.reserve(paths.size() + 2);
	paths.push_back("shaders");
	paths.push_back("particles");
	paths.push_back("scripts/shader_data");
}

void pragma::util::CResourceWatcherManager::OnResourceChanged(const pragma::util::Path &rootPath, const pragma::util::Path &path, const std::string &ext)
{
	ResourceWatcherManager::OnResourceChanged(rootPath, path, ext);
	auto &strPath = path.GetString();
	auto assetType = pragma::asset::determine_type_from_extension(ext);
	if(assetType.has_value()) {
		if(*assetType == pragma::asset::Type::ParticleSystem) {
			if(pragma::ecs::CParticleSystemComponent::IsParticleFilePrecached(strPath) == false)
				return;
#if RESOURCE_WATCHER_VERBOSE > 0
			auto ptPath = "particles\\" + strPath;
			Con::cout << "[ResourceWatcher] Particle has changed: " << ptPath << ". Attempting to reload..." << Con::endl;
#endif
			pragma::ecs::CParticleSystemComponent::Precache(strPath, true);
			CallChangeCallbacks(pragma::util::ECResourceWatcherCallbackType::ParticleSystem, strPath, ext);
		}
	}
	else if(prosper::glsl::is_glsl_file_extension(ext) || ext == "hlsl") {
#if RESOURCE_WATCHER_VERBOSE > 0
		auto shaderPath = "shaders\\" + strPath;
		Con::cout << "[ResourceWatcher] Shader has changed: " << shaderPath << ". Attempting to reload..." << Con::endl;
#endif
		auto canonShader = fs::get_canonicalized_path(strPath);
		ufile::remove_extension_from_filename(canonShader);
		pragma::string::to_lower(canonShader);
		auto &shaderManager = pragma::get_cengine()->GetShaderManager();
		std::vector<std::string> reloadShaders;
		for(auto &pair : shaderManager.GetShaderNameToIndexTable()) {
			auto *shader = shaderManager.GetShader(pair.second);
			if(shader == nullptr)
				continue;
			for(auto &src : shader->GetSourceFilePaths()) {
				auto fname = fs::get_canonicalized_path(src);
				ufile::remove_extension_from_filename(fname);
				pragma::string::to_lower(fname);
				if(canonShader == fname)
					reloadShaders.push_back(pair.first);
			}
		}
		for(auto &name : reloadShaders) {
#if RESOURCE_WATCHER_VERBOSE > 0
			Con::cout << "[ResourceWatcher] Reloading shader '" << name << "'..." << Con::endl;
#endif
			pragma::get_cengine()->ReloadShader(name);
		}
		CallChangeCallbacks(pragma::util::ECResourceWatcherCallbackType::Shader, strPath, ext);
	}
	else if(ext == pragma::shadergraph::Graph::EXTENSION_ASCII || ext == pragma::shadergraph::Graph::EXTENSION_BINARY) {
		auto &graphManager = pragma::get_cengine()->GetShaderGraphManager();
		std::string name {path.GetFileName()};
		ufile::remove_extension_from_filename(name, std::array<std::string, 2> {pragma::shadergraph::Graph::EXTENSION_ASCII, pragma::shadergraph::Graph::EXTENSION_BINARY});
		std::string err;
		auto graph = graphManager.LoadShader(name, err, false /* reload */);
		if(graph == nullptr) {
#if RESOURCE_WATCHER_VERBOSE > 0
			Con::cwar << "[ResourceWatcher] Failed to reload shader graph '" << name << "': " << err << Con::endl;
#endif
			return;
		}
	}
}
