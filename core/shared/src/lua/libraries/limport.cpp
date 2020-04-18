#include "stdafx_shared.h"
#include "pragma/lua/libraries/limport.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/classes/ldef_skeleton.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/game/scene_snapshot.hpp"
#include "pragma/game/game_resources.hpp"
#include <pragma/util/util_game.hpp>
#include <smdmodel.h>
#include <unordered_set>
#include <sharedutils/util_file.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>

extern DLLENGINE Engine *engine;


int Lua::import::import_wad(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);
	auto &skeleton = *Lua::CheckSkeleton(l,2);
	std::array<uint8_t,3> header;
	f.Read(header.data(),header.size() *sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'A' || header.at(2) != 'D')
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto anim = Animation::Create();
	auto version = f.Read<uint16_t>();
	auto flags = f.Read<uint32_t>();
	auto numBones = f.Read<uint32_t>();
	std::vector<uint32_t> boneList;
	boneList.reserve(numBones);
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto boneName = f.ReadString();
		auto boneId = skeleton.LookupBone(boneName);
		boneList.push_back(boneId);
	}
	anim->SetBoneList(boneList);
	auto numFrames = f.Read<uint32_t>();
	for(auto i=decltype(numFrames){0};i<numFrames;++i)
	{
		auto frame = Frame::Create(numBones);
		for(auto j=decltype(numBones){0};j<numBones;++j)
		{
			auto rot = f.Read<Quat>();
			auto pos = f.Read<Vector3>();
			frame->SetBonePosition(j,pos);
			frame->SetBoneOrientation(j,rot);
		}
		anim->AddFrame(frame);
	}
	anim->Localize(skeleton);
	Lua::Push<std::shared_ptr<Animation>>(l,anim);
	return 1;
}
int Lua::import::import_wrci(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);
	auto &mdl = Lua::Check<Model>(l,2);
	std::array<uint8_t,4> header;
	f.Read(header.data(),header.size() *sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'C' || header.at(3) != 'I')
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto version = f.Read<uint16_t>();
	auto numBones = f.Read<uint32_t>();
	std::vector<std::string> boneNames;
	boneNames.reserve(numBones);
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto name = f.ReadString();
		boneNames.push_back(name);
	}
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto pos = f.Read<Vector3>();
		auto rot = f.Read<Quat>();

	}
	auto *game = engine->GetNetworkState(l)->GetGameState();
	auto numMeshes = f.Read<uint8_t>();
	for(auto i=decltype(numMeshes){0};i<numMeshes;++i)
	{
		auto colMesh = CollisionMesh::Create(game);
		auto parentId = f.Read<int32_t>();
		auto numVerts = f.Read<uint64_t>();
		if(parentId != -1)
		{
			auto &boneName = boneNames.at(parentId);
			colMesh->SetBoneParent(mdl.LookupBone(boneName));
		}
		auto &verts = colMesh->GetVertices();
		verts.reserve(numVerts);
		for(auto j=decltype(numVerts){0};j<numVerts;++j)
		{
			auto v = f.Read<Vector3>();
			verts.push_back(v);
		}
		auto &tris = colMesh->GetTriangles();
		auto numFaces = f.Read<uint64_t>();
		tris.reserve(numFaces *3u);
		for(auto j=decltype(numFaces){0};j<numFaces;++j)
		{
			auto idx0 = f.Read<uint64_t>();
			auto idx1 = f.Read<uint64_t>();
			auto idx2 = f.Read<uint64_t>();
			if(version < 0x0002)
			{
				// Version 1 has incorrect indices (offset by 1), we need to correct them here
				--idx0;
				--idx1;
				--idx2;
			}
			tris.push_back(idx0);
			tris.push_back(idx1);
			tris.push_back(idx2);
		}
		mdl.AddCollisionMesh(colMesh);
		colMesh->Update(ModelUpdateFlags::All);
	}
	mdl.Update();
	Lua::PushBool(l,true);
	return 1;
}
int Lua::import::import_wrmi(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);
	auto &mdl = Lua::Check<Model>(l,2);
	std::array<uint8_t,4> header;
	f.Read(header.data(),header.size() *sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'M' || header.at(3) != 'I')
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto meshGroup = mdl.GetMeshGroup(0u);
	if(meshGroup == nullptr) // TODO
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto version = f.Read<uint16_t>();
	auto offData = f.Read<uint64_t>();
	auto offBones = f.Read<uint64_t>();
	auto offMeshes = f.Read<uint64_t>();
	auto numBones = f.Read<uint32_t>();

	auto &skeleton = mdl.GetSkeleton();
	skeleton.GetBones().clear();
	skeleton.GetRootBones().clear();
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto name = f.ReadString();
		auto *bone = new Bone();
		bone->name = name;
		skeleton.AddBone(bone);
	}
	auto &reference = mdl.GetReference(); // TODO: Update skeleton stuff if already exists
	reference.SetBoneCount(numBones);
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto rot = f.Read<Quat>();
		auto pos = f.Read<Vector3>();
		reference.SetBonePosition(i,pos);
		reference.SetBoneOrientation(i,rot);
	}

	std::function<void(std::shared_ptr<Bone>&)> fReadChildBones = nullptr;
	fReadChildBones = [&f,&fReadChildBones,&skeleton](std::shared_ptr<Bone> &parent) {
		auto numChildren = f.Read<uint32_t>();
		for(auto i=decltype(numChildren){0};i<numChildren;++i)
		{
			auto boneId = f.Read<uint32_t>();
			auto child = skeleton.GetBone(boneId).lock();
			parent->children.insert(std::make_pair(boneId,child));
			child->parent = parent;
			fReadChildBones(child);
		}
	};
	auto &rootBones = skeleton.GetRootBones();
	auto numRoot = f.Read<uint32_t>();
	for(auto i=decltype(numRoot){0};i<numRoot;++i)
	{
		auto boneId = f.Read<uint32_t>();
		auto bone = skeleton.GetBone(boneId).lock();
		rootBones.insert(std::make_pair(boneId,bone));
		fReadChildBones(bone);
	}
	auto *nw = engine->GetNetworkState(l);
	auto mesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
	meshGroup->AddMesh(mesh);
	auto numMeshes = f.Read<uint32_t>();
	auto &meta = mdl.GetMetaInfo();
	auto &texGroup = *mdl.GetTextureGroup(0); // TODO
	for(auto i=decltype(numMeshes){0};i<numMeshes;++i)
	{
		auto parentId = f.Read<int32_t>();
		auto numVerts = f.Read<uint64_t>();
		std::vector<std::pair<Vector3,Vector3>> verts(numVerts);
		for(auto j=decltype(numVerts){0};j<numVerts;++j)
		{
			auto &v = verts.at(j);
			v.first = f.Read<Vector3>();
			v.second = f.Read<Vector3>();
		}
		auto numBonesMesh = f.Read<uint32_t>();
		std::vector<VertexWeight> boneWeights(numVerts);
		for(auto j=decltype(numBonesMesh){0};j<numBonesMesh;++j)
		{

			// TODO
			auto boneId = f.Read<uint32_t>();
			//auto bValidBone = false;//(boneId < model.bones.size()) ? true : false;
			auto numBoneVerts = f.Read<uint64_t>();
			/*if(bValidBone == false)
			{
				f.Seek(f.Tell() +(sizeof(uint64_t) +sizeof(float)) *numBoneVerts);
				continue;
			}*/
			for(auto k=decltype(numBoneVerts){0};k<numBoneVerts;++k)
			{
				auto vertexId = f.Read<uint64_t>();
				auto weight = f.Read<Float>();

				auto &vw = boneWeights.at(vertexId);
				for(auto i=0u;i<4u;++i)
				{
					if(vw.boneIds[i] != -1)
						continue;
					vw.boneIds[i] = boneId;
					vw.weights[i] = weight;
					break;
				}
			}
		}

		auto numMaps = f.Read<uint32_t>();
		for(auto j=decltype(numMaps){0};j<numMaps;++j)
		{
			auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
			auto &meshVerts = subMesh->GetVertices();
			auto &meshTriangles = subMesh->GetTriangles();
			auto &meshWeights = subMesh->GetVertexWeights();
			auto map = f.ReadString();
			map += ".wmi";
			ufile::remove_extension_from_filename(map);

			auto *mat = nw->LoadMaterial(map);
			mdl.AddMaterial(j,mat);

			//meta.textures.push_back(map); // TODO: Only if not exists yet
			//texGroup.textures.push_back(meta.textures.size() -1u);
			subMesh->SetSkinTextureIndex(j);

			auto numFaces = f.Read<uint32_t>();
			meshTriangles.reserve(numFaces *3u);
			meshVerts.reserve(numFaces *3u);
			meshWeights.reserve(numFaces *3u);
			for(auto k=decltype(numFaces){0};k<numFaces;++k)
			{
				auto triIdx = k *3u;
				for(auto l=0u;l<3u;++l)
				{
					auto vertId = f.Read<uint64_t>();
					meshTriangles.push_back(meshVerts.size());
					auto u = f.Read<float>();
					auto v = 1.f -f.Read<float>();

					auto &vertData = verts.at(vertId);
					meshVerts.push_back({});
					auto &vert = meshVerts.back();
					vert.position = vertData.first;
					vert.normal = vertData.second;
					vert.uv = {u,v};

					meshWeights.push_back(boneWeights.at(vertId));
				}
			}
			mesh->AddSubMesh(subMesh);
		}
	}
	mdl.Update(ModelUpdateFlags::All);
	mdl.GenerateBindPoseMatrices();
	Lua::PushBool(l,true);
	return 1;
}

