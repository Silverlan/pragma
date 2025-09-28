// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include <fsys/filesystem.h>
#include <servermanager/sv_nwm_recipientfilter.h>
#include <material_property_block_view.hpp>
#include <material_manager2.hpp>
#include <sharedutils/util_file.h>

import pragma.server.entities;
import pragma.server.entities.components;
import pragma.server.game;
import pragma.server.model_manager;
import pragma.server.server_state;

#define RESOURCE_TRANSFER_VERBOSE 0

void NET_sv_resourceinfo_response(pragma::networking::IServerClient &session, NetPacket packet) { ServerState::Get()->HandleServerResourceStart(session, packet); }

void NET_sv_resource_request(pragma::networking::IServerClient &session, NetPacket packet)
{
	bool b = packet->Read<bool>();
#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::csv << "[ResourceManager] Got resource request from client: " << session->GetIdentifier() << " (" << b << ")" << Con::endl;
#endif
	if(b)
		ServerState::Get()->HandleServerNextResource(session);
	else
		ServerState::Get()->HandleServerResourceFragment(session);
}

void NET_sv_resource_begin(pragma::networking::IServerClient &session, NetPacket packet)
{
	session.SetInitialResourceTransferState(pragma::networking::IServerClient::TransferState::Started);
	bool bSend = packet->Read<bool>() && ServerState::Get()->GetConVarBool("sv_allowdownload");
	if(bSend) {
#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::csv << "[ResourceManager] Sending next resource to client: " << session->GetIdentifier() << Con::endl;
#endif
		ServerState::Get()->HandleServerNextResource(session);
	}
	else {
#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::csv << "[ResourceManager] All resources have been sent to: " << session->GetIdentifier() << Con::endl;
#endif
		NetPacket p;
		ServerState::Get()->SendPacket("resourcecomplete", p, pragma::networking::Protocol::SlowReliable, session);
	}
}

void NET_sv_query_resource(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(SGame::Get() == nullptr)
		return;
	auto fileName = packet->ReadString();
	//#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::csv << "[ResourceManager] Query Resource: " << fileName << Con::endl;
	//#endif
	if(SGame::Get()->IsValidGameResource(fileName) == false) // Client isn't allowed to download this resource
	{
		session.ScheduleResource(fileName); // Might be allowed to download the resource in the future, remember it!
		return;
	}
	ServerState::Get()->SendResourceFile(fileName, {&session});
}

void NET_sv_query_model_texture(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto mdlName = packet->ReadString();
	auto matName = packet->ReadString();
	auto *asset = ServerState::Get()->GetModelManager().FindCachedAsset(mdlName);
	if(asset == nullptr)
		return;
	auto mdl = pragma::asset::ModelManager::GetAssetObject(*asset);
	std::string dstName;
	if(mdl->FindMaterial(matName, dstName) == false)
		return;
	auto &matManager = ServerState::Get()->GetMaterialManager();
	auto normalizedName = matManager.ToCacheIdentifier(dstName);
	auto *matAsset = matManager.FindCachedAsset(dstName);
	auto mat = matAsset ? msys::MaterialManager::GetAssetObject(*matAsset) : nullptr;
	if(mat == nullptr)
		return;
	std::vector<std::string> textures;
	std::function<void(const util::Path &path)> fFindTextures = nullptr;
	fFindTextures = [mat, &fFindTextures, &textures](const util::Path &path) {
		for(auto &name : msys::MaterialPropertyBlockView {*mat, path}) {
			auto propType = mat->GetPropertyType(name);
			switch(propType) {
			case msys::PropertyType::Block:
				fFindTextures(util::FilePath(path, name));
				break;
			case msys::PropertyType::Texture:
				{
					std::string texName;
					if(mat->GetProperty(util::FilePath(path, name).GetString(), &texName)) {
						auto path = util::FilePath(pragma::asset::get_asset_root_directory(pragma::asset::Type::Material), texName).GetString();
						textures.push_back(path);
					}
					break;
				}
			}
		}
	};
	fFindTextures({});

	std::vector<pragma::networking::IServerClient *> vSession = {&session};
	for(auto &tex : textures)
		ServerState::Get()->SendResourceFile(tex, vSession);
	ServerState::Get()->SendResourceFile("materials\\" + normalizedName, vSession);
}
