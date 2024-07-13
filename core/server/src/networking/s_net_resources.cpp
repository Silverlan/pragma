/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/networking/s_net_resources.h"
#include "pragma/networking/resourcemanager.h"
#include <fsys/filesystem.h>
#include "pragma/networking/resource.h"
#include "pragma/networking/iserver.hpp"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/player.h"
#include <servermanager/sv_nwm_recipientfilter.h>
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/model/s_modelmanager.h"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/networking/enums.hpp>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/action_input_controller_component.hpp>
#include <material_manager2.hpp>
#include <sharedutils/util_file.h>

#define RESOURCE_TRANSFER_VERBOSE 0

extern DLLSERVER SGame *s_game;
extern DLLSERVER ServerState *server;

void ServerState::InitResourceTransfer(pragma::networking::IServerClient &session)
{
	auto state = session.GetInitialResourceTransferState();
	if(state == pragma::networking::IServerClient::TransferState::Initial || session.IsTransferring() == true)
		return;
	HandleServerNextResource(session);
}

void ServerState::SendResourceFile(const std::string &f, const std::vector<pragma::networking::IServerClient *> &clients)
{
	std::string ext;
	auto bMdl = (ufile::get_extension(f, &ext) == true && ext == "wmd") ? true : false;
	if(bMdl == true)
		SendRoughModel(f, clients);
	for(auto *cl : clients) {
		auto r = cl->AddResource(f);
		UNUSED(r);
		InitResourceTransfer(*cl);
	}
}

void ServerState::SendResourceFile(const std::string &f)
{
	if(m_server == nullptr)
		return;
	auto clientHandles = m_server->GetClients();
	std::vector<pragma::networking::IServerClient *> clients;
	clients.reserve(clientHandles.size());
	for(auto &hClient : clientHandles)
		clients.push_back(hClient.get());
	SendResourceFile(f, clients);
}

void ServerState::SendRoughModel(const std::string &f, const std::vector<pragma::networking::IServerClient *> &clients)
{
	if(m_server == nullptr)
		return;
	auto mdlName = f.substr(7, f.length()); // Path without "models\\"-prefix
	                                        //#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::csv << "[ResourceManager] Got Request For: " << mdlName << Con::endl;
	//#endif
	auto *asset = GetModelManager().FindCachedAsset(mdlName);
	if(asset == nullptr)
		return;
	auto mdl = pragma::asset::ModelManager::GetAssetObject(*asset);
	NetPacket pOut;
	pOut->WriteString(mdlName);

	auto &colMeshes = mdl->GetCollisionMeshes();
	if(colMeshes.empty() == false) {
		pOut->Write<uint8_t>(static_cast<uint8_t>(0));
		pOut->Write<uint32_t>(static_cast<uint32_t>(colMeshes.size()));
		for(auto &colMesh : colMeshes) {
			pOut->Write<int32_t>(colMesh->GetBoneParent());
			pOut->Write<Vector3>(colMesh->GetOrigin());
			auto &verts = colMesh->GetVertices();
			pOut->Write<uint32_t>(static_cast<uint32_t>(verts.size()));
			for(auto &v : verts)
				pOut->Write<Vector3>(v);
		}
	}
	else {
		pOut->Write<uint8_t>(static_cast<uint8_t>(1));
		auto numMeshes = mdl->GetSubMeshCount();
		pOut->Write<uint32_t>(numMeshes);
		for(auto &meshGroup : mdl->GetMeshGroups()) {
			for(auto &mesh : meshGroup->GetMeshes()) {
				for(auto &subMesh : mesh->GetSubMeshes()) {
					assert(numMeshes > 0);
					if(numMeshes-- == 0)
						goto endLoop;
					auto numVerts = subMesh->GetVertexCount();
					pOut->Write<uint32_t>(numVerts);
					for(auto &v : subMesh->GetVertices())
						pOut->Write<Vector3>(v.position);
				}
			}
		}
	endLoop:;
		assert(numMeshes == 0);
		if(numMeshes > 0) {
			Con::cwar << "Model '" << mdlName << "' has invalid mesh count. Unable to generate rough mesh!" << Con::endl;
			return;
		}
	}
	for(auto *cl : clients) {
		server->SendPacket("resource_mdl_rough", pOut, pragma::networking::Protocol::FastUnreliable, *cl);
		//#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::csv << "[ResourceManager] Sent rough model to: " << cl->GetIdentifier() << "..." << Con::endl;
		//#endif
	}
}
void ServerState::SendRoughModel(const std::string &f)
{
	if(m_server == nullptr)
		return;
	auto clientHandles = m_server->GetClients();
	std::vector<pragma::networking::IServerClient *> clients;
	clients.reserve(clientHandles.size());
	for(auto &hClient : clientHandles)
		clients.push_back(hClient.get());
	SendRoughModel(f, clients);
}

