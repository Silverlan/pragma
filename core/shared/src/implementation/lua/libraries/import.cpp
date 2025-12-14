// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#if 0
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/DefaultLogger.hpp>
#endif

module pragma.shared;

import :scripting.lua.libraries.lib_import;

int Lua::import::import_wad(lua::State *l)
{
	auto &f = Lua::Check<LFile>(l, 1);
	auto &skeleton = Lua::Check<pragma::animation::Skeleton>(l, 2);
	std::array<uint8_t, 3> header;
	f.Read(header.data(), header.size() * sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'A' || header.at(2) != 'D') {
		PushBool(l, false);
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
		auto frame = ::Frame::Create(numBones);
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
int Lua::import::import_wrci(lua::State *l)
{
	auto &f = Lua::Check<LFile>(l, 1);
	auto &mdl = Lua::Check<pragma::asset::Model>(l, 2);
	std::array<uint8_t, 4> header;
	f.Read(header.data(), header.size() * sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'C' || header.at(3) != 'I') {
		PushBool(l, false);
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
	auto *game = pragma::Engine::Get()->GetNetworkState(l)->GetGameState();
	auto numMeshes = f.Read<uint8_t>();
	for(auto i = decltype(numMeshes) {0}; i < numMeshes; ++i) {
		auto colMesh = pragma::physics::CollisionMesh::Create(game);
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
		colMesh->Update(pragma::asset::ModelUpdateFlags::All);
	}
	mdl.Update();
	PushBool(l, true);
	return 1;
}
int Lua::import::import_wrmi(lua::State *l)
{
	auto &f = Lua::Check<LFile>(l, 1);
	auto &mdl = Lua::Check<pragma::asset::Model>(l, 2);
	std::array<uint8_t, 4> header;
	f.Read(header.data(), header.size() * sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'R' || header.at(2) != 'M' || header.at(3) != 'I') {
		PushBool(l, false);
		return 1;
	}
	auto meshGroup = mdl.GetMeshGroup(0u);
	if(meshGroup == nullptr) // TODO
	{
		PushBool(l, false);
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
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto mesh = std::shared_ptr<pragma::geometry::ModelMesh>(nw->CreateMesh());
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
		std::vector<pragma::math::VertexWeight> boneWeights(numVerts);
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
			auto subMesh = std::shared_ptr<pragma::geometry::ModelSubMesh>(nw->CreateSubMesh());
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
	mdl.Update(pragma::asset::ModelUpdateFlags::All);
	mdl.GenerateBindPoseMatrices();
	PushBool(l, true);
	return 1;
}

int Lua::import::import_smd(lua::State *l)
{
	auto &nw = *pragma::Engine::Get()->GetNetworkState(l);
	std::string smdFileName = CheckString(l, 1);
	/*if(Lua::file::validate_write_operation(l,smdFileName) == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}*/
	auto f = pragma::fs::open_file(smdFileName.c_str(), pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary);
	if(f == nullptr) {
		PushBool(l, false);
		return 1;
	}
	auto &mdl = Lua::Check<pragma::asset::Model>(l, 2);
	std::string animName = CheckString(l, 3);
	auto isCollisionMesh = false;
	if(IsSet(l, 4))
		isCollisionMesh = CheckBool(l, 4);

	std::vector<std::string> textures;
	auto success = pragma::util::port_hl2_smd(nw, mdl, f, animName, isCollisionMesh, textures);
	PushBool(l, success);
	if(success) {
		auto t = CreateTable(l);
		for(auto i = decltype(textures.size()) {0u}; i < textures.size(); ++i) {
			PushInt(l, i + 1);
			PushString(l, textures.at(i));
			SetTableValue(l, t);
		}
		return 2;
	}
	return 1;
}

int Lua::import::import_obj(lua::State *l)
{
	auto &f = Lua::Check<LFile>(l, 1);
	return import_model_asset(l);
}

int Lua::import::export_model_asset(lua::State *l) { return 0; }

bool Lua::import::import_model_asset(pragma::NetworkState &nw, const std::string &outputPath, std::string &outFilePath, std::string &outErr)
{
	auto mdlPath = outputPath + ".blend";
	auto f = pragma::fs::open_file("models/" + mdlPath, pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary);
	if(!f)
		return false;
	outFilePath = outputPath;
	return false; //::import_model_asset(nw,f,outFilePath,outErr);
}

int Lua::import::import_model_asset(lua::State *l)
{
#if 0
	auto &f = Lua::Check<LFile>(l,1);
	auto hFile = f.GetHandle();
	if(hFile->GetType() != EVFile::Local)
		return 0;
	auto t = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &subMesh : subMeshes)
	{
		Lua::PushInt(l,idx++);
		Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l,subMesh);
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
	return ::import_model_asset(*pragma::Engine::Get()->GetNetworkState(l),f);
#endif
	return 0;
}
