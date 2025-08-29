// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/networking/c_net_resources.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/networking/resources.h"
#include "pragma/entities/components/c_model_component.hpp"
#include <pragma/model/modelmanager.h>
#include <fsys/filesystem.h>
#include "pragma/model/c_modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/model/c_model.h"
#include "pragma/networking/iclient.hpp"
#include <sharedutils/util_file.h>
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_library.hpp>
#include <pragma/game/game_resources.h>

import pragma.client.model;

#define RESOURCE_TRANSFER_VERBOSE 0

void ClientState::StartResourceTransfer()
{
	FileManager::CreatePath(m_svInfo->GetDownloadPath().c_str());

	if(m_client != nullptr)
		m_client->SetTimeoutDuration(0.0); // Disable timeout until resource transfer has been completed
	NetPacket resourceReq;
	resourceReq->Write<bool>(GetConVarBool("cl_allowdownload"));
	SendPacket("resource_begin", resourceReq, pragma::networking::Protocol::SlowReliable);
}

void ClientState::HandleClientResource(NetPacket &packet)
{
	std::string file = packet->ReadString();
	if(!IsValidResource(file)) {
		NetPacket response;
		response->Write<bool>(false);
		SendPacket("resourceinfo_response", response, pragma::networking::Protocol::SlowReliable);
		return;
	}
	auto bDefaultPath = true;
	std::string ext;
	auto fileDst = file;
	if(ufile::get_extension(file, &ext) == true) {
		if(ext == Lua::FILE_EXTENSION_PRECOMPILED) {
			bDefaultPath = false;
			fileDst = m_svInfo->GetDownloadPath() + fileDst; // Lua-files are placed into a sub-directory specific to this server
		}
	}
	if(bDefaultPath == true)
		fileDst = "downloads\\" + fileDst; // Files are placed into 'downloads' directory by default (Which is automatically mounted)

	FileManager::CreatePath(fileDst.substr(0, fileDst.find_last_of('\\')).c_str());
	auto size = packet->Read<UInt64>();
	Con::ccl << "Downloading file '" << file << "' (" << util::get_pretty_bytes(size) << ")..." << Con::endl;
	auto f = FileManager::OpenFile(file.c_str(), "rb"); //,fsys::SearchFlags::Local);
	NetPacket response;
	bool bSkip = false;
	if(f != NULL) {
		if(f->GetSize() == size) {
			bSkip = true;
			Con::ccl << "File '" << file << "' doesn't differ from server's. Skipping..." << Con::endl;
			f.reset();
			response->Write<bool>(false);
		}
		else
			f = nullptr;
	}
	if(bSkip == false)
		f = FileManager::OpenFile((fileDst + ".part").c_str(), "wb");
	if(!bSkip) {
		if(f == NULL) {
			response->Write<bool>(false);
			Con::cwar << Con::PREFIX_CLIENT << "[ResourceManager] Unable to write file '" << fileDst << "'. Skipping..." << Con::endl;
		}
		else {
			response->Write<bool>(true);
			m_resDownload = std::make_unique<ResourceDownload>(std::static_pointer_cast<VFilePtrInternalReal>(f), fileDst, CUInt32(size));
		}
	}
	SendPacket("resourceinfo_response", response, pragma::networking::Protocol::SlowReliable);
}

void ClientState::HandleClientResourceFragment(NetPacket &packet)
{
	if(m_resDownload == NULL)
		return;
	std::array<uint8_t, RESOURCE_TRANSFER_FRAGMENT_SIZE> buf;
	unsigned int read = packet->Read<unsigned int>();
	packet->Read(buf.data(), read);
	auto &res = m_resDownload;
	auto f = res->file;
	f->Write(buf.data(), read);
	NetPacket resourceReq;
#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::ccl << "[ResourceManager] " << ((f->Tell() / float(res->size)) * 100) << "%" << Con::endl;
#endif
	if(read < RESOURCE_TRANSFER_FRAGMENT_SIZE) {
		auto resName = res->name;
		res = nullptr;
		f = nullptr;
		resourceReq->Write<bool>(true);

		if((FileManager::Exists(resName.c_str()) == true && FileManager::RemoveFile(resName.c_str()) == false) || FileManager::RenameFile((resName + ".part").c_str(), resName.c_str()) == false)
			Con::ccl << "File '" << (resName + ".part") << "' successfully received, but unable to rename to '" << resName << "'... Requesting next..." << Con::endl;
		else
			Con::ccl << "File '" << resName << "' successfully received... Requesting next..." << Con::endl;
	}
	else
		resourceReq->Write<bool>(false);
	SendPacket("resource_request", resourceReq, pragma::networking::Protocol::SlowReliable);
}

