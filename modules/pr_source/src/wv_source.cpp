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
#include <smdmodel.h>
#include <pragma/physics/collisionmesh.h>
#include <pragma/model/modelmesh.h>

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
	ufile::remove_extension_from_filename(fname);
	auto &meta = mdl.GetMetaInfo();
	auto it = std::find(meta.textures.begin(),meta.textures.end(),fname);
	auto idx = 0u;
	if(it != meta.textures.end())
		idx = it -meta.textures.begin();
	else
		idx = mdl.AddTexture(fname,nw.LoadMaterial(fname));

	auto *texGroup = mdl.GetTextureGroup(0);
	if(texGroup == nullptr)
		texGroup = mdl.CreateTextureGroup();
	texGroup->textures.push_back(idx);
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

/// SMD
static uint32_t add_bone(const SMDModel::Node &node,Skeleton &skeleton,std::shared_ptr<Bone> *bone=nullptr)
{
	Model mdl;
	auto &bones = skeleton.GetBones();
	auto it = std::find_if(bones.begin(),bones.end(),[&node](const std::shared_ptr<Bone> &bone) {
		return (node.name == bone->name) ? true : false;
		});
	if(it == bones.end())
	{
		auto *ptrBone = new Bone;
		ptrBone->name = node.name;
		skeleton.AddBone(ptrBone);
		it = bones.end() -1;
	}
	if(bone != nullptr)
		*bone = *it;
	return it -bones.begin();
}

static void update_skeletal_hierarchy(SMDModel &smd,Skeleton &skeleton)
{
	auto &nodes = smd.GetNodes();
	for(auto &node : nodes)
	{
		if(node.parent == -1)
			continue;
		auto bone = skeleton.GetBone(skeleton.LookupBone(node.name));
		auto parentBone = skeleton.GetBone(skeleton.LookupBone(nodes.at(node.parent).name));
		bone.lock()->parent = parentBone;
		parentBone.lock()->children[bone.lock()->ID] = bone.lock();
	}
	for(auto &bone : skeleton.GetBones())
	{
		if(bone->parent.expired() == false)
			continue;
		skeleton.GetRootBones()[bone->ID] = bone;
	}
}

static void add_nodes_to_skeleton(SMDModel &smd,Skeleton &skeleton,Animation *anim)
{
	auto &smdSkeleton = smd.GetSkeleton();
	auto &smdNodes = smd.GetNodes();
	auto &bones = skeleton.GetBones();
	auto *boneList = (anim != nullptr) ? &anim->GetBoneList() : nullptr;
	if(anim)
		anim->ReserveBoneIds(anim->GetBoneCount() +smdNodes.size());
	for(auto i=decltype(smdNodes.size()){0};i<smdNodes.size();++i)
	{
		auto &node = smdNodes[i];
		auto boneId = add_bone(node,skeleton);
		if(boneList != nullptr)
			anim->AddBoneId(boneId);
	}
	update_skeletal_hierarchy(smd,skeleton);
}

static std::vector<uint32_t> get_skeleton_translation_table(SMDModel &smd,Model &mdl)
{
	auto &nodes = smd.GetNodes();
	auto &skeleton = mdl.GetSkeleton();
	auto &bones = skeleton.GetBones();

	std::vector<uint32_t> t;
	t.reserve(nodes.size());
	for(auto &node : nodes)
	{
		auto it = std::find_if(bones.begin(),bones.end(),[&node](const std::shared_ptr<Bone> &bone) {
			return (node.name == bone->name) ? true : false;
			});
		t.push_back((*it)->ID);
	}
	return t;
}

