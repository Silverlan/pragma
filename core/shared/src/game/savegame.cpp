/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/game/savegame.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

bool savegame::save(Game &game,const std::string &fileName)
{
	auto f = FileManager::OpenFile<VFilePtrReal>(fileName.c_str(),"wb");
	if(f == nullptr)
		return false;
	f->Write<std::array<char,3>>({'W','S','G'});
	f->Write<uint32_t>(VERSION);

	f->WriteString(game.GetMapName());

	auto &ents = game.GetBaseEntities();
	auto numEnts = game.GetEntityCount();
	f->Write<uint32_t>(numEnts);
	for(auto *ent : ents)
	{
		if(numEnts == 0u)
		{
			Con::cwar<<"WARNING: Entity count does not much number of actual entities! Cannot create savegame."<<Con::endl;
			return false;
		}

		f->Write<uint32_t>(ent->GetIndex());
		f->WriteString(ent->GetClass());

		DataStream ds {};
		ent->Save(ds);

		auto size = ds->GetSize();
		f->Write<uint32_t>(size);
		f->Write(ds->GetData(),size);

		--numEnts;
	}
	return true;
}
bool savegame::load(Game &game,const std::string &fileName)
{
	auto f = FileManager::OpenFile(fileName.c_str(),"rb");
	if(f == nullptr)
		return false;
	auto header = f->Read<std::array<char,3>>();
	if(header.at(0) != 'W' || header.at(1) != 'S' || header.at(2) != 'G')
		return false;
	auto version = f->Read<uint32_t>();
	if(version > VERSION)
	{
		Con::cwar<<"WARNING: Incompatible savegame version "<<version<<" of savegame '"<<fileName<<"'! Cannot load."<<Con::endl;
		return false;
	}
	auto mapName = f->ReadString();

	if(game.LoadMap(mapName.c_str()) == false)
		return false;

	auto numEnts = f->Read<uint32_t>();
	std::vector<EntityHandle> entities {};
	entities.reserve(numEnts);
	for(auto i=decltype(numEnts){0u};i<numEnts;++i)
	{
		auto index = f->Read<uint32_t>();
		auto className = f->ReadString();
		auto entDataSize = f->Read<uint32_t>();

		DataStream ds {entDataSize};
		f->Read(ds->GetData(),entDataSize);

		auto *ent = game.CreateEntity(className);
		if(ent != nullptr)
			ent->Load(ds);
		entities.push_back(ent->GetHandle());
	}
	for(auto &hEnt : entities)
	{
		if(hEnt.IsValid() == false)
			continue;
		hEnt->Spawn();
	}
	return true;
}
