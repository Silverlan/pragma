#include "mdl.h"
#include "wv_source.hpp"
#include "nif.hpp"
#include "fbx.h"
#include <pragma/lua/libraries/lfile.h>
#include <pragma/pragma_module.hpp>
#include <pragma/ishared.hpp>
#include <pragma/engine.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game.h>
#include <mathutil/uquat.h>
#include <functional>
#include <pragma/model/model.h>
#include <mathutil/eulerangles.h>
#include <fsys/filesystem.h>
#include <util_archive.hpp>
#include <sharedutils/util_file.h>
#include <luasystem.h>
#include <luainterface.hpp>

#pragma comment(lib,"libfbxsdk-md.lib")
#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"engine.lib")
#pragma comment(lib,"ishared.lib")
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"util_archive.lib")
#pragma comment(lib,"niflib_static.lib")

uint32_t import::util::add_texture(NetworkState &nw,Model &mdl,const std::string &name)
{
	auto fname = name;
	std::string ext;
	if(ufile::get_extension(name,&ext) == true)
		fname = fname.substr(0,fname.length() -(ext.length() +1));
	auto &meta = mdl.GetMetaInfo();
	auto it = std::find(meta.textures.begin(),meta.textures.end(),fname);
	auto idx = 0u;
	if(it != meta.textures.end())
		idx = it -meta.textures.begin();
	else
	{
		meta.textures.push_back(fname);
		idx = meta.textures.size() -1;
	}
	auto *texGroup = mdl.GetTextureGroup(0);
	if(texGroup == nullptr)
		texGroup = mdl.CreateTextureGroup();
	texGroup->textures.push_back(idx);
	auto *mat = nw.LoadMaterial(name);
	if(mat == nullptr)
		mat = nw.GetMaterialManager().GetErrorMaterial();
	if(mat != nullptr)
		mdl.AddMaterial(0,mat);
	return idx;
}

static bool write_data(const std::string &fpath,const std::vector<uint8_t> &data,const std::string &outRoot="")
{
	auto path = ufile::get_path_from_filename(outRoot +fpath);
	FileManager::CreatePath(path.c_str());
	auto fOut = FileManager::OpenFile<VFilePtrReal>((outRoot +fpath).c_str(),"wb");
	if(fOut == nullptr)
		return false;
	fOut->Write(data.data(),data.size());
	return true;
}

extern DLLENGINE Engine *engine;