////////////////////

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

DLLCLIENT void NET_cl_resourceinfo(NetPacket packet) { client->HandleClientResource(packet); }
DLLCLIENT void NET_cl_resourcecomplete(NetPacket packet)
{
	Con::ccl << "All resources have been received!" << Con::endl;

	auto *cl = client->GetClient();
	if(cl != nullptr)
		cl->SetTimeoutDuration(client->GetConVarFloat("sv_timeout_duration")); // Resource transfer complete; Reset timeout

	Con::ccl << "Requesting Game Info..." << Con::endl;
	client->SendUserInfo();
}

DLLCLIENT void NET_cl_resource_fragment(NetPacket packet) { client->HandleClientResourceFragment(packet); }

class ModelLoadManager {
  private:
	static std::shared_ptr<ModelLoadManager> s_manager;
  private:
	struct ModelLoadQuery {
		NetPacket packet;
		std::shared_ptr<Model> model;
		std::string fileName;
	};
	std::mutex m_loadMutex;
	std::mutex m_completeMutex;
	std::atomic<bool> m_bComplete = {false};
	std::queue<std::shared_ptr<ModelLoadQuery>> m_loadQueries;
	std::queue<std::shared_ptr<ModelLoadQuery>> m_completeQueries;
	std::thread m_thread;
	CallbackHandle m_hCbTick;
	CallbackHandle m_hCbOnGameEnd;
	void Update();
  protected:
	ModelLoadManager();
  public:
	~ModelLoadManager();
	static ModelLoadManager &Initialize();
	void AddQuery(const NetPacket &packet, const std::shared_ptr<Model> &mdl, const std::string &fileName);
};
decltype(ModelLoadManager::s_manager) ModelLoadManager::s_manager = nullptr;

ModelLoadManager &ModelLoadManager::Initialize()
{
	if(s_manager == nullptr)
		s_manager = std::shared_ptr<ModelLoadManager>(new ModelLoadManager());
	return *s_manager;
}

void ModelLoadManager::AddQuery(const NetPacket &packet, const std::shared_ptr<Model> &mdl, const std::string &fileName)
{
	auto query = std::shared_ptr<ModelLoadQuery>(new ModelLoadQuery());
	query->packet = packet;
	query->model = mdl;
	query->fileName = fileName;
	m_loadMutex.lock();
	m_loadQueries.push(query);
	m_loadMutex.unlock();
}

ModelLoadManager::ModelLoadManager()
{
	m_thread = std::thread(std::bind(&ModelLoadManager::Update, this));
	m_hCbTick = c_game->AddCallback("Tick", FunctionCallback<>::Create([this]() {
		m_completeMutex.lock();
		if(!m_completeQueries.empty()) {
			std::vector<BaseEntity *> *ents;
			c_game->GetEntities(&ents);
			while(!m_completeQueries.empty()) {
				auto &query = m_completeQueries.front();
				for(auto *ent : *ents) {
					if(ent == nullptr)
						continue;
					auto mdlComponent = ent->GetModelComponent();
					if(!mdlComponent)
						continue;
					auto mdlName = mdlComponent->GetModelName();
					if(mdlName != query->fileName)
						continue;
#if RESOURCE_TRANSFER_VERBOSE == 1
					Con::ccl << "[ResourceManager] Updating model..." << Con::endl;
#endif
					mdlComponent->SetModel(query->model);
				}
				m_completeQueries.pop();
			}
		}
		m_completeMutex.unlock();
	}));
	m_hCbOnGameEnd = c_game->AddCallback("OnGameEnd", FunctionCallback<void, CGame *>::Create([](CGame *) { s_manager = nullptr; }));
}

ModelLoadManager::~ModelLoadManager()
{
	m_bComplete = true;
	m_thread.join();
	if(m_hCbTick.IsValid())
		m_hCbTick.Remove();
	if(m_hCbOnGameEnd.IsValid())
		m_hCbOnGameEnd.Remove();
}

