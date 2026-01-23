// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.game;

import :entities;
import :game;
import :server_state;

void Lua::game::Server::set_gravity(const Vector3 &gravity)
{
	pragma::Game *game = pragma::ServerState::Get()->GetGameState();
	game->SetGravity(const_cast<Vector3 &>(gravity));
}
Vector3 Lua::game::Server::get_gravity()
{
	pragma::Game *game = pragma::ServerState::Get()->GetGameState();
	return game->GetGravity();
}
luabind::object Lua::game::Server::load_model(lua::State *l, const std::string &name)
{
	auto mdl = pragma::SGame::Get()->LoadModel(name);
	if(mdl == nullptr)
		return {};
	return luabind::object {l, mdl};
}
int Lua::game::Server::create_model(lua::State *l)
{
	std::shared_ptr<pragma::asset::Model> mdl = nullptr;
	if(!IsSet(l, 1))
		mdl = pragma::SGame::Get()->CreateModel();
	else {
		if(IsBool(l, 1)) {
			auto bAddReference = true;
			if(IsSet(l, 1))
				bAddReference = CheckBool(l, 1);
			mdl = pragma::SGame::Get()->CreateModel(bAddReference);
		}
		else {
			std::string name = CheckString(l, 1);
			mdl = pragma::SGame::Get()->CreateModel(name);
		}
	}
	if(mdl == nullptr)
		return 0;
	Lua::Push<decltype(mdl)>(l, mdl);
	return 1;
}

int Lua::game::Server::load_map(lua::State *l)
{
	std::vector<EntityHandle> ents;
	auto hCb = pragma::util::make_shared<CallbackHandle>(pragma::SGame::Get()->AddCallback("OnEntityCreated", FunctionCallback<void, pragma::ecs::BaseEntity *>::Create([&ents](pragma::ecs::BaseEntity *ent) { ents.push_back(ent->GetHandle()); })));
	pragma::util::ScopeGuard sg([hCb]() {
		if(hCb->IsValid() == true)
			hCb->Remove();
	});

	std::string mapName;
	pragma::ecs::BaseEntity *entWorld = nullptr;
	Vector3 origin {};
	//auto startIdx = SGame::Get()->GetEntityMapIndexStart();
	auto pair = game::load_map(l, mapName, &entWorld, origin);
	if(pair.first == false)
		return pair.second;
	::NetPacket packet {};
	packet->WriteString(mapName);
	packet->Write<Vector3>(origin);
	//packet->Write<uint32_t>(startIdx);

	std::vector<SBaseEntity *> ptrEnts;
	ptrEnts.reserve(ents.size());
	for(auto &hEnt : ents) {
		if(hEnt.valid() == false)
			continue;
		ptrEnts.push_back(static_cast<SBaseEntity *>(hEnt.get()));
	}

	pragma::networking::ClientRecipientFilter filter {};
	pragma::SGame::Get()->WriteEntityData(packet, ptrEnts.data(), ptrEnts.size(), filter);
	packet->Write<bool>((entWorld != nullptr) ? true : false);

	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::MAP_LOAD, packet, pragma::networking::Protocol::SlowReliable);
	return pair.second;
}

void Lua::game::Server::change_level(const std::string &mapName, const std::string &landmarkName) { pragma::SGame::Get()->ChangeLevel(mapName, landmarkName); }
void Lua::game::Server::change_level(const std::string &mapName) { change_level(mapName, ""); }