void ServerState::HandleServerNextResource(pragma::networking::IServerClient &session)
{
	auto bRemoveCurrent = true;
	if(session.IsTransferring() == false) {
		bRemoveCurrent = false; // Transfer has just started; Don't remove first resource
		session.SetTransferComplete(false);
	}
	auto &resTransfer = session.GetResourceTransfer();
	size_t numResources;
	auto bComplete = session.IsInitialResourceTransferComplete();
	if(bComplete == false && resTransfer.empty()) {
		auto &resources = ResourceManager::GetResources();
		numResources = resources.size();
		if(numResources > 0) {
			for(auto &res : resources) {
				if(session.AddResource(res.fileName, res.stream) == false)
					Con::cwar << Con::PREFIX_SERVER << "[ResourceManager] Unable to open file '" << res.fileName << "'. Skipping..." << Con::endl;
			}
		}
		numResources = resTransfer.size();
	}
	else if(bRemoveCurrent == true) {
		numResources = resTransfer.size();
		if(numResources > 0) {
#if RESOURCE_TRANSFER_VERBOSE == 1
			auto &r = resTransfer[0];
			Con::csv << "[ResourceManager] File '" << r->name << "' transferred successfully to " << session->GetIP() << ". " << (numResources - 1) << " resources left!" << Con::endl;
#endif
			session.RemoveResource(0);
			numResources--;
		}
	}
	else
		numResources = resTransfer.size();
	if(numResources == 0 || (resTransfer.front()->stream == true && bComplete == false) // All static resources are complete; Starting dynamic resources
	) {
		if(bComplete == false) {
			session.SetInitialResourceTransferState(pragma::networking::IServerClient::TransferState::Complete);
			Con::csv << "All resources have been sent to client '" << session.GetIdentifier() << "'!" << Con::endl;
			NetPacket p;
			SendPacket("resourcecomplete", p, pragma::networking::Protocol::SlowReliable, session);
		}
		if(numResources == 0) {
			session.SetTransferComplete(true);
			return;
		}
	}
	auto &r = resTransfer[0];
	auto f = r->file;
	UInt64 size = CUInt64(f->GetSize());
	NetPacket packetRes;
	packetRes->WriteString(r->name);
	packetRes->Write<UInt64>(size);
	SendPacket("resourceinfo", packetRes, pragma::networking::Protocol::SlowReliable, session);
}

void ServerState::HandleServerResourceStart(pragma::networking::IServerClient &session, NetPacket &packet)
{
	auto &resTransfer = session.GetResourceTransfer();
	if(resTransfer.empty()) {
		Con::cwar << "Attempted to send invalid resource fragment to client " << session.GetIdentifier() << Con::endl;
		return;
	}
	bool send = packet->Read<bool>();
	if(send) {
		Con::csv << "Sending file '" << resTransfer[0]->name << "' to client '" << session.GetIdentifier() << "'" << Con::endl;
		HandleServerResourceFragment(session);
	}
	else
		HandleServerNextResource(session);
}

void ServerState::HandleServerResourceFragment(pragma::networking::IServerClient &session)
{
	auto &resTransfer = session.GetResourceTransfer();
	if(resTransfer.empty()) {
		Con::cwar << "Attempted to send invalid resource fragment to client " << session.GetIdentifier() << Con::endl;
		return;
	}
	auto &r = resTransfer[0];
	auto f = r->file;
	f->Seek(r->offset);
	NetPacket fragment;
	std::array<uint8_t, RESOURCE_TRANSFER_FRAGMENT_SIZE> buf;
	UInt64 size = CUInt64(f->GetSize());
	auto read = CUInt32(size - r->offset);
	if(read > RESOURCE_TRANSFER_FRAGMENT_SIZE)
		read = RESOURCE_TRANSFER_FRAGMENT_SIZE;
	f->Read(buf.data(), read);
	fragment->Write<unsigned int>(read);
	fragment->Write(buf.data(), read);
	r->offset += read;
	SendPacket("resource_fragment", fragment, pragma::networking::Protocol::SlowReliable, session);
}