static void generate_reference(SMDModel &smd,Model &mdl)
{
	auto animId = mdl.LookupAnimation("reference");
	if(animId == -1)
	{
		mdl.AddAnimation("reference",Animation::Create());
		animId = mdl.LookupAnimation("reference");
	}

	auto &smdFrame = smd.GetFrames().front();
	auto anim = mdl.GetAnimation(animId);
	auto frame = anim->GetFrame(0);
	if(frame == nullptr)
	{
		frame = Frame::Create(smdFrame.transforms.size());
		anim->AddFrame(frame);
	}

	auto &nodes = smd.GetNodes();
	auto &skeleton = mdl.GetSkeleton();
	for(auto i=decltype(smdFrame.transforms.size()){0};i<smdFrame.transforms.size();++i)
	{
		auto &t = smdFrame.transforms[i];
		auto &node = nodes[i];
		auto boneId = add_bone(node,skeleton);
		frame->SetBoneCount(umath::max(frame->GetBoneCount(),boneId +1));

		auto &pos = t.position;
		auto &rot = t.rotation;
		//to_local_bone_system(smdFrame,node,pos,rot);

		frame->SetBonePosition(boneId,pos);
		frame->SetBoneOrientation(boneId,rot);

		auto m = glm::toMat4(t.rotation);
		m = glm::translate(m,t.position);
		mdl.SetBindPoseBoneMatrix(boneId,glm::inverse(m));
	}
	update_skeletal_hierarchy(smd,skeleton);
	auto refFrame = Frame::Create(*frame);
	frame->Localize(*anim,mdl.GetSkeleton());
	mdl.SetReference(refFrame);
}

static void to_local_bone_system(const SMDModel::Frame &frame,const SMDModel::Node &node,Vector3 &pos,Quat &rot)
{
	auto parentId = node.parent;
	if(parentId != -1)
	{
		auto &tParent = frame.transforms[parentId];
		pos -= tParent.position;
		auto inv = uquat::get_inverse(tParent.rotation);
		uvec::rotate(&pos,inv);

		rot = inv *rot;
	}
}

static void update_reference(Model &mdl,Animation &anim) // Adds all new bones to the reference pose
{
	auto numBones = mdl.GetBoneCount();
	auto &animBoneList = anim.GetBoneList();
	auto &reference = mdl.GetReference();
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto *mat = mdl.GetBindPoseBoneMatrix(i);
		if(mat == nullptr)
		{
			reference.SetBoneCount(umath::max(reference.GetBoneCount(),i +1));
			auto it = std::find(animBoneList.begin(),animBoneList.end(),i);
			if(it != animBoneList.end())
			{
				auto animBoneId = it -animBoneList.begin();
				auto &frame = *anim.GetFrame(0);
				auto &pos = *frame.GetBonePosition(animBoneId);
				auto &rot = *frame.GetBoneOrientation(animBoneId);

				reference.SetBonePosition(i,pos);
				reference.SetBoneOrientation(i,rot);

				auto m = glm::toMat4(rot);
				m = glm::translate(m,pos);
				mdl.SetBindPoseBoneMatrix(i,glm::inverse(m));
			}
			else
			{
				reference.SetBonePosition(i,{});
				reference.SetBoneOrientation(i,{});
				mdl.SetBindPoseBoneMatrix(i,{});
			}
		}
	}
}

static void generate_animation(SMDModel &smd,Model &mdl,Animation &anim)
{
	auto &smdFrames = smd.GetFrames();
	auto numBones = anim.GetBoneCount();
	auto &nodes = smd.GetNodes();
	for(auto &smdFrame : smdFrames)
	{
		auto frame = Frame::Create(numBones);
		for(auto i=decltype(smdFrame.transforms.size()){0};i<smdFrame.transforms.size();++i)
		{
			auto &node = nodes[i];
			auto &t = smdFrame.transforms[i];

			auto pos = t.position;
			auto rot = t.rotation;
			to_local_bone_system(smdFrame,node,pos,rot);

			frame->SetBonePosition(i,pos);
			frame->SetBoneOrientation(i,rot);
		}
		anim.AddFrame(frame);
	}
	update_reference(mdl,anim);
}