int Lua::import::import_smd(lua_State *l)
{
	auto &nw = *engine->GetNetworkState(l);
	std::string smdFileName = Lua::CheckString(l,1);
	/*if(Lua::file::validate_write_operation(l,smdFileName) == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}*/
	auto f = FileManager::OpenFile(smdFileName.c_str(),"rb");
	if(f == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto &mdl = Lua::Check<::Model>(l,2);
	std::string animName = Lua::CheckString(l,3);
	auto isCollisionMesh = false;
	if(Lua::IsSet(l,4))
		isCollisionMesh = Lua::CheckBool(l,4);

	std::vector<std::string> textures;
	auto success = ::util::port_hl2_smd(nw,mdl,f,animName,isCollisionMesh,textures);
	Lua::PushBool(l,success);
	if(success)
	{
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(textures.size()){0u};i<textures.size();++i)
		{
			Lua::PushInt(l,i +1);
			Lua::PushString(l,textures.at(i));
			Lua::SetTableValue(l,t);
		}
		return 2;
	}
	return 1;
	//bool util::port_hl2_smd(NetworkState &nw,Model &mdl,VFilePtr &f,const std::string &animName,bool isCollisionMesh,std::vector<std::string> &outTextures)
	/*auto &f = *Lua::CheckFile(l,1);
	auto &skeleton = *Lua::CheckSkeleton(l,2);
	auto smd = SMDModel::Load(f.GetHandle());
	if(smd == nullptr)
		return 0;
	auto group = std::make_unique<MeshGroup>(model,meshName);
	auto &nodes = smd->GetNodes();
	auto &frames = smd->GetFrames();
	std::unordered_map<int,Bone*> bones; // SMD Bone ID -> Actual Bone
	if(!frames.empty())
	{
		auto &frame = frames.front();
		for(auto it=nodes.begin();it!=nodes.end();++it)
		{
			auto &node = *it;
			Bone *parent = nullptr;
			if(node.parent != -1)
			{
				auto it = bones.find(node.parent);
				if(it == bones.end())
					continue;
				parent = it->second;
			}
			info.model.bones.push_back(std::make_unique<Bone>());
			auto &bone = info.model.bones.back();
			bone->name = node.name;
			assert(frame.transforms.size() >= (node.id +1));
			auto &t = frame.transforms[node.id];
			bone->position = t.position;
			bone->orientation = t.rotation;
			bones[node.id] = bone.get();

			if(parent != nullptr)
				parent->children.push_back(bone->GetHandle());
			else
				info.model.rootbones.push_back(bone->GetHandle());
		}
	}
	auto &meshes = smd->GetMeshes();
	for(auto it=meshes.begin();it!=meshes.end();++it)
	{
		auto &meshData = *it;
		auto mesh = std::make_unique<Mesh>();
		mesh->maps.push_back(Map());
		auto &map = mesh->maps.back();
		map.map = meshData.texture;
		for(auto it=meshData.triangles.begin();it!=meshData.triangles.end();++it)
		{
			auto &tri = *it;
			map.faces.push_back(Face());
			auto &face = map.faces.back();
			for(auto i=0;i<3;i++)
			{
				auto &v = tri.vertices[i];
				auto &fv = face.vertices[i];
				fv.vertexid = add_unique_vertex(*mesh.get(),v.position,v.normal);
				fv.u = v.uv.x;
				fv.v = 1.f -v.uv.y;
				for(auto it=v.weights.begin();it!=v.weights.end();++it)
				{
					auto nodeId = it->first;
					auto weight = it->second;
					//auto &node = nodes[nodeId];
					auto itBone = bones.find(nodeId);
					if(itBone != bones.end())
					{
						auto *meshBone = add_unique_mesh_bone(info,*mesh,itBone->second);
						VertexWeight w;
						w.vertexid = fv.vertexid;
						w.weight = weight;
						meshBone->weights.push_back(w);
					}
				}
			}
		}
		group->meshes.push_back(mesh.release());
	}
	info.model.meshGroups.push_back(group.release());
	if(lod != LOD_NONE)
	{
		auto meshId = static_cast<unsigned int>(info.model.meshGroups.size()) -1;
		AddLODMesh(info.model,lod,meshId,srcMesh);
	}
	if(meshGroupId != nullptr)
		*meshGroupId = info.model.meshGroups.size() -1;
	return true;
	
	//return smd;*/
	//return 0;
}

