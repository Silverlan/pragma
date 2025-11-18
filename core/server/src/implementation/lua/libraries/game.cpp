// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;
import :scripting.lua.libraries.game;

import :entities;
import :game;
import :server_state;

void Lua::game::Server::set_gravity(const Vector3 &gravity)
{
	pragma::Game *game = ServerState::Get()->GetGameState();
	game->SetGravity(const_cast<Vector3 &>(gravity));
}
Vector3 Lua::game::Server::get_gravity()
{
	pragma::Game *game = ServerState::Get()->GetGameState();
	return game->GetGravity();
}
luabind::object Lua::game::Server::load_model(lua::State *l, const std::string &name)
{
	auto mdl = SGame::Get()->LoadModel(name);
	if(mdl == nullptr)
		return {};
	return luabind::object {l, mdl};
}
int Lua::game::Server::create_model(lua::State *l)
{
	std::shared_ptr<pragma::Model> mdl = nullptr;
	if(!Lua::IsSet(l, 1))
		mdl = SGame::Get()->CreateModel();
	else {
		if(Lua::IsBool(l, 1)) {
			auto bAddReference = true;
			if(Lua::IsSet(l, 1))
				bAddReference = Lua::CheckBool(l, 1);
			mdl = SGame::Get()->CreateModel(bAddReference);
		}
		else {
			std::string name = Lua::CheckString(l, 1);
			mdl = SGame::Get()->CreateModel(name);
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
	auto hCb = ::util::make_shared<CallbackHandle>(SGame::Get()->AddCallback("OnEntityCreated", FunctionCallback<void, pragma::ecs::BaseEntity *>::Create([&ents](pragma::ecs::BaseEntity *ent) { ents.push_back(ent->GetHandle()); })));
	::util::ScopeGuard sg([hCb]() {
		if(hCb->IsValid() == true)
			hCb->Remove();
	});

	std::string mapName;
	pragma::ecs::BaseEntity *entWorld = nullptr;
	Vector3 origin {};
	//auto startIdx = SGame::Get()->GetEntityMapIndexStart();
	auto pair = Lua::game::load_map(l, mapName, &entWorld, origin);
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
	SGame::Get()->WriteEntityData(packet, ptrEnts.data(), ptrEnts.size(), filter);
	packet->Write<bool>((entWorld != nullptr) ? true : false);

	ServerState::Get()->SendPacket("map_load", packet, pragma::networking::Protocol::SlowReliable);
	return pair.second;
}

void Lua::game::Server::change_level(const std::string &mapName, const std::string &landmarkName) { SGame::Get()->ChangeLevel(mapName, landmarkName); }
void Lua::game::Server::change_level(const std::string &mapName) { change_level(mapName, ""); }
