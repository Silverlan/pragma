// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.savegame;
import pragma.filesystem;

using namespace pragma;

bool game::savegame::save(Game &game, const std::string &fileName, std::string &outErr)
{
	auto f = fs::open_file<fs::VFilePtrReal>(fileName, fs::FileMode::Write | fs::FileMode::Binary);
	if(f == nullptr) {
		outErr = "Unable to open file '" + fileName + "'!";
		return false;
	}
	auto udmData = udm::Data::Create(PSAV_IDENTIFIER, FORMAT_VERSION);
	auto outData = udmData->GetAssetData().GetData();

	outData["map"] << game.GetMapName();

	auto &ents = game.GetBaseEntities();
	uint32_t numEnts = 0;
	for(auto *ent : ents) {
		if(ent == nullptr)
			continue;
		++numEnts;
	}
	auto udmEntities = outData.AddArray("entities", numEnts);
	uint32_t entIdx = 0;
	for(auto *ent : ents) {
		if(ent == nullptr)
			continue;
		auto udmEnt = udmEntities[entIdx];
		udmEnt["index"] << ent->GetIndex();
		udmEnt["class"] << ent->GetClass().str;
		auto data = udmEnt["data"];
		ent->Save(data);

		++entIdx;
	}
	return udmData->Save(f);
}
bool game::savegame::load(Game &game, const std::string &fileName, std::string &outErr)
{
	auto udmData = util::load_udm_asset(fileName, &outErr);
	if(udmData == nullptr)
		return false;
	auto &data = *udmData;
	if(data.GetAssetType() != PSAV_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto &udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}
	// if(version > FORMAT_VERSION)
	// 	return false;

	std::string map;
	data["map"] >> map;

	if(game.LoadMap(map) == false) {
		outErr = "Unable to load map '" + map + "'!";
		return false;
	}

	auto udmEntities = data["entities"];
	std::vector<EntityHandle> entities {};
	entities.reserve(udmEntities.GetSize());
	for(auto udmEnt : udmEntities) {
		EntityIndex idx = 0;
		udmEnt["index"] >> idx;
		std::string className;
		udmEnt["class"] >> className;

		auto data = udmEnt["data"];
		auto *ent = game.CreateEntity(className);
		if(ent) {
			ent->Load(data);
			entities.push_back(ent->GetHandle());
		}
	}
	for(auto &hEnt : entities) {
		if(hEnt.valid() == false)
			continue;
		hEnt->Spawn();
	}
	return true;
}
