// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.net_messages;

import :entities;
import :entities.components;
import :game;
import :model_manager;
import :networking.iserver_client;
import :server_state;
import pragma.shared;

static void NET_sv_RESOURCEINFO_RESPONSE(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_RESOURCE_REQUEST(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_RESOURCE_BEGIN(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_QUERY_RESOURCE(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_QUERY_MODEL_TEXTURE(pragma::networking::IServerClient &session, NetPacket packet);

static void NET_sv_DISCONNECT(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_USERINPUT(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_CLIENTINFO(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_GAME_READY(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_CMD_CALL(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_RCON(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_SERVERINFO_REQUEST(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_AUTHENTICATE(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_LUANET(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_CMD_SETPOS(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_CVAR_SET(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_NOCLIP(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_NOTARGET(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_GODMODE(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_SUICIDE(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_HURTME(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_WEAPON_NEXT(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_WEAPON_PREVIOUS(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_ENT_EVENT(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_GIVE_WEAPON(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_STRIP_WEAPONS(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_GIVE_AMMO(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_DEBUG_AI_NAVIGATION(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_DEBUG_AI_SCHEDULE_PRINT(pragma::networking::IServerClient &session, NetPacket packet);
static void NET_sv_DEBUG_AI_SCHEDULE_TREE(pragma::networking::IServerClient &session, NetPacket packet);

static void NET_sv_CL_SEND(pragma::networking::IServerClient &session, NetPacket packet);

#define REGISTER_NET_MSG(NAME) netMessageMap.RegisterNetMessage(server::NAME, +[](pragma::networking::ServerClientHandle session, NetPacket packet) { NET_sv_##NAME(*static_cast<pragma::networking::IServerClient *>(session), packet); })

static void register_net_messages(pragma::networking::ServerMessageMap &netMessageMap)
{
	using namespace pragma::networking::net_messages;
	REGISTER_NET_MSG(RESOURCEINFO_RESPONSE);
	REGISTER_NET_MSG(RESOURCE_REQUEST);
	REGISTER_NET_MSG(RESOURCE_BEGIN);
	REGISTER_NET_MSG(QUERY_RESOURCE);
	REGISTER_NET_MSG(QUERY_MODEL_TEXTURE);

	REGISTER_NET_MSG(DISCONNECT);
	REGISTER_NET_MSG(USERINPUT);
	REGISTER_NET_MSG(CLIENTINFO);
	REGISTER_NET_MSG(GAME_READY);
	REGISTER_NET_MSG(CMD_CALL);
	REGISTER_NET_MSG(RCON);
	REGISTER_NET_MSG(SERVERINFO_REQUEST);
	REGISTER_NET_MSG(AUTHENTICATE);
	REGISTER_NET_MSG(LUANET);
	REGISTER_NET_MSG(CMD_SETPOS);
	REGISTER_NET_MSG(CVAR_SET);
	REGISTER_NET_MSG(NOCLIP);
	REGISTER_NET_MSG(NOTARGET);
	REGISTER_NET_MSG(GODMODE);
	REGISTER_NET_MSG(SUICIDE);
	REGISTER_NET_MSG(HURTME);
	REGISTER_NET_MSG(WEAPON_NEXT);
	REGISTER_NET_MSG(WEAPON_PREVIOUS);
	REGISTER_NET_MSG(ENT_EVENT);
	REGISTER_NET_MSG(GIVE_WEAPON);
	REGISTER_NET_MSG(STRIP_WEAPONS);
	REGISTER_NET_MSG(GIVE_AMMO);
	REGISTER_NET_MSG(DEBUG_AI_NAVIGATION);
	REGISTER_NET_MSG(DEBUG_AI_SCHEDULE_PRINT);
	REGISTER_NET_MSG(DEBUG_AI_SCHEDULE_TREE);

	REGISTER_NET_MSG(CL_SEND);
}

void pragma::networking::register_server_net_messages()
{
	static auto netMessagesRegistered = false;
	if(netMessagesRegistered)
		return;
	netMessagesRegistered = true;

	::register_net_messages(*get_server_message_map());
}

#define RESOURCE_TRANSFER_VERBOSE 0

void NET_sv_RESOURCEINFO_RESPONSE(pragma::networking::IServerClient &session, NetPacket packet) { pragma::ServerState::Get()->HandleServerResourceStart(session, packet); }

void NET_sv_RESOURCE_REQUEST(pragma::networking::IServerClient &session, NetPacket packet)
{
	bool b = packet->Read<bool>();
#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::CSV << "[ResourceManager] Got resource request from client: " << session->GetIdentifier() << " (" << b << ")" << Con::endl;
#endif
	if(b)
		pragma::ServerState::Get()->HandleServerNextResource(session);
	else
		pragma::ServerState::Get()->HandleServerResourceFragment(session);
}

void NET_sv_RESOURCE_BEGIN(pragma::networking::IServerClient &session, NetPacket packet)
{
	session.SetInitialResourceTransferState(pragma::networking::IServerClient::TransferState::Started);
	bool bSend = packet->Read<bool>() && pragma::ServerState::Get()->GetConVarBool("sv_allowdownload");
	if(bSend) {
#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::CSV << "[ResourceManager] Sending next resource to client: " << session->GetIdentifier() << Con::endl;
#endif
		pragma::ServerState::Get()->HandleServerNextResource(session);
	}
	else {
#if RESOURCE_TRANSFER_VERBOSE == 1
		Con::CSV << "[ResourceManager] All resources have been sent to: " << session->GetIdentifier() << Con::endl;
#endif
		NetPacket p;
		pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::RESOURCECOMPLETE, p, pragma::networking::Protocol::SlowReliable, session);
	}
}

void NET_sv_QUERY_RESOURCE(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(pragma::SGame::Get() == nullptr)
		return;
	auto fileName = packet->ReadString();
	//#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::CSV << "[ResourceManager] Query Resource: " << fileName << Con::endl;
	//#endif
	if(pragma::SGame::Get()->IsValidGameResource(fileName) == false) // Client isn't allowed to download this resource
	{
		session.ScheduleResource(fileName); // Might be allowed to download the resource in the future, remember it!
		return;
	}
	pragma::ServerState::Get()->SendResourceFile(fileName, {&session});
}

void NET_sv_QUERY_MODEL_TEXTURE(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto mdlName = packet->ReadString();
	auto matName = packet->ReadString();
	auto *asset = pragma::ServerState::Get()->GetModelManager().FindCachedAsset(mdlName);
	if(asset == nullptr)
		return;
	auto mdl = pragma::asset::ModelManager::GetAssetObject(*asset);
	std::string dstName;
	if(mdl->FindMaterial(matName, dstName) == false)
		return;
	auto &matManager = pragma::ServerState::Get()->GetMaterialManager();
	auto normalizedName = matManager.ToCacheIdentifier(dstName);
	auto *matAsset = matManager.FindCachedAsset(dstName);
	auto mat = matAsset ? pragma::material::MaterialManager::GetAssetObject(*matAsset) : nullptr;
	if(mat == nullptr)
		return;
	std::vector<std::string> textures;
	std::function<void(const pragma::util::Path &path)> fFindTextures = nullptr;
	fFindTextures = [mat, &fFindTextures, &textures](const pragma::util::Path &path) {
		for(auto &name : pragma::material::MaterialPropertyBlockView {*mat, path}) {
			auto propType = mat->GetPropertyType(name);
			switch(propType) {
			case pragma::material::PropertyType::Block:
				fFindTextures(pragma::util::FilePath(path, name));
				break;
			case pragma::material::PropertyType::Texture:
				{
					std::string texName;
					if(mat->GetProperty(pragma::util::FilePath(path, name).GetString(), &texName)) {
						auto path = pragma::util::FilePath(pragma::asset::get_asset_root_directory(pragma::asset::Type::Material), texName).GetString();
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
		pragma::ServerState::Get()->SendResourceFile(tex, vSession);
	pragma::ServerState::Get()->SendResourceFile("materials\\" + normalizedName, vSession);
}

void NET_sv_DISCONNECT(pragma::networking::IServerClient &session, NetPacket packet)
{
#ifdef DEBUG_SOCKET
	Con::CSV << "Client '" << session.GetIdentifier() << "' has disconnected." << Con::endl;
#endif
	pragma::ServerState::Get()->DropClient(session);
}

void NET_sv_USERINPUT(pragma::networking::IServerClient &session, NetPacket packet) { pragma::ServerState::Get()->ReceiveUserInput(session, packet); }

void NET_sv_ENT_EVENT(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->IsGameActive())
		return;
	auto *pl = pragma::SGame::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	packet->SetOffset(packet->GetDataSize() - sizeof(UInt32) - sizeof(unsigned int));
	auto *ent = static_cast<SBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto eventId = packet->Read<UInt32>();
	packet->SetOffset(0);
	ent->ReceiveNetEvent(*pl, eventId, packet);
}

void NET_sv_CLIENTINFO(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->IsGameActive())
		return;
	pragma::SGame *game = pragma::ServerState::Get()->GetGameState();
	game->ReceiveUserInfo(session, packet);
}

void NET_sv_GAME_READY(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->IsGameActive())
		return;
	auto *game = pragma::ServerState::Get()->GetGameState();
	game->ReceiveGameReady(session, packet);
}

void NET_sv_CMD_SETPOS(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(pragma::ServerState::Get()->CheatsEnabled() == false)
		return;
	if(!pragma::ServerState::Get()->IsGameActive())
		return;
	auto *pl = pragma::SGame::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto pTrComponent = pl->GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	Vector3 pos = pragma::networking::read_vector(packet);
	pTrComponent->SetPosition(pos);
}

void NET_sv_CMD_CALL(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto *pl = pragma::SGame::Get()->GetPlayer(session);
	std::string cmd = packet->ReadString();
	auto pressState = static_cast<KeyState>(packet->Read<uint8_t>());
	auto magnitude = packet->Read<float>();
	unsigned char argc = packet->Read<unsigned char>();
	std::vector<std::string> argv;
	for(unsigned char i = 0; i < argc; i++)
		argv.push_back(packet->ReadString());

	auto bActionCmd = (cmd.empty() == false && cmd.front() == '+') ? true : false;
	auto bReleased = (pressState == KeyState::Release) ? true : false;
	auto r = pragma::ServerState::Get()->RunConsoleCommand(cmd, argv, pl, pressState, magnitude, [bActionCmd, bReleased](pragma::console::ConConf *cf, float &magnitude) -> bool {
		if(bReleased == false || bActionCmd == true)
			return true;
		auto flags = cf->GetFlags();
		if((flags & (pragma::console::ConVarFlags::JoystickAxisContinuous | pragma::console::ConVarFlags::JoystickAxisSingle)) == pragma::console::ConVarFlags::None)
			return false;
		return true;
	});
	if(pragma::Engine::Get()->GetClientState() != nullptr)
		return;
	NetPacket p;
	auto *cv = pragma::ServerState::Get()->GetConVar(cmd);
	if(r == false || cv == nullptr)
		p->Write<uint8_t>(static_cast<uint8_t>(0));
	else {
		if(cv->GetType() == pragma::console::ConType::Var) {
			p->Write<uint8_t>(2);
			p->WriteString(static_cast<pragma::console::ConVar *>(cv)->GetString());
		}
		else {
			p->Write<uint8_t>(1);
			p->WriteString("");
		}
	}
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::CMD_CALL_RESPONSE, p, pragma::networking::Protocol::SlowReliable, session);
}

void NET_sv_RCON(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->IsGameActive())
		return;
	std::string passCl = packet->ReadString();
	std::string cvar = packet->ReadString();
	std::string pass = pragma::ServerState::Get()->GetConVarString("rcon_password");
	if(pass.empty() || pass != passCl) {
		/*Game *game = pragma::ServerState::Get()->GetGameState();
		Player *pl = game->GetPlayer(session);
		if(pl == nullptr)
			return;
		pl->PrintMessage("Bad RCON password",MESSAGE::PRINTCONSOLE);*/ // TODO
		Con::CERR << "Incorrect RCON Password! (" << passCl << ")" << Con::endl;
		return;
	}
	Con::CSV << "Remote console input from " << session.GetIdentifier() << ": '" << cvar << "'" << Con::endl;
	pragma::Engine::Get()->ConsoleInput(cvar.c_str());
}

void NET_sv_SERVERINFO_REQUEST(pragma::networking::IServerClient &session, NetPacket packet)
{
	std::string password = packet->ReadString();
	std::string passSv = pragma::ServerState::Get()->GetConVarString("sv_password").c_str();
	if(passSv.empty() == false && passSv != password && session.IsListenServerHost() == false) {
		NetPacket p;
		//pragma::ServerState::Get()->SendPacket("invalidpassword", p, pragma::networking::Protocol::SlowReliable, session);
		pragma::ServerState::Get()->DropClient(session);
		return;
	}
	NetPacket p;
	auto *sv = pragma::ServerState::Get()->GetServer();
	if(sv && sv->GetHostPort().has_value()) {
		auto port = sv->GetHostPort();
		p->Write<unsigned char>(1);
		p->Write<unsigned short>(*port);
	}
	else
		p->Write<unsigned char>((unsigned char)(0));

	p->Write<bool>(pragma::ServerState::Get()->IsClientAuthenticationRequired());
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::SERVERINFO, p, pragma::networking::Protocol::SlowReliable, session);
}

void NET_sv_AUTHENTICATE(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto hasAuth = packet->Read<bool>();
	if(pragma::ServerState::Get()->IsClientAuthenticationRequired()) {
		if(hasAuth == false) {
			Con::CERR << "Unable to authenticate client '" << session.GetIdentifier() << "': Client did not transmit authentication information!" << Con::endl;
			pragma::ServerState::Get()->DropClient(session, pragma::networking::DropReason::AuthenticationFailed);
			return;
		}
		auto *reg = pragma::ServerState::Get()->GetMasterServerRegistration();
		if(reg == nullptr) {
			Con::CERR << "Unable to authenticate client '" << session.GetIdentifier() << "': Server is not connected to master server!" << Con::endl;
			pragma::ServerState::Get()->DropClient(session, pragma::networking::DropReason::AuthenticationFailed);
			return;
		}
		auto steamId = packet->Read<uint64_t>();
		auto lenToken = packet->Read<uint16_t>();
		std::vector<char> token;
		token.resize(lenToken);
		packet->Read(token.data(), token.size() * sizeof(token.front()));

		std::string err;
		auto libSteamworks = pragma::ServerState::Get()->InitializeLibrary("steamworks/pr_steamworks", &err);
		if(libSteamworks == nullptr) {
			Con::CERR << "Unable to authenticate client with steam id '" << steamId << "': Steamworks module could not be loaded: " << err << Con::endl;
			pragma::ServerState::Get()->DropClient(session, pragma::networking::DropReason::AuthenticationFailed);
			reg->DropClient(steamId);
			return;
		}
		session.SetSteamId(steamId);
		// Authentication will end with 'OnClientAuthenticated' callback,
		// which will handle the rest
		reg->AuthenticateAndAddClient(steamId, token, "Player");
		return;
	}
	// No authentication required; Continue immediately
	pragma::ServerState::Get()->OnClientAuthenticated(session, {});
}

void NET_sv_CVAR_SET(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->IsGameActive())
		return;
	pragma::SGame *game = pragma::ServerState::Get()->GetGameState();
	auto *pl = game->GetPlayer(session);
	if(pl == nullptr)
		return;
	std::string cvar = packet->ReadString();
	std::string val = packet->ReadString();
	std::unordered_map<std::string, std::string> *cvars;
	pl->GetConVars(&cvars);
	(*cvars)[cvar] = val;
	game->OnClientConVarChanged(*pl, cvar, val);
}

void NET_sv_NOCLIP(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto pPhysComponent = pl->GetEntity().GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	auto bNoclip = pPhysComponent->GetMoveType() != pragma::physics::MoveType::Noclip;
	if(bNoclip == false) {
		pPhysComponent->SetMoveType(pragma::physics::MoveType::Walk);
		pPhysComponent->SetCollisionFilterGroup(pragma::physics::CollisionMask::Player);
	}
	else {
		pPhysComponent->SetMoveType(pragma::physics::MoveType::Noclip);
		pPhysComponent->SetCollisionFilterGroup(pragma::physics::CollisionMask::NoCollision);
		//pl->SetCollisionsEnabled(false); // Bugged due to CCD
	}
	NetPacket p;
	pragma::networking::write_entity(p, &pl->GetEntity());
	p->Write<bool>(bNoclip);
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::PL_TOGGLE_NOCLIP, p, pragma::networking::Protocol::SlowReliable);
}

void NET_sv_LUANET(pragma::networking::IServerClient &session, NetPacket packet) { pragma::ServerState::Get()->HandleLuaNetPacket(session, packet); }

void NET_sv_NOTARGET(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(charComponent == nullptr)
		return;
	charComponent->SetNoTarget(!charComponent->GetNoTarget());
	pl->PrintMessage(std::string("Notarget turned ") + ((charComponent->GetNoTarget() == true) ? "ON" : "OFF"), pragma::console::MESSAGE::PRINTCONSOLE);
}

void NET_sv_GODMODE(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(charComponent == nullptr)
		return;
	charComponent->SetGodMode(!charComponent->GetGodMode());
	pl->PrintMessage(std::string("God mode turned ") + ((charComponent->GetGodMode() == true) ? "ON" : "OFF"), pragma::console::MESSAGE::PRINTCONSOLE);
}

void NET_sv_SUICIDE(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	charComponent->Kill();
}

void NET_sv_HURTME(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled())
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	auto pDamageableComponent = ent.GetComponent<pragma::DamageableComponent>();
	if(pDamageableComponent.expired())
		return;
	auto dmg = packet->Read<uint16_t>();
	pragma::game::DamageInfo dmgInfo {};
	dmgInfo.SetDamage(dmg);
	dmgInfo.SetAttacker(&ent);
	dmgInfo.SetInflictor(&ent);
	dmgInfo.SetDamageType(Crush);
	pDamageableComponent->TakeDamage(dmgInfo);
}

void NET_sv_WEAPON_NEXT(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectNextWeapon();
}

void NET_sv_WEAPON_PREVIOUS(pragma::networking::IServerClient &session, NetPacket packet)
{
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->SelectPreviousWeapon();
}

void NET_sv_GIVE_WEAPON(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled() || pragma::SGame::Get() == nullptr)
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	auto weaponClass = packet->ReadString();
	auto *wep = sCharComponent->GiveWeapon(weaponClass);
	if(wep == nullptr)
		return;
	sCharComponent->DeployWeapon(*wep);
}

void NET_sv_STRIP_WEAPONS(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled() || pragma::SGame::Get() == nullptr)
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	sCharComponent->RemoveWeapons();
}

void NET_sv_GIVE_AMMO(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled() || pragma::SGame::Get() == nullptr)
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto ammoTypeClass = packet->ReadString();
	uint32_t ammoTypeId;
	if(pragma::SGame::Get()->GetAmmoType(ammoTypeClass, &ammoTypeId) == nullptr)
		return;
	auto sCharComponent = static_cast<pragma::SCharacterComponent *>(pl->GetEntity().GetCharacterComponent().get());
	if(sCharComponent == nullptr)
		return;
	auto am = packet->Read<uint32_t>();
	am += sCharComponent->GetAmmoCount(ammoTypeId);
	sCharComponent->SetAmmoCount(ammoTypeId, static_cast<uint16_t>(am));
}

void NET_sv_DEBUG_AI_SCHEDULE_PRINT(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled() || pragma::SGame::Get() == nullptr)
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *npc = pragma::networking::read_entity(packet);
	if(npc == nullptr || npc->IsNPC() == false)
		return;
	auto sAiComponent = npc->GetComponent<pragma::SAIComponent>();
	auto schedule = (sAiComponent.expired() == false) ? sAiComponent.get()->GetCurrentSchedule() : nullptr;
	NetPacket response;
	if(schedule == nullptr)
		response->Write<bool>(false);
	else {
		response->Write<bool>(true);
		std::stringstream ss;
		schedule->DebugPrint(ss);
		response->WriteString(ss.str());
	}
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::DEBUG_AI_SCHEDULE_PRINT, response, pragma::networking::Protocol::SlowReliable, session);
}

void NET_sv_DEBUG_AI_SCHEDULE_TREE(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled() || pragma::SGame::Get() == nullptr)
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto *ent = pragma::networking::read_entity(packet);
	if(ent == nullptr || ent->IsNPC() == false)
		return;
	auto hPl = pl->GetHandle<pragma::SPlayerComponent>();
	auto sAiComponent = ent->GetComponent<pragma::SAIComponent>();
	auto hSAiComponent = (sAiComponent.expired() == false) ? sAiComponent.get()->GetHandle<pragma::SAIComponent>() : pragma::ComponentHandle<pragma::SAIComponent> {};
	auto dbgTree = pragma::util::make_shared<pragma::debug::DebugBehaviorTreeNode>();
	std::shared_ptr<pragma::ai::Schedule> aiSchedule = nullptr;
	auto tLastScheduleUpdate = 0.f;
	auto hCbTick = FunctionCallback<void>::Create(nullptr);
	auto hCbOnGameEnd = FunctionCallback<void, pragma::SGame *>::Create(nullptr);
	auto fClearCallbacks = [hCbTick, hCbOnGameEnd]() mutable {
		if(hCbTick.IsValid())
			hCbTick.Remove();
		if(hCbOnGameEnd.IsValid())
			hCbOnGameEnd.Remove();
	};
	hCbTick.get<Callback<void>>()->SetFunction([hPl, hSAiComponent, hCbTick, hCbOnGameEnd, fClearCallbacks, dbgTree, aiSchedule, tLastScheduleUpdate]() mutable {
		if(hPl.expired() || hSAiComponent.expired()) {
			fClearCallbacks();
			return;
		}
		hSAiComponent.get()->_debugSendScheduleInfo(*hPl.get(), dbgTree, aiSchedule, tLastScheduleUpdate);
	});
	hCbOnGameEnd.get<Callback<void, pragma::SGame *>>()->SetFunction([fClearCallbacks](pragma::SGame *game) mutable { fClearCallbacks(); });
	pragma::SGame::Get()->AddCallback("Tick", hCbTick);
	pragma::SGame::Get()->AddCallback("OnGameEnd", hCbOnGameEnd);
}

void NET_sv_DEBUG_AI_NAVIGATION(pragma::networking::IServerClient &session, NetPacket packet)
{
	if(!pragma::ServerState::Get()->CheatsEnabled() || pragma::SGame::Get() == nullptr)
		return;
	auto *pl = pragma::ServerState::Get()->GetPlayer(session);
	if(pl == nullptr)
		return;
	auto b = packet->Read<bool>();
	if(b == false) {
		auto it = std::find_if(pragma::SAIComponent::s_plDebugAiNav.begin(), pragma::SAIComponent::s_plDebugAiNav.end(), [pl](const pragma::ComponentHandle<pragma::SPlayerComponent> &sPlComponent) { return (sPlComponent.get() == pl) ? true : false; });
		if(it != pragma::SAIComponent::s_plDebugAiNav.end())
			pragma::SAIComponent::s_plDebugAiNav.erase(it);
		return;
	}
	pragma::SAIComponent::s_plDebugAiNav.push_back(pl->GetHandle<pragma::SPlayerComponent>());
	auto &npcs = pragma::SAIComponent::GetAll();
	for(auto *npc : npcs)
		npc->_debugSendNavInfo(*pl);
}

void NET_sv_CL_SEND(pragma::networking::IServerClient &session, NetPacket packet)
{
	std::string msg = packet->ReadString();
	Con::CSV << "Received cl_send message from client '" << session.GetIdentifier() << "': " << msg << Con::endl;
}
