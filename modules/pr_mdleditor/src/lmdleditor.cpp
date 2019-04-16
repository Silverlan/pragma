#include "stdafx_mdleditor.h"
#include "lmdleditor.h"
#include "wvmodule.h"
#include "fbx.h"
#include "smd.h"
#include "mdl.h"
#include "phy.h"
#include <pragma/model/animation/animation.h>
#include <pragma/model/animation/skeleton.h>
#include <pragma/lua/classes/lmodel.h>
#include <pragma/lua/libraries/lfile.h>
#include <fsys/filesystem.h>

extern DLLENGINE Engine *engine;

static bool get_lua_data(lua_State *l,NetworkState **nw,VFilePtr &f,std::string &name,Model **mdl,const char *fileMode)
{
	*nw = engine->GetNetworkState(l);
	f = nullptr;
	name = Lua::CheckString(l,1);
	if(Lua::IsFile(l,2))
	{
		auto *lf = Lua::CheckFile(l,2);
		f = lf->GetHandle();
	}
	else
	{
		auto *path = Lua::CheckString(l,2);
		f = FileManager::OpenFile(path,fileMode);
	}
	if(f == nullptr)
		return false;
	*mdl = &Lua::Check<Model>(l,3);
	return true;
}

static void push_textures(lua_State *l,const std::vector<std::string> &textures)
{
	auto tTextures = Lua::CreateTable(l);
	for(auto i=decltype(textures.size()){0};i<textures.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushString(l,textures[i]);
		Lua::SetTableValue(l,tTextures);
	}
}