//

class UVCalculator {
  protected:
	enum class Facing : uint32_t { Up, Forward, Right };
	static Facing FacingDirection(const Vector3 &v, Vector2 &scale)
	{
		auto ret = Facing::Up;
		auto maxDot = 0.f;
		if(!FacesThisWay(v, uvec::RIGHT, Facing::Right, maxDot, ret)) {
			if(FacesThisWay(v, -uvec::RIGHT, Facing::Right, maxDot, ret))
				scale.x *= -1.f;
		}

		if(!FacesThisWay(v, uvec::FORWARD, Facing::Forward, maxDot, ret))
			FacesThisWay(v, -uvec::FORWARD, Facing::Forward, maxDot, ret);

		if(!FacesThisWay(v, uvec::UP, Facing::Up, maxDot, ret))
			FacesThisWay(v, -uvec::UP, Facing::Up, maxDot, ret);
		return ret;
	}
	static bool FacesThisWay(const Vector3 &v, const Vector3 &dir, Facing p, float &maxDot, Facing &ret)
	{
		auto t = uvec::dot(v, dir);
		if(t > maxDot) {
			ret = p;
			maxDot = t;
			return true;
		}
		return false;
	}
	static Vector2 ScaledUV(float uv1, float uv2, float scale) { return {uv1 / scale, uv2 / -scale}; }
  public:
	static std::vector<Vector2> CalculateUVs(const std::vector<Vector3> &vertices, float scale = 1.f)
	{
		std::vector<Vector2> uvs;
		uvs.resize(vertices.size());

		for(auto i = decltype(vertices.size()) {0}; i < vertices.size(); i += 3) {
			auto i0 = i;
			auto i1 = i0 + 1;
			auto i2 = i1 + 1;
			auto &v0 = vertices[i0];
			auto &v1 = vertices[i1];
			auto &v2 = vertices[i2];

			auto s1 = v1 - v0;
			auto s2 = v2 - v0;
			auto dir = uvec::cross(s1, s2);
			Vector2 uvScale {1.f, 1.f};
			auto facing = FacingDirection(dir, uvScale);
			switch(facing) {
			case Facing::Forward:
				uvs[i0] = ScaledUV(v0.z, v0.y, scale);
				uvs[i1] = ScaledUV(v1.z, v1.y, scale);
				uvs[i2] = ScaledUV(v2.z, v2.y, scale);
				break;
			case Facing::Up:
				uvs[i0] = ScaledUV(v0.x, v0.z, scale);
				uvs[i1] = ScaledUV(v1.x, v1.z, scale);
				uvs[i2] = ScaledUV(v2.x, v2.z, scale);
				break;
			case Facing::Right:
				uvs[i0] = ScaledUV(v0.x, v0.y, scale);
				uvs[i1] = ScaledUV(v1.x, v1.y, scale);
				uvs[i2] = ScaledUV(v2.x, v2.y, scale);
				break;
			}
			uvs[i0] *= uvScale;
			uvs[i1] *= uvScale;
			uvs[i2] *= uvScale;
		}
		return uvs;
	}
};

//

