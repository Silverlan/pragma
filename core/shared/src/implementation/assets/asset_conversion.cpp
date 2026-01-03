// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :assets.conversion;

static bool s_bModuleInitialized = false;

static std::shared_ptr<pragma::util::Library> dllHandle = nullptr;
void pragma::util::close_mount_external_library() { dllHandle = nullptr; }
static std::shared_ptr<pragma::util::Library> load_module(pragma::NetworkState *nw)
{
	if(dllHandle == nullptr) {
		std::string err;
		dllHandle = nw->InitializeLibrary("mount_external/pr_mount_external", &err);
		if(dllHandle == nullptr) {
			static auto bPrintError = true;
			if(bPrintError == true) {
				bPrintError = false;
				Con::CWAR << "Unable to load pr_mount_external module: " << err << Con::endl;
			}
			return nullptr;
		}
		auto *fInit = dllHandle->FindSymbolAddress<void (*)()>("initialize_archive_manager");
		if(fInit != nullptr)
			fInit();
		s_bModuleInitialized = true;
	}
	return dllHandle;
}
std::shared_ptr<pragma::util::Library> pragma::util::initialize_external_archive_manager(NetworkState *nw) { return load_module(nw); }
void pragma::util::close_external_archive_manager()
{
	if(s_bModuleInitialized == false)
		return;
	auto dllHandle = load_module(nullptr);
	auto *fClose = dllHandle->FindSymbolAddress<void (*)()>("close_archive_manager");
	if(fClose != nullptr)
		fClose();
}
std::optional<int32_t> pragma::util::get_mounted_game_priority(const std::string &game)
{
	if(s_bModuleInitialized == false)
		return {};
	auto dllHandle = load_module(nullptr);
	auto *fGetMountedGamePriority = dllHandle->FindSymbolAddress<bool (*)(const std::string &, int32_t &)>("get_mounted_game_priority");
	if(fGetMountedGamePriority) {
		int32_t priority;
		auto r = fGetMountedGamePriority(game, priority);
		return r ? priority : std::optional<int32_t> {};
	}
	return {};
}
void pragma::util::set_mounted_game_priority(const std::string &game, int32_t priority)
{
	if(s_bModuleInitialized == false)
		return;
	auto dllHandle = load_module(nullptr);
	auto *fSetMountedGamePriority = dllHandle->FindSymbolAddress<void (*)(const std::string &, int32_t)>("set_mounted_game_priority");
	if(fSetMountedGamePriority != nullptr)
		fSetMountedGamePriority(game, priority);
}

static bool port_model(pragma::NetworkState *nw, const std::string &path, std::string mdlName, const std::string &formatName,
  std::function<bool(pragma::NetworkState *, const std::function<std::shared_ptr<pragma::asset::Model>()> &, const std::function<bool(const std::shared_ptr<pragma::asset::Model> &, const std::string &, const std::string &)> &, const std::string &, const std::string &, std::ostream *)> ptrConvertModel)
{
	if(nw->IsGameActive() == false || pragma::Engine::Get()->ShouldMountExternalGameResources() == false)
		return false;
	//if(fs::exists(path +mdlName) == false) // Could be in bsa archive
	//	return false;
	//Con::COUT<<"Found "<<formatName<<" Model '"<<(path +mdlName)<<"', attempting to port..."<<Con::endl;
	ufile::remove_extension_from_filename(mdlName, pragma::asset::get_supported_extensions(pragma::asset::Type::Model, pragma::asset::FormatType::All));
	auto *game = nw->GetGameState();
	auto fcreateModel = static_cast<std::shared_ptr<pragma::asset::Model> (pragma::Game::*)(bool) const>(&pragma::Game::CreateModel);
	std::vector<std::shared_ptr<pragma::asset::Model>> models;
	if(ptrConvertModel(
	     nw,
	     [fcreateModel, game, &models]() -> std::shared_ptr<pragma::asset::Model> {
		     auto mdl = (game->*fcreateModel)(false);
		     models.push_back(mdl);
		     return mdl;
	     },
	     [game, formatName](const std::shared_ptr<pragma::asset::Model> &mdl, const std::string &path, const std::string &mdlName) {
		     auto outPath = pragma::string::substr(path, 7) // Remove "models/"-prefix
		       + mdlName + '.' + pragma::asset::FORMAT_MODEL_BINARY;
		     if(pragma::fs::create_path((ufile::get_path_from_filename(pragma::util::CONVERT_PATH + "models/" + outPath))) == false)
			     return false;
		     auto f = pragma::fs::open_file<pragma::fs::VFilePtrReal>((pragma::util::CONVERT_PATH + "models/" + outPath).c_str(), pragma::fs::FileMode::Write | pragma::fs::FileMode::Binary);
		     if(f == nullptr) {
			     Con::CWAR << "Unable to save model '" << outPath << "': Unable to open file!" << Con::endl;
			     return false;
		     }
		     mdl->ApplyPostImportProcessing();
		     auto udmData = udm::Data::Create();
		     std::string err;
		     if(mdl->Save(*game, udmData->GetAssetData(), err) == false) {
			     Con::CWAR << "Unable to save model '" << outPath << "': " << err << Con::endl;
			     return false;
		     }
		     auto r = udmData->Save(f);
		     if(r)
			     pragma::fs::update_file_index_cache("models/" + outPath);
		     r = r ? pragma::fs::exists(pragma::util::CONVERT_PATH + "models/" + outPath) : false;
		     if(r == true)
			     Con::COUT << "Successfully ported " << formatName << " Model '" << (path + mdlName) << "' and saved it as '" << outPath << "'!" << Con::endl;
		     return r;
	     },
	     path, mdlName, &std::cout)
	  == false)
		return false;
	return true;
}