int Lua::mde::lib::load_fbx(lua_State *l)
{
	NetworkState *nw;
	std::string name;
	::Model *mdl;
	VFilePtr f;
	if(get_lua_data(l,&nw,f,name,&mdl,"rb") == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	std::vector<std::string> textures;
	auto r = import::load_fbx(nw,*mdl,f,textures);
	Lua::PushBool(l,r);
	if(r == true)
	{
		push_textures(l,textures);
		return 2;
	}
	return 1;
}

int Lua::mde::lib::load_smd(lua_State *l)
{
	NetworkState *nw;
	std::string name;
	::Model *mdl;
	VFilePtr f;
	if(get_lua_data(l,&nw,f,name,&mdl,"r") == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto smd = SMDModel::Load(f);
	if(smd == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto bCollision = false;
	if(Lua::IsSet(l,4) == true)
		bCollision = Lua::CheckBool(l,4);
	std::vector<std::string> textures;
	auto r = import::load_smd(nw,name,*mdl,*smd,bCollision,textures);
	Lua::PushBool(l,r);
	if(r == true)
	{
		push_textures(l,textures);
		return 2;
	}
	return 1;
}

int Lua::mde::lib::load_mdl(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto fcreateModel = static_cast<std::shared_ptr<::Model>(Game::*)(bool) const>(&Game::CreateModel);

	std::string name = Lua::CheckString(l,1);

	int32_t t = 2;
	Lua::CheckTable(l,t);
	std::unordered_map<std::string,VFilePtr> files;
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		Lua::PushValue(l,-2);
		std::string ext = Lua::CheckString(l,-1);
		Lua::Pop(l,1);
		auto f = *Lua::CheckFile(l,-1);
		files[ext] = f.GetHandle();
		Lua::Pop(l,1);
	}
	auto bCollision = false;
	if(Lua::IsSet(l,3) == true)
		bCollision = Lua::CheckBool(l,3);
	std::vector<std::string> textures;
	std::vector<std::shared_ptr<::Model>> models;
	auto r = import::load_mdl(nw,files,[fcreateModel,game,&models]() -> std::shared_ptr<::Model> {
		auto mdl = (game->*fcreateModel)(false);
		models.push_back(mdl);
		return mdl;
	},[](const std::shared_ptr<::Model>&,const std::string&,const std::string&) -> bool {return true;},bCollision,textures);
	Lua::PushBool(l,(r != nullptr) ? true : false);
	if(r != nullptr)
	{
		Lua::Push<std::shared_ptr<::Model>>(l,models.front());
		push_textures(l,textures);
		return 2;
	}
	return 1;
	/*
	if(Lua::IsFile(l,2))
	{
		auto *lf = Lua::CheckFile(l,2);
		f = (*lf)->GetHandle();
	}
	else
	{
		auto *path = Lua::CheckString(l,2);
		f = FileManager::OpenFile(path,fileMode);
	}
	if(f == nullptr)
		return false;
	*mdl = Lua::CheckModel(l,3)->get();*/

	/*


	NetworkState *nw;
	std::string name;
	::Model *mdl;
	VFilePtr f;
	if(get_lua_data(l,&nw,f,name,&mdl,"rb") == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto bCollision = false;
	if(Lua::IsSet(l,4) == true)
		bCollision = Lua::CheckBool(l,4);
	std::vector<std::string> textures;
	auto r = import::load_mdl(nw,f,*mdl,bCollision,textures);
	Lua::PushBool(l,r);
	if(r == true)
	{
		push_textures(l,textures);
		return 2;
	}
	return 1;*/
}

int Lua::mde::lib::load_phy(lua_State *l)
{
	NetworkState *nw;
	std::string name;
	::Model *mdl;
	VFilePtr f;
	if(get_lua_data(l,&nw,f,name,&mdl,"rb") == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	//import::mdl::phy::load_phy("",);
	// TODO Read phy
	return 0;
}

//#include <sharedutils/util_file.h>
//namespace Lua
//{
//	namespace mde
//	{
//		namespace lib
//		{
//			int port_hl2_models(lua_State *l);
//		};
//	};
//};
//int Lua::mde::lib::port_hl2_models(lua_State *l)
//{
//	int32_t t = 1;
//	Lua::CheckTable(l,t);
//	std::vector<std::string> mdlNames;
//	auto numNames = Lua::GetObjectLength(l,t);
//	mdlNames.reserve(numNames);
//	for(auto i=decltype(numNames){0};i<numNames;++i)
//	{
//		Lua::PushInt(l,i +1); /* 1 */
//		Lua::GetTableValue(l,t);
//
//		auto *name = Lua::CheckString(l,-1);
//		mdlNames.push_back(name);
//
//		Lua::Pop(l,1);
//	}
//
//	t = 2;
//	Lua::CheckTable(l,t);
//	std::vector<std::shared_ptr<::Model>> models;
//	auto numMdls = Lua::GetObjectLength(l,t);
//	if(numNames != numMdls)
//		return 0;
//	for(auto i=decltype(numMdls){0};i<numMdls;++i)
//	{
//		Lua::PushInt(l,i +1); /* 1 */
//		Lua::GetTableValue(l,t);
//
//		auto *mdl = Lua::CheckModel(l,-1);
//		models.push_back(*mdl);
//
//		Lua::Pop(l,1);
//	}
//
//	auto rot = uquat::create(EulerAngles(0.f,-90.f,0.f));
//	auto *nw = engine->GetNetworkState(l);
//	for(auto i=decltype(mdlNames.size()){0};i<mdlNames.size();++i)
//	{
//		auto &name = mdlNames.at(i);
//
//		const std::string path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\half-life 2\\hl2\\models\\";
//		const std::array<std::string,6> extensions = {
//			"dx80.vtx",
//			"dx90.vtx",
//			"mdl",
//			"phy",
//			"sw.vtx",
//			"vvd"
//		};
//		std::unordered_map<std::string,VFilePtr> files;
//		for(auto &ext : extensions)
//		{
//			auto subPath = path +name +"." +ext;
//			auto f = FileManager::OpenSystemFile(subPath.c_str(),"rb");
//			if(f != nullptr)
//				files[ext] = f;
//		}
//		if(files.find("dx90.vtx") != files.end())
//			files["vtx"] = files["dx90.vtx"];
//		else if(files.find("dx80.vtx") != files.end())
//			files["vtx"] = files["dx80.vtx"];
//		else if(files.find("sw.vtx") != files.end())
//			files["vtx"] = files["sw.vtx"];
//
//		auto &mdl = models.at(i);
//		std::vector<std::string> textures;
//		auto r = import::load_mdl(nw,files,*mdl,true,textures);
//		if(r == false)
//			std::cout<<"Unable to load model '"<<""<<"'!"<<std::endl;
//		else
//		{
//			mdl->Rotate(rot);
//			auto mdlPath = "props_fork\\" +name +".wmd";
//			FileManager::CreatePath((std::string("models\\") +ufile::get_path_from_filename(mdlPath)).c_str());
//			r = mdl->Save(nw->GetGameState(),mdlPath);
//			if(r == false)
//				std::cout<<"Unable to save model '"<<mdlPath<<"'!"<<std::endl;
//		}
//	}
//	return 0;
//}

void Lua::mde::register_lua_library(lua_State *l)
{
	Lua::RegisterLibrary(l,"mde",{
		{"load_smd",Lua::mde::lib::load_smd},
		{"load_fbx",Lua::mde::lib::load_fbx},
		{"load_mdl",Lua::mde::lib::load_mdl},
		{"load_phy",Lua::mde::lib::load_phy}
	});
}