int Lua::import::import_obj(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);
	return import_model_asset(l);
}

class AssimpPragmaIOStream
	: public Assimp::IOStream
{
public:
	AssimpPragmaIOStream(const VFilePtr &f)
		: m_file{f}
	{}
    virtual size_t Read(void* pvBuffer,size_t pSize,size_t pCount) override {return m_file->Read(pvBuffer,pSize *pCount);}
    virtual size_t Write(const void* pvBuffer,size_t pSize,size_t pCount) override
	{
		if(m_file->GetType() != VFILE_LOCAL)
			return 0;
		return std::static_pointer_cast<VFilePtrInternalReal>(m_file)->Write(pvBuffer,pSize *pCount);
	}

    virtual aiReturn Seek(size_t pOffset,aiOrigin pOrigin) override
	{
		m_file->Seek(pOffset,pOrigin);
		return aiReturn::aiReturn_SUCCESS;
	}

    virtual size_t Tell() const override {return m_file->Tell();}
    virtual size_t FileSize() const override {return m_file->GetSize();}
    virtual void Flush() override {}
private:
	VFilePtr m_file = nullptr;
};

class AssimpPragmaIo
    : public Assimp::IOSystem
{
public:
    AssimpPragmaIo(VFilePtr f)
		: m_rootFile{f}
	{};
    virtual ~AssimpPragmaIo() override=default;
    virtual bool Exists(const char* pFile) const override {return ustring::compare(pFile,"__rootAssetFile",15) || FileManager::ExistsSystem(pFile);}
	virtual char getOsSeparator() const override {return '/';}

    virtual Assimp::IOStream* Open(const char* pFile,const char* pMode = "rb") override
	{
		if(ustring::compare(pFile,"__rootAssetFile",15))
		{
			m_rootFile->Seek(0);
			return new AssimpPragmaIOStream{m_rootFile};
		}
		auto f = FileManager::OpenSystemFile(pFile,pMode);
		if(f == nullptr)
			return nullptr;
		return new AssimpPragmaIOStream{f};
	}

    virtual void Close(Assimp::IOStream* pFile) override {delete pFile;}
private:
	VFilePtr m_rootFile = nullptr;
};

