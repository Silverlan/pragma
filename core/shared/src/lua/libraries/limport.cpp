/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/limport.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/classes/ldef_skeleton.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/game/scene_snapshot.hpp"
#include "pragma/game/game_resources.hpp"
#include "pragma/game/game_limits.h"
#include <pragma/util/util_game.hpp>
#include <unordered_set>
#include <sharedutils/util_file.h>
#if 0
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/DefaultLogger.hpp>
#endif
#include "pragma/model/animation/skeleton.hpp"
#include "pragma/model/animation/bone.hpp"

extern DLLNETWORK Engine *engine;

int Lua::import::import_wad(lua_State *l)
{
	auto &f = *Lua::CheckFile(l, 1);
	auto &skeleton = *Lua::CheckSkeleton(l, 2);
	std::array<uint8_t, 3> header;
	f.Read(header.data(), header.size() * sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'A' || header.at(2) != 'D') {
		Lua::PushBool(l, false);
		return 1;
	}
	auto anim = pragma::animation::Animation::Create();
	auto version = f.Read<uint16_t>();
	auto flags = f.Read<uint32_t>();
	auto numBones = f.Read<uint32_t>();
	std::vector<uint16_t> boneList;
	boneList.reserve(numBones);
	for(auto i = decltype(numBones) {0}; i < numBones; ++i) {
		auto boneName = f.ReadString();
		auto boneId = skeleton.LookupBone(boneName);
		boneList.push_back(boneId);
	}
	anim->SetBoneList(boneList);
	auto numFrames = f.Read<uint32_t>();
	for(auto i = decltype(numFrames) {0}; i < numFrames; ++i) {
		auto frame = Frame::Create(numBones);
		for(auto j = decltype(numBones) {0}; j < numBones; ++j) {
			auto rot = f.Read<Quat>();
			auto pos = f.Read<Vector3>();
			frame->SetBonePosition(j, pos);
			frame->SetBoneOrientation(j, rot);
		}
		anim->AddFrame(frame);
	}
	anim->Localize(skeleton);
	Lua::Push<std::shared_ptr<pragma::animation::Animation>>(l, anim);
	return 1;
}
int Lua::import::import_wrci(lua_State *l)
{
	auto &f = *Lua::CheckFile(l, 1);
	auto &mdl = Lua::Check<Model>(l, 2);
	std::array<uint8_t, 4> header;
	f.Read(header.data(), header.size() * sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'C' || header.at(3) != 'I') {
		Lua::PushBool(l, false);
		return 1;
	}
	auto version = f.Read<uint16_t>();
	auto numBones = f.Read<uint32_t>();
	std::vector<std::string> boneNames;
	boneNames.reserve(numBones);
	for(auto i = decltype(numBones) {0}; i < numBones; ++i) {
		auto name = f.ReadString();
		boneNames.push_back(name);
	}
	for(auto i = decltype(numBones) {0}; i < numBones; ++i) {
		auto pos = f.Read<Vector3>();
		auto rot = f.Read<Quat>();
	}
	auto *game = engine->GetNetworkState(l)->GetGameState();
	auto numMeshes = f.Read<uint8_t>();
	for(auto i = decltype(numMeshes) {0}; i < numMeshes; ++i) {
		auto colMesh = CollisionMesh::Create(game);
		auto parentId = f.Read<int32_t>();
		auto numVerts = f.Read<uint64_t>();
		if(parentId != -1) {
			auto &boneName = boneNames.at(parentId);
			colMesh->SetBoneParent(mdl.LookupBone(boneName));
		}
		auto &verts = colMesh->GetVertices();
		verts.reserve(numVerts);
		for(auto j = decltype(numVerts) {0}; j < numVerts; ++j) {
			auto v = f.Read<Vector3>();
			verts.push_back(v);
		}
		auto &tris = colMesh->GetTriangles();
		auto numFaces = f.Read<uint64_t>();
		tris.reserve(numFaces * 3u);
		for(auto j = decltype(numFaces) {0}; j < numFaces; ++j) {
			auto idx0 = f.Read<uint64_t>();
			auto idx1 = f.Read<uint64_t>();
			auto idx2 = f.Read<uint64_t>();
			if(version < 0x0002) {
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
	Lua::PushBool(l, true);
	return 1;
}
int Lua::import::import_wrmi(lua_State *l)
{
	auto &f = *Lua::CheckFile(l, 1);
	auto &mdl = Lua::Check<Model>(l, 2);
	std::array<uint8_t, 4> header;
	f.Read(header.data(), header.size() * sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'M' || header.at(3) != 'I') {
		Lua::PushBool(l, false);
		return 1;
	}
	auto meshGroup = mdl.GetMeshGroup(0u);
	if(meshGroup == nullptr) // TODO
	{
		Lua::PushBool(l, false);
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
	for(auto i = decltype(numBones) {0}; i < numBones; ++i) {
		auto name = f.ReadString();
		auto *bone = new pragma::animation::Bone();
		bone->name = name;
		skeleton.AddBone(bone);
	}
	auto &reference = mdl.GetReference(); // TODO: Update skeleton stuff if already exists
	reference.SetBoneCount(numBones);
	for(auto i = decltype(numBones) {0}; i < numBones; ++i) {
		auto rot = f.Read<Quat>();
		auto pos = f.Read<Vector3>();
		reference.SetBonePosition(i, pos);
		reference.SetBoneOrientation(i, rot);
	}

	std::function<void(std::shared_ptr<pragma::animation::Bone> &)> fReadChildBones = nullptr;
	fReadChildBones = [&f, &fReadChildBones, &skeleton](std::shared_ptr<pragma::animation::Bone> &parent) {
		auto numChildren = f.Read<uint32_t>();
		for(auto i = decltype(numChildren) {0}; i < numChildren; ++i) {
			auto boneId = f.Read<uint32_t>();
			auto child = skeleton.GetBone(boneId).lock();
			parent->children.insert(std::make_pair(boneId, child));
			child->parent = parent;
			fReadChildBones(child);
		}
	};
	auto &rootBones = skeleton.GetRootBones();
	auto numRoot = f.Read<uint32_t>();
	for(auto i = decltype(numRoot) {0}; i < numRoot; ++i) {
		auto boneId = f.Read<uint32_t>();
		auto bone = skeleton.GetBone(boneId).lock();
		rootBones.insert(std::make_pair(boneId, bone));
		fReadChildBones(bone);
	}
	auto *nw = engine->GetNetworkState(l);
	auto mesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
	meshGroup->AddMesh(mesh);
	auto numMeshes = f.Read<uint32_t>();
	auto &meta = mdl.GetMetaInfo();
	auto &texGroup = *mdl.GetTextureGroup(0); // TODO
	for(auto i = decltype(numMeshes) {0}; i < numMeshes; ++i) {
		auto parentId = f.Read<int32_t>();
		auto numVerts = f.Read<uint64_t>();
		std::vector<std::pair<Vector3, Vector3>> verts(numVerts);
		for(auto j = decltype(numVerts) {0}; j < numVerts; ++j) {
			auto &v = verts.at(j);
			v.first = f.Read<Vector3>();
			v.second = f.Read<Vector3>();
		}
		auto numBonesMesh = f.Read<uint32_t>();
		std::vector<umath::VertexWeight> boneWeights(numVerts);
		for(auto j = decltype(numBonesMesh) {0}; j < numBonesMesh; ++j) {

			// TODO
			auto boneId = f.Read<uint32_t>();
			//auto bValidBone = false;//(boneId < model.bones.size()) ? true : false;
			auto numBoneVerts = f.Read<uint64_t>();
			/*if(bValidBone == false)
			{
				f.Seek(f.Tell() +(sizeof(uint64_t) +sizeof(float)) *numBoneVerts);
				continue;
			}*/
			for(auto k = decltype(numBoneVerts) {0}; k < numBoneVerts; ++k) {
				auto vertexId = f.Read<uint64_t>();
				auto weight = f.Read<Float>();

				auto &vw = boneWeights.at(vertexId);
				for(auto i = 0u; i < 4u; ++i) {
					if(vw.boneIds[i] != -1)
						continue;
					vw.boneIds[i] = boneId;
					vw.weights[i] = weight;
					break;
				}
			}
		}

		auto numMaps = f.Read<uint32_t>();
		for(auto j = decltype(numMaps) {0}; j < numMaps; ++j) {
			auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
			auto &meshVerts = subMesh->GetVertices();
			auto &meshWeights = subMesh->GetVertexWeights();
			auto map = f.ReadString();
			map += ".wmi";
			ufile::remove_extension_from_filename(map);

			auto *mat = nw->LoadMaterial(map);
			mdl.AddMaterial(j, mat);

			//meta.textures.push_back(map); // TODO: Only if not exists yet
			//texGroup.textures.push_back(meta.textures.size() -1u);
			subMesh->SetSkinTextureIndex(j);

			auto numFaces = f.Read<uint32_t>();
			subMesh->ReserveIndices(numFaces * 3u);
			meshVerts.reserve(numFaces * 3u);
			meshWeights.reserve(numFaces * 3u);
			for(auto k = decltype(numFaces) {0}; k < numFaces; ++k) {
				auto triIdx = k * 3u;
				for(auto l = 0u; l < 3u; ++l) {
					auto vertId = f.Read<uint64_t>();
					subMesh->AddIndex(meshVerts.size());
					auto u = f.Read<float>();
					auto v = 1.f - f.Read<float>();

					auto &vertData = verts.at(vertId);
					meshVerts.push_back({});
					auto &vert = meshVerts.back();
					vert.position = vertData.first;
					vert.normal = vertData.second;
					vert.uv = {u, v};

					meshWeights.push_back(boneWeights.at(vertId));
				}
			}
			mesh->AddSubMesh(subMesh);
		}
	}
	mdl.Update(ModelUpdateFlags::All);
	mdl.GenerateBindPoseMatrices();
	Lua::PushBool(l, true);
	return 1;
}

int Lua::import::import_smd(lua_State *l)
{
	auto &nw = *engine->GetNetworkState(l);
	std::string smdFileName = Lua::CheckString(l, 1);
	/*if(Lua::file::validate_write_operation(l,smdFileName) == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}*/
	auto f = FileManager::OpenFile(smdFileName.c_str(), "rb");
	if(f == nullptr) {
		Lua::PushBool(l, false);
		return 1;
	}
	auto &mdl = Lua::Check<::Model>(l, 2);
	std::string animName = Lua::CheckString(l, 3);
	auto isCollisionMesh = false;
	if(Lua::IsSet(l, 4))
		isCollisionMesh = Lua::CheckBool(l, 4);

	std::vector<std::string> textures;
	auto success = ::util::port_hl2_smd(nw, mdl, f, animName, isCollisionMesh, textures);
	Lua::PushBool(l, success);
	if(success) {
		auto t = Lua::CreateTable(l);
		for(auto i = decltype(textures.size()) {0u}; i < textures.size(); ++i) {
			Lua::PushInt(l, i + 1);
			Lua::PushString(l, textures.at(i));
			Lua::SetTableValue(l, t);
		}
		return 2;
	}
	return 1;
}

int Lua::import::import_obj(lua_State *l)
{
	auto &f = *Lua::CheckFile(l, 1);
	return import_model_asset(l);
}
#if 0
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
#endif
std::shared_ptr<aiScene> Lua::import::snapshot_to_assimp_scene(const pragma::SceneSnapshot &snapshot)
{
#if 0
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
			aiMesh.mVertices[vertIdx] = aiVector3D{v.position.x,v.position.y,v.position.z} *static_cast<float>(pragma::units_to_metres(1.0));
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
#endif
	return nullptr;
}

int Lua::import::export_model_asset(lua_State *l)
{
#if 0
	auto &mdl = Lua::Check<Model>(l,1);

	Assimp::Exporter exporter;
	auto sceneSnapshot = pragma::SceneSnapshot::Create();
	sceneSnapshot->AddModel(mdl);

	auto aiScene = snapshot_to_assimp_scene(*sceneSnapshot);
	auto result = exporter.Export(aiScene.get(),"fbx","E:/projects/pragma/build_winx64/output/box.fbx");
	Lua::PushBool(l,result == aiReturn::aiReturn_SUCCESS);
	
	auto *error = exporter.GetErrorString();
	Con::cwar<<"Export error: '"<<error<<"'!"<<Con::endl;
	return 1;
#endif
	return 0;
}
#if 0
inline static Mat4 Assimp2Glm(const aiMatrix4x4& from) {
    return Mat4(
        (double)from.a1, (double)from.b1, (double)from.c1, (double)from.d1,
        (double)from.a2, (double)from.b2, (double)from.c2, (double)from.d2,
        (double)from.a3, (double)from.b3, (double)from.c3, (double)from.d3,
        (double)from.a4, (double)from.b4, (double)from.c4, (double)from.d4
    );
}

class myStream : public Assimp::LogStream {
public:
    void write(const char* message) {
		Con::cout<<"[AssImp] "<<message<<Con::endl;
    }
};


static bool import_model_asset(NetworkState &nw,VFilePtr hFile,const std::string &outputFilePath,std::string &outErr)
{
	auto *game = nw.GetGameState();
	if(!game)
		return false;
	if(hFile->GetType() != VFILE_LOCAL)
		return false;
	auto hFileLocal = std::static_pointer_cast<VFilePtrInternalReal>(hFile);

	const unsigned int severity = Assimp::Logger::Debugging|Assimp::Logger::Info|Assimp::Logger::Err|Assimp::Logger::Warn;
	// Attaching it to the default logger
	Assimp::DefaultLogger::get()->attachStream( new myStream, severity );

	Assimp::Importer importer;
	importer.SetIOHandler(new AssimpPragmaIo{hFile});
	importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES,umath::to_integral(GameLimits::MaxBones));
	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE,1);
	importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT,umath::to_integral(GameLimits::MaxMeshVertices));
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS,4); // umath::to_integral(GameLimits::MaxVertexWeights));
	importer.SetPropertyInteger(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES,0);
	auto *aiScene = importer.ReadFile(
		hFileLocal->GetPath(),
		aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FindDegenerates | 
		aiProcess_FindInvalidData | aiProcess_LimitBoneWeights | aiProcess_GenSmoothNormals | aiProcess_EmbedTextures | aiProcess_FixInfacingNormals
	);
	if(aiScene == nullptr)
	{
		auto *error = importer.GetErrorString();
		outErr = error;
		return false;
	}

	auto fConvertMesh = [&nw,&outErr](aiMesh &mesh) -> std::shared_ptr<ModelSubMesh> {
		auto subMesh = std::shared_ptr<ModelSubMesh>(nw.CreateSubMesh());
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
			if(face.mNumIndices != 3)
				continue;
			for(uint8_t k=0;k<3;++k)
			{
				auto idx = face.mIndices[k];
				constexpr auto maxVerts = std::numeric_limits<util::base_type<decltype(triangles[0])>>::max();
				if(idx >= maxVerts)
				{

					outErr = "Mesh has more than " +std::to_string(maxVerts) +" vertices, this is currently not supported!";
					return nullptr;
				}
				triangles.push_back(idx);
			}
		}
		subMesh->SetSkinTextureIndex(mesh.mMaterialIndex);
		return subMesh;
	};

	auto fGetAbsolutePose = [](aiNode &node) {
		auto t = node.mTransformation;
		auto *parent = node.mParent;
		while(parent)
		{
			t = t *parent->mTransformation;
			parent = parent->mParent;
		}
		return t;
	};

	auto mdl = game->CreateModel();
	auto &skeleton = mdl->GetSkeleton();

	std::function<bool(aiNode&)> x = nullptr;
	x = [&x,&aiScene](aiNode &node) -> bool {
		std::cout<<"Node: "<<node.mName.C_Str()<<" ("<<node.mNumMeshes<<")"<<std::endl;
		for(auto i=decltype(node.mNumMeshes){0u};i<node.mNumMeshes;++i)
		{
			auto meshIdx = node.mMeshes[i];
			auto &aiMesh = *aiScene->mMeshes[meshIdx];
			for(auto j=decltype(aiMesh.mNumBones){0u};j<aiMesh.mNumBones;++j)
			{
				auto &aiBone = *aiMesh.mBones[j];
				auto &t = aiBone.mOffsetMatrix;
				std::string boneName = aiBone.mName.C_Str();
				std::cout<<"Bone: "<<boneName<<std::endl;
			}
		}

		for(auto i=decltype(node.mNumChildren){0u};i<node.mNumChildren;++i)
		{
			x(*node.mChildren[i]);
		}
		return true;
	};
	x(*aiScene->mRootNode);
	for(auto i=decltype(aiScene->mNumMeshes){0u};i<aiScene->mNumMeshes;++i)
	{
		auto &aiMesh = aiScene->mMeshes[i];
		if(aiMesh->HasBones())
			std::cout<<"!!"<<std::endl;
	}

	std::vector<std::shared_ptr<ModelSubMesh>> subMeshes {};
	std::function<bool(aiNode&)> fIterateTree = nullptr;
	fIterateTree = [aiScene,&skeleton,&fGetAbsolutePose,&fIterateTree,&fConvertMesh,&subMeshes](aiNode &node) -> bool {
		auto t = fGetAbsolutePose(node);
		//t.Decompose(scale,rot,pos);
		auto m = Assimp2Glm(t);
		Vector3 translation {};
		Quat rotation = uquat::identity();
		Vector3 scale = Vector3{1,1,1};
		//umat::decompose(m,translation,rotation,&scale);

		//umath::ScaledTransform pose {Vector3{pos.x,pos.y,pos.z},Quat{rot.w,rot.x,rot.y,rot.z},Vector3{scale.x,scale.y,scale.z}};
		umath::ScaledTransform pose {translation,rotation,scale};
		auto ang = EulerAngles{pose.GetRotation()};
		//umath::swap(ang.r,ang.y);
		//pose.SetRotation(uquat::identity());//uquat::create(ang));
		//pose = parentPose *pose;

		auto meshScale = pose.GetScale();
		auto invPose = pose;//pose.GetInverse();
		//invPose.SetRotation(uquat::create(EulerAngles(-90,26.3579,0)));
		//invPose.RotateLocal(uquat::create(EulerAngles{0.f,180.f,0.f}));
		invPose.SetScale(Vector3{1.f,1.f,1.f});

		//ang = EulerAngles{invPose.GetRotation()};

		for(auto i=decltype(node.mNumMeshes){0u};i<node.mNumMeshes;++i)
		{
			auto meshIdx = node.mMeshes[i];
			auto &aiMesh = *aiScene->mMeshes[meshIdx];
			auto subMesh = fConvertMesh(aiMesh);
			if(!subMesh)
				return false;
			subMesh->Scale(meshScale);
			subMesh->Transform(invPose);
			subMeshes.push_back(subMesh);

			for(auto j=decltype(aiMesh.mNumBones){0u};j<aiMesh.mNumBones;++j)
			{
				auto &aiBone = *aiMesh.mBones[j];
				auto &t = aiBone.mOffsetMatrix;
				std::string boneName = aiBone.mName.C_Str();
				if(skeleton.LookupBone(boneName) != -1)
					continue;
				auto *bone = new pragma::animation::Bone{};
				bone->name = std::move(boneName);
				skeleton.AddBone(bone);
			}
		}

		for(auto i=decltype(node.mNumChildren){0u};i<node.mNumChildren;++i)
		{
			auto r = fIterateTree(*node.mChildren[i]);
			if(!r)
				return false;
		}
		return true;
	};
	if(!fIterateTree(*aiScene->mRootNode))
		return false;

	std::vector<std::string> importedTextures {};
	auto rfile = std::static_pointer_cast<VFilePtrInternalReal>(hFile);
	auto texturesImported = false;
	std::optional<std::string> filePath = rfile ? rfile->GetPath() : std::optional<std::string>{};
	if(filePath.has_value())
	{
		auto modelName = ufile::get_file_from_filename(*filePath);
		ufile::remove_extension_from_filename(modelName);
	}
	std::unordered_map<uint32_t,uint32_t> originalMaterialIndexToModelMaterialIndex {};
	if(texturesImported == false)
	{
		auto srcMatPath = ufile::get_path_from_filename(*filePath);
		importedTextures.reserve(aiScene->mNumMaterials);
		for(auto i=decltype(aiScene->mNumMaterials){0u};i<aiScene->mNumMaterials;++i)
		{
			auto *mat = aiScene->mMaterials[i];
			aiString aiName;
			mat->Get(AI_MATKEY_NAME,aiName);
			std::string matName = aiName.C_Str();
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
	
	// Build skeleton
	auto referencePose = Frame::Create(1);
	std::function<std::shared_ptr<pragma::animation::Bone>(aiNode&,pragma::animation::Bone*)> fIterateBones = nullptr;
	fIterateBones = [&fIterateBones,&skeleton,&referencePose,&fGetAbsolutePose](aiNode &node,pragma::animation::Bone *parent) -> std::shared_ptr<pragma::animation::Bone> {
		auto *bone = new pragma::animation::Bone{};
		bone->name = node.mName.C_Str();
		auto boneIdx = skeleton.AddBone(bone);
		if(bone->name.empty())
			bone->name = "bone" +std::to_string(boneIdx);
		if(parent)
			parent->children.insert(std::make_pair(boneIdx,skeleton.GetBone(boneIdx).lock()));
		
		aiVector3D scale;
		aiQuaternion rot;
		aiVector3D pos;
		auto t = fGetAbsolutePose(node);
		// t.Inverse();
		t.Decompose(scale,rot,pos);
		referencePose->SetBonePosition(boneIdx,Vector3{pos.x,pos.y,pos.z});
		referencePose->SetBoneOrientation(boneIdx,Quat{rot.w,rot.x,rot.y,rot.z});
		if(scale.x != 1.f || scale.y != 1.f || scale.z != 1.f)
			referencePose->SetBoneScale(boneIdx,Vector3{scale.x,scale.y,scale.z});

		auto numBones = skeleton.GetBones().size() +node.mNumChildren;
		skeleton.GetBones().reserve(numBones);
		referencePose->SetBoneCount(numBones);
		for(auto i=decltype(node.mNumChildren){0};i<node.mNumChildren;++i)
		{
			auto &child = *node.mChildren[i];
			fIterateBones(child,bone);
		}
		return skeleton.GetBone(boneIdx).lock();
	};
	if(aiScene->mRootNode)
	{
		auto rootBone = fIterateBones(*aiScene->mRootNode,nullptr);
		skeleton.GetRootBones().insert(std::make_pair(rootBone->ID,rootBone));
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

	// Generate model
	auto meshGroup = mdl->AddMeshGroup("reference");
	auto mesh = game->CreateModelMesh();
	meshGroup->AddMesh(mesh);

	for(auto &subMesh : subMeshes)
		mesh->AddSubMesh(subMesh);

	mdl->Update();
	return mdl->Save(*game,"addons/converted/models/" +outputFilePath,outErr);
}
#endif
bool Lua::import::import_model_asset(NetworkState &nw, const std::string &outputPath, std::string &outFilePath, std::string &outErr)
{
	auto mdlPath = outputPath + ".blend";
	auto f = filemanager::open_file("models/" + mdlPath, filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		return false;
	outFilePath = outputPath;
	return false; //::import_model_asset(nw,f,outFilePath,outErr);
}

int Lua::import::import_model_asset(lua_State *l)
{
#if 0
	auto &f = *Lua::CheckFile(l,1);
	auto hFile = f.GetHandle();
	if(hFile->GetType() != VFILE_LOCAL)
		return 0;
	auto t = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &subMesh : subMeshes)
	{
		Lua::PushInt(l,idx++);
		Lua::Push<std::shared_ptr<ModelSubMesh>>(l,subMesh);
		Lua::SetTableValue(l,t);
	}

	auto tTextures = Lua::CreateTable(l);
	for(auto i=decltype(importedTextures.size()){0};i<importedTextures.size();++i)
	{
		auto &tex = importedTextures.at(i);
		Lua::PushInt(l,i +1);
		Lua::PushString(l,tex);
		Lua::SetTableValue(l,tTextures);
	}

	Lua::Push<std::shared_ptr<pragma::animation::Skeleton>>(l,skeleton);
	Lua::Push<std::shared_ptr<Frame>>(l,referencePose);
	return ::import_model_asset(*engine->GetNetworkState(l),f);
#endif
	return 0;
}