static bool load_smd(NetworkState *nw,const std::string &name,Model &mdl,SMDModel &smd,bool bCollision,std::vector<std::string> &textures)
{
	auto *game = nw->GetGameState();
	auto &meshes = smd.GetMeshes();
	if(bCollision == true)
	{
		for(auto &mesh : meshes)
		{
			auto colMesh = CollisionMesh::Create(game);
			for(auto &tri : mesh.triangles)
			{
				for(uint8_t i=0;i<3;++i)
					colMesh->AddVertex(tri.vertices[i].position);
			}
			colMesh->Update();
			mdl.AddCollisionMesh(colMesh);
		}
		mdl.Update();
		return true;
	}
	auto anim = Animation::Create();
	mdl.GetSkeleton().GetBones().clear();
	mdl.GetSkeleton().GetRootBones().clear();
	add_nodes_to_skeleton(smd,mdl.GetSkeleton(),anim.get());
	if(!meshes.empty())
	{
		auto mdlMesh = std::shared_ptr<ModelMesh>(nw->CreateMesh())->shared_from_this();
		auto boneTranslation = get_skeleton_translation_table(smd,mdl);

		auto numMats = mdl.GetMaterials().size();
		textures.reserve(textures.size() +smd.GetMeshes().size());
		for(auto &mesh : smd.GetMeshes())
		{
			auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
			auto &verts = subMesh->GetVertices();
			for(auto &tri : mesh.triangles)
			{
				std::array<uint32_t,3> triangleIndices = {0,0,0};
				for(uint8_t i=0;i<3;++i)
				{
					auto &triVert = tri.vertices[i];
					auto it = std::find_if(verts.begin(),verts.end(),[&triVert](const Vertex &v) {
						return (umath::abs(triVert.position.x -v.position.x) <= VERTEX_EPSILON && umath::abs(triVert.position.y -v.position.y) <= VERTEX_EPSILON && umath::abs(triVert.position.z -v.position.z) <= VERTEX_EPSILON) ? true : false;
						});
					if(it == verts.end())
					{
						auto vertId = subMesh->AddVertex({triVert.position,{triVert.uv.x,-triVert.uv.y},triVert.normal});
						triangleIndices[i] = subMesh->GetVertexCount() -1;

						VertexWeight w {};
						uint8_t c = 0;
						for(auto &pair : triVert.weights)
						{
							if(pair.first != -1)
							{
								w.boneIds[c] = boneTranslation[pair.first];
								w.weights[c] = pair.second;
								if(++c == 4)
									break;
							}
						}
						subMesh->SetVertexWeight(vertId,w);

						it = verts.end() -1;
					}
					else
						triangleIndices[i] = it -verts.begin();
				}
				subMesh->AddTriangle(triangleIndices[0],triangleIndices[1],triangleIndices[2]);
			}
			textures.push_back(mesh.texture);
			auto texId = import::util::add_texture(*nw,mdl,mesh.texture);
			subMesh->SetSkinTextureIndex(texId);
			subMesh->Update();
			mdlMesh->AddSubMesh(subMesh);
		}
		mdlMesh->Update();
		auto meshGroup = mdl.GetMeshGroup(0);
		if(meshGroup == nullptr)
			meshGroup = mdl.AddMeshGroup("reference");
		meshGroup->AddMesh(mdlMesh);
		generate_reference(smd,mdl);
	}
	generate_animation(smd,mdl,*anim);
	mdl.AddAnimation(name,anim);
	return true;
}
///

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
				auto fHandle = f.GetHandle();
				auto bSuccess = import::load_fbx(engine->GetNetworkState(l),*mdl,fHandle,textures);
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
	PRAGMA_EXPORT bool convert_smd(NetworkState &nw,Model &mdl,VFilePtr &smdFile,const std::string &animName,bool isCollisionMesh,std::vector<std::string> &outTextures)
	{
		auto smd = SMDModel::Load(smdFile);
		if(smd == nullptr)
			return false;
		return load_smd(&nw,animName,mdl,*smd,isCollisionMesh,outTextures);
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