class Model;
class NetworkState;
extern "C" {
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &lua)
	{
		auto &libSteamWorks = lua.RegisterLibrary("import",{
			{"import_fbx",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {

				auto &f = *Lua::CheckFile(l,1);
				auto &mdl = Lua::Check<std::shared_ptr<Model>>(l,2);

				std::vector<std::string> textures {};
				auto bSuccess = import::load_fbx(engine->GetNetworkState(l),*mdl,f.GetHandle(),textures);
				Lua::PushBool(l,bSuccess);
				return 1;
			})},
		});
	}
	PRAGMA_EXPORT void initialize_archive_manager() {uarch::initialize();}
	PRAGMA_EXPORT void close_archive_manager() {uarch::close();}
	PRAGMA_EXPORT void find_files(const std::string &path,std::vector<std::string> *outFiles,std::vector<std::string> *outDirectories) {uarch::find_files(path,outFiles,outDirectories);}
	PRAGMA_EXPORT void open_archive_file(const std::string &path,VFilePtr &f)
	{
		f = FileManager::OpenFile(path.c_str(),"rb");
		if(f == nullptr)
			f = uarch::load(path);
	}
	PRAGMA_EXPORT bool extract_resource(NetworkState *nw,const std::string &fpath,const std::string &outRoot)
	{
		auto f = uarch::load(fpath);
		auto fv = std::dynamic_pointer_cast<VFilePtrInternalVirtual>(f);
		if(fv != nullptr)
		{
			auto data = fv->GetData();
			if(data == nullptr)
				return false;
			return write_data(fpath,*data,outRoot);
		}
		auto fr = std::dynamic_pointer_cast<VFilePtrInternalReal>(f);
		if(fr != nullptr)
		{
			std::vector<uint8_t> data(fr->GetSize());
			fr->Read(data.data(),data.size());
			return write_data(fpath,data,outRoot);
		}
		return false;
	}
	PRAGMA_EXPORT bool convert_hl2_model(NetworkState *nw,const std::function<std::shared_ptr<Model>()> &fCreateModel,const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)> &fCallback,const std::string &path,const std::string &mdlName)
	{
		auto rot = uquat::create(EulerAngles(0.f,-90.f,0.f));

		const std::array<std::string,7> extensions = {
			"dx80.vtx",
			"dx90.vtx",
			"mdl",
			"phy",
			"sw.vtx",
			"vvd",
			"ani"
		};
		std::unordered_map<std::string,VFilePtr> files;
		for(auto &ext : extensions)
		{
			auto subPath = path +mdlName +"." +ext;
			auto f = FileManager::OpenFile(subPath.c_str(),"rb");
			if(f == nullptr)
				f = uarch::load(subPath);
			if(f != nullptr)
				files[ext] = f;
		}
		if(files.find("dx90.vtx") != files.end())
			files["vtx"] = files["dx90.vtx"];
		else if(files.find("dx80.vtx") != files.end())
			files["vtx"] = files["dx80.vtx"];
		else if(files.find("sw.vtx") != files.end())
			files["vtx"] = files["sw.vtx"];

		std::vector<std::string> textures;
		auto r = ::import::load_mdl(nw,files,fCreateModel,fCallback,true,textures);
		if(r == nullptr)
			return false;
		r->Rotate(rot);
		return fCallback(r,path,mdlName);
	}
	PRAGMA_EXPORT bool convert_nif_model(NetworkState *nw,const std::function<std::shared_ptr<Model>()> &fCreateModel,const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)> &fCallback,const std::string &pathRoot,const std::string &mdlName)
	{
		auto mdl = fCreateModel();
		auto fpath = pathRoot +mdlName +".nif";
		auto path = ufile::get_path_from_filename(fpath);
		::import::load_nif(nw,mdl,path +"skeleton.nif"); // Attempt to load skeleton before loading actual mesh (To retrieve correct bone hierarchy)
		if(::import::load_nif(nw,mdl,fpath) == false)
			return false;

		auto numMeshGroups = mdl->GetMeshGroupCount();
		for(auto i=decltype(numMeshGroups){0};i<numMeshGroups;++i)
			mdl->GetBaseMeshes().push_back(i);

		auto refAnim = Animation::Create();
		auto &skeleton = mdl->GetSkeleton();
		auto numBones = skeleton.GetBoneCount();
		auto &boneList = refAnim->GetBoneList();
		refAnim->ReserveBoneIds(refAnim->GetBoneCount() +numBones);
		for(auto i=decltype(numBones){0};i<numBones;++i)
			refAnim->AddBoneId(i);
		auto refFrame = Frame::Create(mdl->GetReference());
		refAnim->AddFrame(refFrame);
		mdl->AddAnimation("reference",refAnim);
		//refFrame->Localize(*refAnim,skeleton);
		//mdl->GetReference().Localize(*refAnim,skeleton);
		mdl->GetReference().Globalize(*refAnim,skeleton);

		//mdl->GenerateBindPoseMatrices();

		std::function<void(const std::string&,uint32_t)> fLoadAnimations = nullptr;
		fLoadAnimations = [&fLoadAnimations,nw,&mdl](const std::string &path,uint32_t depth) {
			std::vector<std::string> files;
			FileManager::FindFiles((path +"*.kf").c_str(),&files,nullptr);
			uarch::find_files(path +"*kf",&files,nullptr);
			// TODO: Find in archive
			for(auto &f : files)
			{
				try
				{
					::import::load_nif(nw,mdl,path +f);
				}
				catch(const std::exception &e)
				{
					std::cout<<"ex: "<<e.what()<<std::endl;
				}
			}
			if(depth == 0)
				return;
			std::vector<std::string> dirs;
			FileManager::FindFiles((path +"*").c_str(),nullptr,&dirs);
			uarch::find_files(path +"*",nullptr,&dirs);
			for(auto &dir : dirs)
				fLoadAnimations(path +dir +"\\",depth -1);
		};
		fLoadAnimations(path,4);

		auto &textures = mdl->GetTextures();
		auto *texGroup = mdl->CreateTextureGroup();
		texGroup->textures.reserve(textures.size());
		for(auto i=decltype(textures.size()){0};i<textures.size();++i)
			texGroup->textures.push_back(i); // TODO: Generate material files
		return fCallback(mdl,pathRoot,mdlName);
	}
};
