#include "stdafx_shared.h"
#include "pragma/lua/libraries/limport.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/classes/ldef_skeleton.hpp"
#include "pragma/lua/libraries/lfile.h"
#include <smdmodel.h>
#include <unordered_set>
#include <sharedutils/util_file.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef ENABLE_MMD_IMPORT
#include <util_mmd.hpp>
#endif

extern DLLENGINE Engine *engine;

int Lua::import::import_wad(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);
	auto &skeleton = *Lua::CheckSkeleton(l,2);
	std::array<uint8_t,3> header;
	f->Read(header.data(),header.size() *sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'A' || header.at(2) != 'D')
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto anim = std::make_shared<Animation>();
	auto version = f->Read<uint16_t>();
	auto flags = f->Read<uint32_t>();
	auto numBones = f->Read<uint32_t>();
	std::vector<uint32_t> boneList;
	boneList.reserve(numBones);
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto boneName = f->ReadString();
		auto boneId = skeleton.LookupBone(boneName);
		boneList.push_back(boneId);
	}
	anim->SetBoneList(boneList);
	auto numFrames = f->Read<uint32_t>();
	for(auto i=decltype(numFrames){0};i<numFrames;++i)
	{
		auto frame = std::make_shared<Frame>(numBones);
		for(auto j=decltype(numBones){0};j<numBones;++j)
		{
			auto rot = f->Read<Quat>();
			auto pos = f->Read<Vector3>();
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
	auto &mdl = *Lua::CheckModel(l,2);
	std::array<uint8_t,4> header;
	f->Read(header.data(),header.size() *sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'C' || header.at(3) != 'I')
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto version = f->Read<uint16_t>();
	auto numBones = f->Read<uint32_t>();
	std::vector<std::string> boneNames;
	boneNames.reserve(numBones);
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto name = f->ReadString();
		boneNames.push_back(name);
	}
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto pos = f->Read<Vector3>();
		auto rot = f->Read<Quat>();

	}
	auto *game = engine->GetNetworkState(l)->GetGameState();
	auto numMeshes = f->Read<uint8_t>();
	for(auto i=decltype(numMeshes){0};i<numMeshes;++i)
	{
		auto colMesh = CollisionMesh::Create(game);
		auto parentId = f->Read<int32_t>();
		auto numVerts = f->Read<uint64_t>();
		if(parentId != -1)
		{
			auto &boneName = boneNames.at(parentId);
			colMesh->SetBoneParent(mdl->LookupBone(boneName));
		}
		auto &verts = colMesh->GetVertices();
		verts.reserve(numVerts);
		for(auto j=decltype(numVerts){0};j<numVerts;++j)
		{
			auto v = f->Read<Vector3>();
			verts.push_back(v);
		}
		auto &tris = colMesh->GetTriangles();
		auto numFaces = f->Read<uint64_t>();
		tris.reserve(numFaces *3u);
		for(auto j=decltype(numFaces){0};j<numFaces;++j)
		{
			auto idx0 = f->Read<uint64_t>();
			auto idx1 = f->Read<uint64_t>();
			auto idx2 = f->Read<uint64_t>();
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
		mdl->AddCollisionMesh(colMesh);
		colMesh->Update(ModelUpdateFlags::All);
	}
	mdl->Update();
	Lua::PushBool(l,true);
	return 1;
}
int Lua::import::import_wrmi(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);
	auto &mdl = *Lua::CheckModel(l,2);
	std::array<uint8_t,4> header;
	f->Read(header.data(),header.size() *sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'M' || header.at(3) != 'I')
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto meshGroup = mdl->GetMeshGroup(0u);
	if(meshGroup == nullptr) // TODO
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto version = f->Read<uint16_t>();
	auto offData = f->Read<uint64_t>();
	auto offBones = f->Read<uint64_t>();
	auto offMeshes = f->Read<uint64_t>();
	auto numBones = f->Read<uint32_t>();

	auto &skeleton = mdl->GetSkeleton();
	skeleton.GetBones().clear();
	skeleton.GetRootBones().clear();
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto name = f->ReadString();
		auto *bone = new Bone();
		bone->name = name;
		skeleton.AddBone(bone);
	}
	auto &reference = mdl->GetReference(); // TODO: Update skeleton stuff if already exists
	reference.SetBoneCount(numBones);
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto rot = f->Read<Quat>();
		auto pos = f->Read<Vector3>();
		reference.SetBonePosition(i,pos);
		reference.SetBoneOrientation(i,rot);
	}

	std::function<void(std::shared_ptr<Bone>&)> fReadChildBones = nullptr;
	fReadChildBones = [&f,&fReadChildBones,&skeleton](std::shared_ptr<Bone> &parent) {
		auto numChildren = f->Read<uint32_t>();
		for(auto i=decltype(numChildren){0};i<numChildren;++i)
		{
			auto boneId = f->Read<uint32_t>();
			auto child = skeleton.GetBone(boneId).lock();
			parent->children.insert(std::make_pair(boneId,child));
			child->parent = parent;
			fReadChildBones(child);
		}
	};
	auto &rootBones = skeleton.GetRootBones();
	auto numRoot = f->Read<uint32_t>();
	for(auto i=decltype(numRoot){0};i<numRoot;++i)
	{
		auto boneId = f->Read<uint32_t>();
		auto bone = skeleton.GetBone(boneId).lock();
		rootBones.insert(std::make_pair(boneId,bone));
		fReadChildBones(bone);
	}
	auto *nw = engine->GetNetworkState(l);
	auto mesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
	meshGroup->AddMesh(mesh);
	auto numMeshes = f->Read<uint32_t>();
	auto &meta = mdl->GetMetaInfo();
	auto &texGroup = *mdl->GetTextureGroup(0); // TODO
	for(auto i=decltype(numMeshes){0};i<numMeshes;++i)
	{
		auto parentId = f->Read<int32_t>();
		auto numVerts = f->Read<uint64_t>();
		std::vector<std::pair<Vector3,Vector3>> verts(numVerts);
		for(auto j=decltype(numVerts){0};j<numVerts;++j)
		{
			auto &v = verts.at(j);
			v.first = f->Read<Vector3>();
			v.second = f->Read<Vector3>();
		}
		auto numBonesMesh = f->Read<uint32_t>();
		std::vector<VertexWeight> boneWeights(numVerts);
		for(auto j=decltype(numBonesMesh){0};j<numBonesMesh;++j)
		{

			// TODO
			auto boneId = f->Read<uint32_t>();
			//auto bValidBone = false;//(boneId < model.bones.size()) ? true : false;
			auto numBoneVerts = f->Read<uint64_t>();
			/*if(bValidBone == false)
			{
				f->Seek(f->Tell() +(sizeof(uint64_t) +sizeof(float)) *numBoneVerts);
				continue;
			}*/
			for(auto k=decltype(numBoneVerts){0};k<numBoneVerts;++k)
			{
				auto vertexId = f->Read<uint64_t>();
				auto weight = f->Read<Float>();

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

		auto numMaps = f->Read<uint32_t>();
		for(auto j=decltype(numMaps){0};j<numMaps;++j)
		{
			auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
			auto &meshVerts = subMesh->GetVertices();
			auto &meshTriangles = subMesh->GetTriangles();
			auto &meshWeights = subMesh->GetVertexWeights();
			auto map = f->ReadString();
			map += ".wmi";
			ufile::remove_extension_from_filename(map);

			auto *mat = nw->LoadMaterial(map);
			mdl->AddMaterial(j,mat);

			//meta.textures.push_back(map); // TODO: Only if not exists yet
			//texGroup.textures.push_back(meta.textures.size() -1u);
			subMesh->SetTexture(j);

			auto numFaces = f->Read<uint32_t>();
			meshTriangles.reserve(numFaces *3u);
			meshVerts.reserve(numFaces *3u);
			meshWeights.reserve(numFaces *3u);
			for(auto k=decltype(numFaces){0};k<numFaces;++k)
			{
				auto triIdx = k *3u;
				for(auto l=0u;l<3u;++l)
				{
					auto vertId = f->Read<uint64_t>();
					meshTriangles.push_back(meshVerts.size());
					auto u = f->Read<float>();
					auto v = 1.f -f->Read<float>();

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
	mdl->Update(ModelUpdateFlags::All);
	mdl->GenerateBindPoseMatrices();
	Lua::PushBool(l,true);
	return 1;
}

int Lua::import::import_smd(lua_State *l)
{
	/*auto &f = *Lua::CheckFile(l,1);
	auto &skeleton = *Lua::CheckSkeleton(l,2);
	auto smd = SMDModel::Load(f->GetHandle());
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
	return 0;
}

int Lua::import::import_obj(lua_State *l)
{
	auto &f = *Lua::CheckFile(l,1);

	std::vector<uint8_t> data(f->Size());
	f->Read(data.data(),data.size() *sizeof(data.front()));
	Assimp::Importer importer;
	auto *aiScene = importer.ReadFileFromMemory(data.data(),data.size() *sizeof(data.front()),0u);
	if(aiScene == nullptr)
		return 0;
	auto t = Lua::CreateTable(l);
	auto *nw = engine->GetNetworkState(l);
	for(auto i=decltype(aiScene->mNumMeshes){0};i<aiScene->mNumMeshes;++i)
	{
		auto *mesh = aiScene->mMeshes[i];
		auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
		auto &verts = subMesh->GetVertices();
		verts.reserve(mesh->mNumVertices);
		for(auto j=decltype(mesh->mNumVertices){0};j<mesh->mNumVertices;++j)
		{
			auto &aiVert = mesh->mVertices[j];
			//mesh->mNumUVComponents
			auto &aiUvw = mesh->mTextureCoords[0][j];
			auto &aiNormal = mesh->mNormals[j];

			verts.push_back({});
			auto &v = verts.back();
			v.position = {aiVert.x,aiVert.y,aiVert.z};
			v.uv = {aiUvw.x,-aiUvw.y};
			v.normal = {aiNormal.x,aiNormal.y,aiNormal.z};
		}

		auto &triangles = subMesh->GetTriangles();
		triangles.reserve(mesh->mNumFaces *3);
		for(auto j=decltype(mesh->mNumFaces){0};j<mesh->mNumFaces;++j)
		{
			auto &face = mesh->mFaces[j];
			//face.mNumIndices // TODO
			triangles.push_back(face.mIndices[0]);
			triangles.push_back(face.mIndices[1]);
			triangles.push_back(face.mIndices[2]);
		}
		Lua::PushInt(l,i +1);
		Lua::Push<std::shared_ptr<ModelSubMesh>>(l,subMesh);
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::import::import_pmx(lua_State *l)
{
#ifdef ENABLE_MMD_IMPORT
	auto &f = *Lua::CheckFile(l,1);
	auto &mdl = *Lua::CheckModel(l,2);
	auto fptr = f->GetHandle();
	auto mdlData = mmd::pmx::load(fptr);
	if(mdlData == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto meshGroup = mdl->GetMeshGroup(0u);
	if(meshGroup == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto *nw = engine->GetNetworkState(l);
	auto mesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
	meshGroup->AddMesh(mesh);

	std::string matPath = mdlData->characterName;
	ustring::to_lower(matPath);
	ustring::replace(matPath," ","_");
	matPath = "models\\mmd\\" +matPath;

	std::vector<uint32_t> texIds;
	texIds.reserve(mdlData->textures.size());
	auto *texGroup = mdl->GetTextureGroup(0u);
	for(auto &tex : mdlData->textures)
	{
		auto fTex = FileManager::OpenFile(tex.c_str(),"rb");
		if(fTex == nullptr)
			Con::cwar<<"WARNING: Unable to open texture '"<<tex<<"'!"<<Con::endl;
		else
		{

		}
		// Load DDS


		mdl->AddTexturePath(ufile::get_path_from_filename(tex));
		auto texName = ufile::get_file_from_filename(tex);
		ufile::remove_extension_from_filename(texName);
		auto localMatPath = matPath +'\\' +texName;
		auto matPathWmi = localMatPath +".wmi";
		if(FileManager::Exists(matPathWmi) == false && FileManager::CreatePath(ufile::get_path_from_filename(matPathWmi).c_str()) == true)
		{
			auto f = FileManager::OpenFile<VFilePtrReal>(matPathWmi.c_str(),"w");
			if(f != nullptr)
			{
				std::stringstream ss;
				ss<<"\"textured\"\n";
				ss<<"{\n";
				ss<<"\t$texture diffusemap \""<<localMatPath<<"\"\n";
				ss<<"\tsurfacematerial \"flesh\"\n";
				ss<<"}";
				f->WriteString(ss.str());
			}
		}
		auto *mat = nw->LoadMaterial(localMatPath);
		auto texId = mdl->AddTexture(texName,mat);
		if(texGroup != nullptr)
			texGroup->textures.push_back(texId);
		texIds.push_back(texId);
	}
	auto faceOffset = 0ull;
	std::unordered_map<int32_t,std::shared_ptr<ModelSubMesh>> subMeshes;
	for(auto &mat : mdlData->materials)
	{
		if(mat.textureIndex == -1)
			continue;
		auto it = subMeshes.find(mat.textureIndex);
		if(it == subMeshes.end())
			it = subMeshes.insert(std::make_pair(mat.textureIndex,std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh()))).first;
		auto &subMesh = *it->second;
		subMesh.SetTexture(texIds.at(mat.textureIndex));
		auto &verts = subMesh.GetVertices();
		auto &vertWeights = subMesh.GetVertexWeights();
		auto &tris = subMesh.GetTriangles();

		std::unordered_map<uint16_t,uint16_t> faceTranslationTable;
		tris.reserve(mat.faceCount);
		for(auto i=faceOffset;i<faceOffset +mat.faceCount;++i)
		{
			auto idx = mdlData->faces.at(i);
			auto it = faceTranslationTable.find(idx);
			if(it == faceTranslationTable.end())
			{
				if(verts.size() == verts.capacity())
				{
					verts.reserve(verts.size() +500);
					vertWeights.reserve(verts.capacity());
				}
				verts.push_back({});
				vertWeights.push_back({});
				auto &vMesh = verts.back();
				auto &vw = vertWeights.back();
				auto &v = mdlData->vertices.at(idx);
				vMesh.position = {v.position.at(0),v.position.at(1),v.position.at(2)};
				vMesh.normal = {v.normal.at(0),v.normal.at(1),v.normal.at(2)};
				vMesh.uv = {v.uv.at(0),v.uv.at(1)};

				vw.boneIds = {v.boneIds.at(0),v.boneIds.at(1),v.boneIds.at(2),v.boneIds.at(3)};
				vw.weights = {v.boneWeights.at(0),v.boneWeights.at(1),v.boneWeights.at(2),v.boneWeights.at(3)};

				it = faceTranslationTable.insert(std::make_pair(idx,verts.size() -1)).first;
			}
			tris.push_back(it->second);
		}
		faceOffset += mat.faceCount;

		mesh->AddSubMesh(it->second);
	}
	mdl->AddTexturePath(matPath);

	auto &skeleton = mdl->GetSkeleton();
	auto &bones = skeleton.GetBones();
	bones.clear();
	for(auto &bone : mdlData->bones)
	{
		auto *mdlBone = new Bone();
		mdlBone->name = bone.name;
		auto boneId = skeleton.AddBone(mdlBone);
		if(mdlBone->name.empty())
			mdlBone->name = "bone" +std::to_string(boneId);
	}
	auto &rootBones = skeleton.GetRootBones();
	rootBones.clear();
	auto &reference = mdl->GetReference();
	reference.SetBoneCount(mdlData->bones.size());
	auto boneId = 0u;
	for(auto &bone : mdlData->bones)
	{
		auto parentId = bone.parentBoneIdx;
		if(parentId != -1)
		{
			auto &parent = bones.at(parentId);
			auto &child = bones.at(boneId);
			parent->children.insert(std::make_pair(boneId,child));
			child->parent = parent;
		}
		else
			rootBones.insert(std::make_pair(boneId,bones.at(boneId)));
		reference.SetBonePosition(boneId,Vector3(bone.position.at(0),bone.position.at(1),bone.position.at(2)));
		reference.SetBoneOrientation(boneId,uquat::identity());
		++boneId;
	}
	auto frame = std::make_shared<Frame>(reference);
	auto anim = std::make_shared<Animation>();
	anim->AddFrame(frame);
	mdl->AddAnimation("reference",anim);

	auto numBones = mdlData->bones.size();
	std::vector<uint32_t> boneList(numBones);
	for(auto i=decltype(numBones){0};i<numBones;++i)
		boneList.at(i) = i;
	anim->SetBoneList(boneList);
	anim->Localize(skeleton);

	mdl->Update(ModelUpdateFlags::All);
	mdl->GenerateBindPoseMatrices();
	Lua::PushBool(l,true);
	return 1;
#else
	Lua::PushBool(l,false);
	return 1;
#endif
}
