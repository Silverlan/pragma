#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/model/model.h"
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>

extern DLLENGINE Engine *engine;

#pragma optimize("",off)
static bool s_bModuleInitialized = false;
static std::shared_ptr<util::Library> load_module(NetworkState *nw)
{
	static std::shared_ptr<util::Library> dllHandle = nullptr;
	if(dllHandle == nullptr)
	{
		std::string err;
		dllHandle = nw->LoadLibraryModule("mount_external/pr_mount_external",{},&err);
		if(dllHandle == nullptr)
		{
			static auto bPrintError = true;
			if(bPrintError == true)
			{
				bPrintError = false;
				Con::cwar<<"WARNING: Unable to load wv_external_mount module: "<<err<<Con::endl;
			}
			return nullptr;
		}
		auto *fInit = dllHandle->FindSymbolAddress<void(*)()>("initialize_archive_manager");
		if(fInit != nullptr)
			fInit();
		s_bModuleInitialized = true;
	}
	return dllHandle;
}
void util::initialize_external_archive_manager(NetworkState *nw)
{
	load_module(nw);
}
void util::close_external_archive_manager()
{
	if(s_bModuleInitialized == false)
		return;
	auto dllHandle = load_module(nullptr);
	auto *fClose = dllHandle->FindSymbolAddress<void(*)()>("close_archive_manager");
	if(fClose != nullptr)
		fClose();
}

static bool port_model(
	NetworkState *nw,const std::string &path,std::string mdlName,const std::string &formatName,
	bool(*ptrConvertModel)(
		NetworkState*,
		const std::function<std::shared_ptr<Model>()>&,
		const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)>&,
		const std::string&,const std::string&
	)
)
{
	if(nw->IsGameActive() == false || engine->ShouldMountExternalGameResources() == false)
		return false;
	//if(FileManager::Exists(path +mdlName) == false) // Could be in bsa archive
	//	return false;
	//Con::cout<<"Found "<<formatName<<" Model '"<<(path +mdlName)<<"', attempting to port..."<<Con::endl;
	ufile::remove_extension_from_filename(mdlName);
	auto *game = nw->GetGameState();
	auto fcreateModel = static_cast<std::shared_ptr<Model>(Game::*)(bool) const>(&Game::CreateModel);
	std::vector<std::shared_ptr<Model>> models;
	if(ptrConvertModel(nw,[fcreateModel,game,&models]() -> std::shared_ptr<Model> {
		auto mdl = (game->*fcreateModel)(false);
		models.push_back(mdl);
		return mdl;
	},[game,formatName](const std::shared_ptr<Model> &mdl,const std::string &path,const std::string &mdlName) {
		auto outPath = ustring::substr(path,7) // Remove "models/"-prefix
			+mdlName +".wmd";
		if(FileManager::CreatePath((ufile::get_path_from_filename(util::IMPORT_PATH +"models\\" +outPath)).c_str()) == false)
			return false;
		auto r = false;
		try
		{
			r = mdl->Save(game,outPath,util::IMPORT_PATH);
		}
		catch(const std::logic_error &err)
		{
			Con::cwar<<"WARNING: Unable to save model '"<<outPath<<"': "<<err.what()<<Con::endl;
			return false;
		}
		if(r == false)
			return false;
		r = FileManager::Exists(util::IMPORT_PATH +"models\\" +outPath);
		if(r == true)
			Con::cout<<"Successfully ported "<<formatName<<" Model '"<<(path +mdlName)<<"' and saved it as '"<<outPath<<"'!"<<Con::endl;
		return r;
	},path,mdlName) == false)
		return false;
	return true;
}

void *util::impl::get_module_func(NetworkState *nw,const std::string &name)
{
	static auto dllHandle = load_module(nw);
	if(dllHandle == nullptr)
		return nullptr;
	return dllHandle->FindSymbolAddress(name);
}

bool util::port_nif_model(NetworkState *nw,const std::string &path,std::string mdlName)
{
	std::string ext;
	if(ufile::get_extension(mdlName,&ext) == false || ext != "nif")
		return false;
	static auto *ptrConvertModel = reinterpret_cast<bool(*)(NetworkState*,const std::function<std::shared_ptr<Model>()>&,const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)>&,const std::string&,const std::string&)>(impl::get_module_func(nw,"convert_nif_model"));
	if(ptrConvertModel == nullptr)
		return false;
	return port_model(nw,path,mdlName,"nif",ptrConvertModel);
}

bool util::port_hl2_model(NetworkState *nw,const std::string &path,std::string mdlName)
{
	std::string ext;
	if(ufile::get_extension(mdlName,&ext) == false || ext != "mdl")
		return false;
	static auto *ptrConvertModel = reinterpret_cast<bool(*)(NetworkState*nw,const std::function<std::shared_ptr<Model>()>&,const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)>&,const std::string&,const std::string&)>(impl::get_module_func(nw,"convert_hl2_model"));
	if(ptrConvertModel == nullptr)
		return false;
	return port_model(nw,path,mdlName,"HL2",ptrConvertModel);
}

bool util::port_file(NetworkState *nw,const std::string &path)
{
	if(engine->ShouldMountExternalGameResources() == false)
		return false;
	auto dllHandle = load_module(nw);
	if(dllHandle == nullptr)
		return false;
	static auto *ptrExtractResource = dllHandle->FindSymbolAddress<bool(*)(NetworkState*,const std::string&,const std::string&)>("extract_resource");
	if(ptrExtractResource == nullptr)
		return false;
	return ptrExtractResource(nw,path,util::IMPORT_PATH);
}
#pragma optimize("",on)