std::shared_ptr<aiScene> Lua::import::snapshot_to_assimp_scene(const pragma::SceneSnapshot &snapshot)
{
	auto &materials = snapshot.GetMaterials();
	std::unordered_map<Material*,uint32_t> materialToIndex {};
	uint32_t matIdx = 0u;
	for(auto &hMat : materials)
		materialToIndex.insert(std::make_pair(hMat.get(),matIdx++));

	auto scene = std::make_shared<aiScene>();
	scene->mRootNode = new aiNode{};
	scene->mRootNode->mNumChildren = 1;
	scene->mRootNode->mChildren = new aiNode*[1];
	auto *node = new aiNode {};
	scene->mRootNode->mChildren[0] = node;
	node->mName = "mesh";

	scene->mMaterials = new aiMaterial*[materials.size()];
	for(auto i=decltype(materials.size()){0u};i<materials.size();++i)
		scene->mMaterials[i] = new aiMaterial{};
	scene->mNumMaterials = materials.size();

	//aiString name {"soldier_d.dds"};
	//scene.mMaterials[0]->AddProperty(&name,AI_MATKEY_NAME);
	
	std::vector<uint32_t> nodeIds {};
	auto &meshes = snapshot.GetMeshes();
	scene->mMeshes = new aiMesh*[meshes.size()];
	scene->mNumMeshes = meshes.size();
	nodeIds.reserve(meshes.size());
	for(auto i=decltype(meshes.size()){0u};i<meshes.size();++i)
	{
		auto &mesh = meshes.at(i);
		scene->mMeshes[i] = new aiMesh{};
		auto &aiMesh = *scene->mMeshes[i];

		auto numVerts = mesh->verts.size();
		aiMesh.mVertices = new aiVector3D[numVerts];
		aiMesh.mNormals = new aiVector3D[numVerts];
		aiMesh.mNumVertices = numVerts;

		aiMesh.mTextureCoords[0] = new aiVector3D[numVerts];
		aiMesh.mNumUVComponents[0] = 2;

		auto &lightmapUvs = mesh->lightmapUvs;
		if(lightmapUvs.empty() == false)
		{
			aiMesh.mTextureCoords[1] = new aiVector3D[numVerts];
			aiMesh.mNumUVComponents[1] = 2;
		}

		auto numTris = mesh->tris.size() /3;
		aiMesh.mFaces = new aiFace[numTris];
		aiMesh.mNumFaces = numTris;
		aiMesh.mName = "mesh";

		auto itMat = materialToIndex.find(mesh->material.get());
		aiMesh.mMaterialIndex = (itMat != materialToIndex.end()) ? itMat->second : 0u;

		for(uint32_t vertIdx=0u;vertIdx<numVerts;++vertIdx)
		{
			auto &v = mesh->verts.at(vertIdx);
			aiMesh.mVertices[vertIdx] = aiVector3D{v.position.x,v.position.y,v.position.z} *static_cast<float>(util::units_to_metres(1.0));
			aiMesh.mNormals[vertIdx] = aiVector3D{v.normal.x,v.normal.y,v.normal.z};
			aiMesh.mTextureCoords[0][vertIdx] = aiVector3D{v.uv.x,1.f -v.uv.y,0.f};
			if(lightmapUvs.empty() == false)
			{
				auto &uv = lightmapUvs.at(vertIdx);
				aiMesh.mTextureCoords[1][vertIdx] = aiVector3D{uv.x,1.f -uv.y,0.f};
			}
		}

		for(uint32_t triIdx=0u;triIdx<numTris;++triIdx)
		{
			auto &face = aiMesh.mFaces[triIdx];
			face.mIndices = new unsigned int[3];
			face.mNumIndices = 3;

			for(uint8_t i=0;i<3;++i)
				face.mIndices[i] = mesh->tris.at(triIdx *3 +i);
		}
		nodeIds.push_back(i);
	}
	node->mNumMeshes = nodeIds.size();
	node->mMeshes = new uint32_t[nodeIds.size()];
	for(auto i=decltype(nodeIds.size()){0u};i<nodeIds.size();++i)
		node->mMeshes[i] = nodeIds.at(i);
	return scene;
}