void ModelLoadManager::Update()
{
	while(m_bComplete == false) {
		m_loadMutex.lock();
		if(m_loadQueries.empty()) {
			m_loadMutex.unlock();
			continue;
		}
		auto query = m_loadQueries.front();
		m_loadQueries.pop();
		m_loadMutex.unlock();
		static std::shared_ptr<util::Library> dllHandle = nullptr;
		if(dllHandle == nullptr) {
			std::string err;
			dllHandle = client->LoadLibraryModule("pcl/util_pcl", {}, &err);
			if(dllHandle == nullptr) {
				Con::cwar << "Unable to load PCL module. Rough models will not be generated!" << Con::endl;
				return;
			}
		}
		static auto *ptrBuildMesh = dllHandle->FindSymbolAddress<void (*)(const std::vector<Vector3> &, std::vector<Vector3> &, std::vector<uint32_t> &)>("pcl_build_convex_mesh");

		auto &mdl = query->model;
		auto &packet = query->packet;

		auto group = mdl->AddMeshGroup("reference");
		auto mesh = std::make_shared<CModelMesh>();

		auto type = packet->Read<uint8_t>();
		auto numMeshes = packet->Read<uint32_t>();
		//#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::ccl << "[ResourceManager] Received " << numMeshes << " meshes" << Con::endl;
		//#endif
		for(auto i = decltype(numMeshes) {0}; i < numMeshes; ++i) {
			auto subMesh = std::make_shared<CModelSubMesh>();
			auto colMesh = CollisionMesh::Create(c_game);

			Vector3 origin {};
			if(type == 0) {
				auto boneId = packet->Read<int32_t>();
				colMesh->SetBoneParent(boneId);

				origin = packet->Read<Vector3>();
				colMesh->SetOrigin(origin);
			}

			auto numVerts = packet->Read<uint32_t>();
			//#if RESOURCE_TRANSFER_VERBOSE == 1
			Con::ccl << "[ResourceManager] Vertex Count: " << numVerts << Con::endl;
			//#endif
			std::vector<Vector3> verts(numVerts);
			packet->Read(verts.data(), sizeof(Vector3) * numVerts);

			// Build convex mesh
			std::vector<Vector3> convexVerts;
			std::vector<uint32_t> convexTriangles;
			ptrBuildMesh(verts, convexVerts, convexTriangles);
			//

			std::vector<Vector3> convexNormals;
			convexNormals.reserve(convexVerts.size());
			for(auto i = decltype(convexVerts.size()) {0}; i < convexVerts.size(); ++i) {
				Vector3 n {};
				for(auto j = decltype(convexTriangles.size()) {0}; j < convexTriangles.size(); j += 3) {
					auto idx0 = convexTriangles[j];
					auto idx1 = convexTriangles[j + 1];
					auto idx2 = convexTriangles[j + 2];
					if(idx0 == i || idx1 == i || idx2 == i) // Vertex is part of this triangle
					{
						Vector3 faceNormal = uvec::cross(convexVerts[idx1] - convexVerts[idx0], convexVerts[idx2] - convexVerts[idx0]);
						uvec::normalize(&faceNormal);
						n += faceNormal;
					}
				}
				uvec::normalize(&n);
				convexNormals.push_back(n);
			}

			auto &dstVerts = subMesh->GetVertices();
			colMesh->GetVertices() = convexVerts;

			// Build UV coordinates
			std::vector<Vector3> meshVerts;
			meshVerts.reserve(convexTriangles.size());
			std::vector<Vector3> meshNormals;
			meshNormals.reserve(convexTriangles.size());
			std::vector<uint32_t> meshIndices;
			meshIndices.reserve(convexTriangles.size());
			for(auto idx : convexTriangles) {
				meshVerts.push_back(convexVerts[idx]);
				meshNormals.push_back(convexNormals[idx]);
				meshIndices.push_back(static_cast<uint32_t>(meshIndices.size()));
			}
			auto uvs = UVCalculator::CalculateUVs(meshVerts, 50.f);
			//

			for(auto i = decltype(meshVerts.size()) {0}; i < meshVerts.size(); ++i) {
				auto &v = meshVerts[i];
				auto &uv = uvs[i];
				auto &n = meshNormals[i];
				dstVerts.push_back(umath::Vertex {-origin + v, uv, n});
			}

			subMesh->ReserveIndices(subMesh->GetIndexCount() + meshIndices.size());
			for(auto &idx : meshIndices)
				subMesh->AddIndex(idx);
			//

			mesh->AddSubMesh(subMesh);
			mdl->AddCollisionMesh(colMesh);
		}
		group->AddMesh(mesh);
		mdl->Update(ModelUpdateFlags::All);

		m_completeMutex.lock();
		m_completeQueries.push(query);
		m_completeMutex.unlock();
	}
}

void NET_cl_resource_mdl_rough(NetPacket packet)
{
	if(client == nullptr)
		return;
	auto fileName = packet->ReadString();
	auto mdl = client->GetModelManager().CreateModel(fileName);
	if(mdl == nullptr)
		return;
	mdl->AddMaterial(0, client->LoadMaterial("loading"));

	auto &manager = ModelLoadManager::Initialize();
#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::ccl << "[ResourceManager] Adding query: " << fileName << Con::endl;
#endif
	manager.AddQuery(packet, mdl, fileName);
}
