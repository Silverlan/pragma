// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.server;
import :server_state;

import :entities;
import :entities.components;
import :model_manager;

#define RESOURCE_TRANSFER_VERBOSE 0

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
		// UNUSED(r);
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
		ServerState::Get()->SendPacket("resource_mdl_rough", pOut, pragma::networking::Protocol::FastUnreliable, *cl);
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

	auto actions = packet->Read<pragma::Action>();
	pOut->Write<pragma::Action>(actions);

	auto bController = packet->Read<bool>();
	pOut->Write<bool>(bController);
	auto *actionInputC = pl->GetActionInputController();
	if(bController == true) {
		auto actionValues = umath::get_power_of_2_values(umath::to_integral(actions));
		for(auto v : actionValues) {
			auto magnitude = packet->Read<float>();
			pOut->Write<float>(magnitude);
			if(actionInputC)
				actionInputC->SetActionInputAxisMagnitude(static_cast<pragma::Action>(v), magnitude);
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