void *pragma::util::impl::get_module_func(NetworkState *nw, const std::string &name)
{
	static auto *dllHandle = load_module(nw).get();
	if(dllHandle == nullptr)
		return nullptr;
	return dllHandle->FindSymbolAddress(name);
}

bool pragma::util::port_nif_model(NetworkState *nw, const std::string &path, std::string mdlName)
{
	std::string ext;
	if(ufile::get_extension(mdlName, &ext) == false || ext != "nif")
		return false;
	static auto *ptrConvertModel = reinterpret_cast<bool (*)(NetworkState *, const std::function<std::shared_ptr<asset::Model>()> &, const std::function<bool(const std::shared_ptr<asset::Model> &, const std::string &, const std::string &)> &, const std::string &, const std::string &)>(
	  impl::get_module_func(nw, "convert_nif_model"));
	if(ptrConvertModel == nullptr)
		return false;
	auto lockWatcher = Engine::Get()->ScopeLockResourceWatchers();
	return port_model(nw, path, mdlName, "nif",
	  [](NetworkState *nw, const std::function<std::shared_ptr<asset::Model>()> &fCreateModel, const std::function<bool(const std::shared_ptr<asset::Model> &, const std::string &, const std::string &)> &fCallback, const std::string &path, const std::string &mdlName,
	    std::ostream *optLog) -> bool { return ptrConvertModel(nw, fCreateModel, fCallback, path, mdlName); });
}

bool pragma::util::port_hl2_particle(NetworkState *nw, const std::string &path)
{
	static auto *ptrLoadParticle = reinterpret_cast<bool (*)(NetworkState &, const std::string &)>(impl::get_module_func(nw, "load_source_particle"));
	if(ptrLoadParticle == nullptr)
		return false;
	auto lockWatcher = Engine::Get()->ScopeLockResourceWatchers();
	return ptrLoadParticle(*nw, path);
}

bool pragma::util::port_source2_model(NetworkState *nw, const std::string &path, std::string mdlName)
{
	ufile::remove_extension_from_filename(mdlName, std::array<std::string, 2> {"vmdl", "vmdl_c"});
	mdlName += ".vmdl_c";
	static auto *ptrConvertModel
	  = reinterpret_cast<bool (*)(NetworkState *nw, const std::function<std::shared_ptr<asset::Model>()> &, const std::function<bool(const std::shared_ptr<asset::Model> &, const std::string &, const std::string &)> &, const std::string &, const std::string &, std::ostream *)>(
	    impl::get_module_func(nw, "convert_source2_model"));
	if(ptrConvertModel == nullptr)
		return false;
	auto lockWatcher = Engine::Get()->ScopeLockResourceWatchers();
	return port_model(nw, path, mdlName, "source2", ptrConvertModel);
}

bool pragma::util::port_hl2_model(NetworkState *nw, const std::string &path, std::string mdlName)
{
	ufile::remove_extension_from_filename(mdlName, std::array<std::string, 1> {"mdl"});
	mdlName += ".mdl";
	static auto *ptrConvertModel
	  = reinterpret_cast<bool (*)(NetworkState *nw, const std::function<std::shared_ptr<asset::Model>()> &, const std::function<bool(const std::shared_ptr<asset::Model> &, const std::string &, const std::string &)> &, const std::string &, const std::string &, std::ostream *)>(
	    impl::get_module_func(nw, "convert_hl2_model"));
	if(ptrConvertModel == nullptr)
		return false;
	auto lockWatcher = Engine::Get()->ScopeLockResourceWatchers();
	return port_model(nw, path, mdlName, "HL2", ptrConvertModel);
}

bool pragma::util::port_hl2_smd(NetworkState &nw, asset::Model &mdl,fs::VFilePtr &f, const std::string &animName, bool isCollisionMesh, std::vector<std::string> &outTextures)
{
	static auto *ptrConvertSmd = reinterpret_cast<bool (*)(NetworkState &, asset::Model &,fs::VFilePtr &, const std::string &, bool, std::vector<std::string> &)>(impl::get_module_func(&nw, "convert_smd"));
	if(ptrConvertSmd == nullptr)
		return false;
	auto lockWatcher = nw.GetResourceWatcher().ScopeLock();
	return ptrConvertSmd(nw, mdl, f, animName, isCollisionMesh, outTextures);
}

bool pragma::util::port_file(NetworkState *nw, const std::string &path, const std::optional<std::string> &optOutputPath)
{
	if(Engine::Get()->ShouldMountExternalGameResources() == false)
		return false;
	auto outputPath = optOutputPath.has_value() ? *optOutputPath : path;
	if(fs::exists(outputPath))
		return true;
	if(outputPath != path && fs::exists(path)) {
		auto fullOutputPath = IMPORT_PATH + outputPath;
		fs::create_path(ufile::get_path_from_filename(fullOutputPath));
		return fs::copy_file(path, fullOutputPath);
	}
	auto dllHandle = load_module(nw);
	if(dllHandle == nullptr)
		return false;
	static auto *ptrExtractResource = dllHandle->FindSymbolAddress<bool (*)(NetworkState *, const std::string &, const std::string &)>("extract_resource");
	if(ptrExtractResource == nullptr)
		return false;
	auto lockWatcher = Engine::Get()->ScopeLockResourceWatchers();
	return ptrExtractResource(nw, path, IMPORT_PATH + outputPath);
}