void ServerState::ReceiveUserInput(pragma::networking::IServerClient &client, NetPacket &packet)
{
	auto *pl = GetPlayer(client);
	if(pl == nullptr)
		return;
	auto latency = client.GetLatency() / 2.f; // Latency is entire roundtrip; We need the time for one way
	auto tActivated = (util::clock::to_int(util::clock::get_duration_since_start()) - packet.GetTimeActivated()) / 1'000'000.0;
	//Con::ccl<<"Snapshot delay: "<<+latency<<"+ "<<tActivated<<" = "<<(latency +tActivated)<<Con::endl;
	auto tDelta = static_cast<float>((latency + tActivated) / 1'000.0);

	NetPacket pOut;
	nwm::write_player(pOut, pl);

	auto userInputId = packet->Read<uint8_t>();

	//auto latency = packet->Read<uint8_t>();
	//client->SetLatency(latency); // Deprecated: Let the networkmanager handle it

	auto sPlComponent = pl->GetEntity().GetComponent<pragma::SPlayerComponent>();
	auto orientation = nwm::read_quat(packet);
	if(sPlComponent.expired() == false)
		sPlComponent.get()->UpdateViewOrientation(orientation);
	nwm::write_quat(pOut, orientation);
	pl->SetViewPos(packet->Read<Vector3>());

	auto actions = packet->Read<Action>();
	pOut->Write<Action>(actions);

	auto bController = packet->Read<bool>();
	pOut->Write<bool>(bController);
	auto *actionInputC = pl->GetActionInputController();
	if(bController == true) {
		auto actionValues = umath::get_power_of_2_values(umath::to_integral(actions));
		for(auto v : actionValues) {
			auto magnitude = packet->Read<float>();
			pOut->Write<float>(magnitude);
			if(actionInputC)
				actionInputC->SetActionInputAxisMagnitude(static_cast<Action>(v), magnitude);
		}
	}
	if(actionInputC)
		actionInputC->SetActionInputs(actions, bController);
	//Con::csv<<"Action inputs "<<actions<<" for player "<<pl<<" ("<<pl->GetClientSession()->GetIP()<<")"<<Con::endl;

	SendPacket("playerinput", pOut, pragma::networking::Protocol::FastUnreliable, {client, pragma::networking::ClientRecipientFilter::FilterType::Exclude});

	NetPacket plPacket;
	plPacket->Write<uint8_t>(userInputId);
	SendPacket("playerinput", plPacket, pragma::networking::Protocol::FastUnreliable, client);
}

////////////////////

extern ServerState *server;
void NET_sv_resourceinfo_response(pragma::networking::IServerClient &session, NetPacket packet) { server->HandleServerResourceStart(session, packet); }

void NET_sv_resource_request(pragma::networking::IServerClient &session, NetPacket packet)
{
	bool b = packet->Read<bool>();
#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::csv << "[ResourceManager] Got resource request from client: " << session->GetIdentifier() << " (" << b << ")" << Con::endl;
#endif
	if(b)
		server->HandleServerNextResource(session);
	else
		server->HandleServerResourceFragment(session);
}

void NET_sv_resource_begin(pragma::networking::IServerClient &session, NetPacket packet)
{
	session.SetInitialResourceTransferState(pragma::networking::IServerClient::TransferState::Started);
	bool bSend = packet->Read<bool>() && server->GetConVarBool("sv_allowdownload");
	if(bSend) {
#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::csv << "[ResourceManager] Sending next resource to client: " << session->GetIdentifier() << Con::endl;
#endif
		server->HandleServerNextResource(session);
	}
	else {
#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::csv << "[ResourceManager] All resources have been sent to: " << session->GetIdentifier() << Con::endl;
#endif
		NetPacket p;
		server->SendPacket("resourcecomplete", p, pragma::networking::Protocol::SlowReliable, session);
	}
}

void NET_sv_query_resource(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(s_game == nullptr)
		return;
	auto fileName = packet->ReadString();
	//#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::csv << "[ResourceManager] Query Resource: " << fileName << Con::endl;
	//#endif
	if(s_game->IsValidGameResource(fileName) == false) // Client isn't allowed to download this resource
	{
		session.ScheduleResource(fileName); // Might be allowed to download the resource in the future, remember it!
		return;
	}
	server->SendResourceFile(fileName, {&session});
}

void NET_sv_query_model_texture(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto mdlName = packet->ReadString();
	auto matName = packet->ReadString();
	auto *asset = server->GetModelManager().FindCachedAsset(mdlName);
	if(asset == nullptr)
		return;
	auto mdl = pragma::asset::ModelManager::GetAssetObject(*asset);
	std::string dstName;
	if(mdl->FindMaterial(matName, dstName) == false)
		return;
	auto &matManager = server->GetMaterialManager();
	auto normalizedName = matManager.ToCacheIdentifier(dstName);
	auto *matAsset = matManager.FindCachedAsset(dstName);
	auto mat = matAsset ? msys::MaterialManager::GetAssetObject(*matAsset) : nullptr;
	if(mat == nullptr)
		return;
	auto &block = mat->GetDataBlock();
	std::vector<std::string> textures;
	std::function<void(const std::shared_ptr<ds::Block> &, std::vector<std::string> &)> fFindTextures = nullptr;
	fFindTextures = [&fFindTextures](const std::shared_ptr<ds::Block> &block, std::vector<std::string> &textures) {
		auto *data = block->GetData();
		if(data == nullptr)
			return;
		for(auto &pair : *data) {
			auto v = pair.second;
			if(v->IsBlock() == true)
				fFindTextures(std::static_pointer_cast<ds::Block>(v), textures);
			else {
				auto dataTex = std::dynamic_pointer_cast<ds::Texture>(v);
				if(dataTex != nullptr) {
					auto &texInfo = dataTex->GetValue();
					textures.push_back("materials\\" + texInfo.name);
				}
			}
		}
	};
	fFindTextures(block, textures);

	std::vector<pragma::networking::IServerClient *> vSession = {&session};
	for(auto &tex : textures)
		server->SendResourceFile(tex, vSession);
	server->SendResourceFile("materials\\" + normalizedName, vSession);
}