int Lua::import::export_model_asset(lua_State *l)
{
	auto &mdl = Lua::Check<Model>(l,1);

	Assimp::Exporter exporter;
	auto sceneSnapshot = pragma::SceneSnapshot::Create();
	sceneSnapshot->AddModel(mdl);

	auto aiScene = snapshot_to_assimp_scene(*sceneSnapshot);
	auto result = exporter.Export(aiScene.get(),"fbx","E:/projects/pragma/build_winx64/output/box.fbx");
	Lua::PushBool(l,result == aiReturn::aiReturn_SUCCESS);
	
	auto *error = exporter.GetErrorString();
	Con::cwar<<"WARNING: Export error: '"<<error<<"'!"<<Con::endl;
	return 1;
}

int Lua::import::import_model_asset(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);
	auto hFile = f.GetHandle();
	if(hFile->GetType() != VFILE_LOCAL)
		return 0;
	auto hFileLocal = std::static_pointer_cast<VFilePtrInternalReal>(hFile);

	Assimp::Importer importer;
	importer.SetIOHandler(new AssimpPragmaIo{hFile});
	auto *aiScene = importer.ReadFile(
		hFileLocal->GetPath(),
		aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FindDegenerates | 
		aiProcess_FindInvalidData | aiProcess_LimitBoneWeights | aiProcess_GenSmoothNormals | aiProcess_EmbedTextures
	);
	if(aiScene == nullptr)
	{
		auto *error = importer.GetErrorString();
		Con::cwar<<"WARNING: Unable to import model asset: '"<<error<<"'!"<<Con::endl;
		return 0;
	}

	auto *nw = engine->GetNetworkState(l);
	auto fConvertMesh = [nw](aiMesh &mesh) {
		auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
		auto &verts = subMesh->GetVertices();
		verts.reserve(mesh.mNumVertices);
		for(auto j=decltype(mesh.mNumVertices){0};j<mesh.mNumVertices;++j)
		{
			auto &aiVert = mesh.mVertices[j];

			verts.push_back({});
			auto &v = verts.back();
			v.position = {aiVert.x,aiVert.y,aiVert.z};
			if(mesh.mTextureCoords && mesh.mTextureCoords[0])
			{
				auto &aiUvw = mesh.mTextureCoords[0][j];
				v.uv = {aiUvw.x,-aiUvw.y};
			}
			if(mesh.mNormals)
			{
				auto &aiNormal = mesh.mNormals[j];
				v.normal = {aiNormal.x,aiNormal.y,aiNormal.z};
			}
		}

		auto &triangles = subMesh->GetTriangles();
		triangles.reserve(mesh.mNumFaces *3);
		for(auto j=decltype(mesh.mNumFaces){0};j<mesh.mNumFaces;++j)
		{
			auto &face = mesh.mFaces[j];
			//face.mNumIndices // TODO
			triangles.push_back(face.mIndices[0]);
			triangles.push_back(face.mIndices[1]);
			triangles.push_back(face.mIndices[2]);
		}
		subMesh->SetSkinTextureIndex(mesh.mMaterialIndex);
		return subMesh;
	};

	std::vector<std::shared_ptr<ModelSubMesh>> subMeshes {};
	std::function<void(aiNode&,const pragma::physics::ScaledTransform&)> fIterateTree = nullptr;
	fIterateTree = [aiScene,&fIterateTree,&fConvertMesh,&subMeshes](aiNode &node,const pragma::physics::ScaledTransform &parentPose) {
		aiVector3D scale;
		aiQuaternion rot;
		aiVector3D pos;
		auto t = node.mTransformation;
		// t.Inverse();
		t.Decompose(scale,rot,pos);

		pragma::physics::ScaledTransform pose {Vector3{pos.x,pos.y,pos.z},Quat{rot.w,rot.x,rot.y,rot.z},Vector3{scale.x,scale.y,scale.z}};
		auto ang = EulerAngles{pose.GetRotation()};
		//umath::swap(ang.r,ang.y);
		//pose.SetRotation(uquat::identity());//uquat::create(ang));
		pose = parentPose *pose;

		auto meshScale = pose.GetScale();
		auto invPose = pose;//pose.GetInverse();
		//invPose.SetRotation(uquat::create(EulerAngles(-90,26.3579,0)));
		//invPose.RotateLocal(uquat::create(EulerAngles{0.f,180.f,0.f}));
		invPose.SetScale(Vector3{1.f,1.f,1.f});

		ang = EulerAngles{invPose.GetRotation()};

		for(auto i=decltype(node.mNumMeshes){0u};i<node.mNumMeshes;++i)
		{
			auto meshIdx = node.mMeshes[i];
			auto subMesh = fConvertMesh(*aiScene->mMeshes[meshIdx]);
			subMesh->Scale(meshScale);
			subMesh->Transform(invPose);
			subMeshes.push_back(subMesh);
		}

		for(auto i=decltype(node.mNumChildren){0u};i<node.mNumChildren;++i)
			fIterateTree(*node.mChildren[i],pose);
	};
	pragma::physics::ScaledTransform pose {};
	fIterateTree(*aiScene->mRootNode,pose);

	auto t = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &subMesh : subMeshes)
	{
		Lua::PushInt(l,idx++);
		Lua::Push<std::shared_ptr<ModelSubMesh>>(l,subMesh);
		Lua::SetTableValue(l,t);
	}
	std::vector<std::string> importedTextures {};
	auto rfile = std::static_pointer_cast<VFilePtrInternalReal>(f.GetHandle());
	auto texturesImported = false;
	std::optional<std::string> filePath = rfile ? rfile->GetPath() : std::optional<std::string>{};
	if(filePath.has_value())
	{
		auto modelName = ufile::get_file_from_filename(*filePath);
		ufile::remove_extension_from_filename(modelName);

#if 0
		auto matPath = "models/" +FileManager::GetCanonicalizedPath(modelName);
		auto dstMatPath = "addons/imported/materials/" +matPath;
		if(FileManager::CreatePath(dstMatPath.c_str()) == false)
			Con::cwar<<"WARNING: Unable to create material output path '"<<dstMatPath<<"'! Textures will not be imported."<<Con::endl;
		else
		{
			texturesImported = true;
			auto srcMatPath = ufile::get_path_from_filename(*filePath);
			importedTextures.reserve(aiScene->mNumMaterials);
			for(auto i=decltype(aiScene->mNumMaterials){0u};i<aiScene->mNumMaterials;++i)
			{
				auto *mat = aiScene->mMaterials[i];
				std::string matName = mat->GetName().C_Str();

				auto dstPath = dstMatPath +'/' +matName;
				importedTextures.push_back(dstPath);

				auto matPath = srcMatPath +matName;
				std::vector<std::string> files {};
				FileManager::FindSystemFiles((matPath +".*").c_str(),&files,nullptr);
				auto &suportedFormats = MaterialManager::get_supported_image_formats();
				auto foundSupportedImageFile = false;
				for(auto &fName : files)
				{
					std::string ext = "";
					ufile::get_extension(fName,&ext);
					auto it = std::find_if(suportedFormats.begin(),suportedFormats.end(),[&ext](const MaterialManager::ImageFormat &format) {
						return ustring::compare(format.extension,ext,false);
					});
					if(it != suportedFormats.end())
					{
						if(FileManager::Exists(dstPath))
							Con::cout<<"Texture '"<<dstPath<<"' already exists and will not be imported!"<<Con::endl;
						else
						{
							if(FileManager::CopySystemFile((srcMatPath +fName).c_str(),(FileManager::GetProgramPath() +'/' +dstMatPath +'/' +fName).c_str()) == false)
								Con::cwar<<"WARNING: Unable to copy texture file '"<<fName<<"'! Texture will not be imported."<<Con::endl;
							else
								Con::cout<<"Imported texture '"<<dstPath<<"'!"<<Con::endl;
						}
						foundSupportedImageFile = true;
						break;
					}
				}
				if(foundSupportedImageFile == false)
				{
					Con::cwar<<"WARNING: Could not find texture file '"<<matName<<"' with supported format! Texture will not be imported."<<Con::endl;
					continue;
				}
			}
		}
#endif
	}
	std::unordered_map<uint32_t,uint32_t> originalMaterialIndexToModelMaterialIndex {};
	if(texturesImported == false)
	{
		auto srcMatPath = ufile::get_path_from_filename(*filePath);
		importedTextures.reserve(aiScene->mNumMaterials);
		for(auto i=decltype(aiScene->mNumMaterials){0u};i<aiScene->mNumMaterials;++i)
		{
			auto *mat = aiScene->mMaterials[i];
			std::string matName = mat->GetName().C_Str();
			auto numDiffuseMaps = mat->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE);
			/*for(auto j=decltype(mat->mNumProperties){0};j<mat->mNumProperties;++j)
			{
				auto *prop = mat->mProperties[j];
				std::cout<<"Semantic: "<<prop->mSemantic<<std::endl;
			}*/
			if(numDiffuseMaps > 0)
			{
				aiString path;
				if(mat->GetTexture(aiTextureType::aiTextureType_DIFFUSE,0u,&path) == aiReturn::aiReturn_SUCCESS && path.length > 3)
				{
					std::string npath = path.data;
					if(npath.empty() == false && npath.front() == '*')
						npath = path.data +npath.length() +1;
					auto br = umath::min(npath.rfind('/'),npath.rfind('\\'));
					if(br != std::string::npos)
						npath = npath.substr(br +1);
					ufile::remove_extension_from_filename(npath);
					importedTextures.push_back(npath);
					originalMaterialIndexToModelMaterialIndex.insert(std::make_pair(i,importedTextures.size() -1));
				}
			}
		}
	}

	// Update material indices
	for(auto &subMesh : subMeshes)
	{
		auto matIdx = subMesh->GetSkinTextureIndex();
		auto it = originalMaterialIndexToModelMaterialIndex.find(matIdx);
		if(it != originalMaterialIndexToModelMaterialIndex.end())
			subMesh->SetSkinTextureIndex(it->second);
		else
			subMesh->SetSkinTextureIndex(std::numeric_limits<uint32_t>::max());
	}

	auto tTextures = Lua::CreateTable(l);
	for(auto i=decltype(importedTextures.size()){0};i<importedTextures.size();++i)
	{
		auto &tex = importedTextures.at(i);
		Lua::PushInt(l,i +1);
		Lua::PushString(l,tex);
		Lua::SetTableValue(l,tTextures);
	}

	// Build skeleton
	auto skeleton = std::make_shared<Skeleton>();
	auto referencePose = Frame::Create(1);
	std::function<std::shared_ptr<Bone>(aiNode&,Bone*)> fIterateBones = nullptr;
	fIterateBones = [&fIterateBones,&skeleton,&referencePose](aiNode &node,Bone *parent) -> std::shared_ptr<Bone> {
		auto *bone = new Bone{};
		bone->name = node.mName.C_Str();
		auto boneIdx = skeleton->AddBone(bone);
		if(parent)
			parent->children.insert(std::make_pair(boneIdx,skeleton->GetBone(boneIdx).lock()));

		aiVector3D scale;
		aiQuaternion rot;
		aiVector3D pos;
		auto t = node.mTransformation;
		// t.Inverse();
		t.Decompose(scale,rot,pos);
		referencePose->SetBonePosition(boneIdx,Vector3{pos.x,pos.y,pos.z});
		referencePose->SetBoneOrientation(boneIdx,Quat{rot.w,rot.x,rot.y,rot.z});
		if(scale.x != 1.f || scale.y != 1.f || scale.z != 1.f)
			referencePose->SetBoneScale(boneIdx,Vector3{scale.x,scale.y,scale.z});

		auto numBones = skeleton->GetBones().size() +node.mNumChildren;
		skeleton->GetBones().reserve(numBones);
		referencePose->SetBoneCount(numBones);
		for(auto i=decltype(node.mNumChildren){0};i<node.mNumChildren;++i)
		{
			auto &child = *node.mChildren[i];
			fIterateBones(child,bone);
		}
		return skeleton->GetBone(boneIdx).lock();
	};
	if(aiScene->mRootNode)
	{
		auto rootBone = fIterateBones(*aiScene->mRootNode,nullptr);
		skeleton->GetRootBones().insert(std::make_pair(rootBone->ID,rootBone));
	}
	// http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
	/*aiScene->mRootNode;
	for(auto i=decltype(aiScene->mNumAnimations){0};i<aiScene->mNumAnimations;++i)
	{
		//aiScene->
		auto *anim = aiScene->mAnimations[i];
		anim->mDuration;
		anim->mTicksPerSecond;
		anim->mChannels;
		anim->
	}*/

	Lua::Push<std::shared_ptr<Skeleton>>(l,skeleton);
	Lua::Push<std::shared_ptr<Frame>>(l,referencePose);

	return 4;
}

