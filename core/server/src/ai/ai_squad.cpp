/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/ai/ai_squad.h"
#include <pragma/entities/baseentity.h>
#include <pragma/entities/baseentity_handle.h>

std::vector<std::shared_ptr<AISquad>> AISquad::s_squads;

std::shared_ptr<AISquad> AISquad::Create(const std::string &name)
{
	return std::shared_ptr<AISquad>(new AISquad(name));
}

const std::vector<std::shared_ptr<AISquad>> &AISquad::GetAll()
{
	for(auto it=s_squads.begin();it!=s_squads.end();)
	{
		auto &squad = *it;
		if(squad.use_count() == 1)
			it = s_squads.erase(it);
		else
			++it;
	}
	return s_squads;
}

AISquad::AISquad(const std::string &squadName)
	: std::enable_shared_from_this<AISquad>(),name(squadName)
{
	s_squads.push_back(shared_from_this());
}

AISquad::~AISquad()
{
	auto it = std::find_if(s_squads.begin(),s_squads.end(),[this](const std::shared_ptr<AISquad> &squad) {
		return (squad.get() == this) ? true : false;
	});
	if(it != s_squads.end())
		s_squads.erase(it);
}

void AISquad::Join(BaseEntity *ent)
{
	auto it = std::find_if(members.begin(),members.end(),[ent](const EntityHandle &hEnt) {
		return (hEnt.get() == ent) ? true : false;
	});
	if(it != members.end())
		return;
	members.push_back(ent->GetHandle());
}
void AISquad::Leave(BaseEntity *ent)
{
	auto it = std::find_if(members.begin(),members.end(),[ent](const EntityHandle &hEnt) {
		return (hEnt.get() == ent) ? true : false;
	});
	if(it == members.end())
		return;
	members.erase(it);
}
