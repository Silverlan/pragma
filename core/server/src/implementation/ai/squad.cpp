// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.squad;

std::vector<std::shared_ptr<AISquad>> AISquad::s_squads;

std::shared_ptr<AISquad> AISquad::Create(const std::string &name) { return std::shared_ptr<AISquad>(new AISquad(name)); }

const std::vector<std::shared_ptr<AISquad>> &AISquad::GetAll()
{
	for(auto it = s_squads.begin(); it != s_squads.end();) {
		auto &squad = *it;
		if(squad.use_count() == 1)
			it = s_squads.erase(it);
		else
			++it;
	}
	return s_squads;
}

AISquad::AISquad(const std::string &squadName) : std::enable_shared_from_this<AISquad>(), name(squadName) { s_squads.push_back(shared_from_this()); }

AISquad::~AISquad()
{
	auto it = std::find_if(s_squads.begin(), s_squads.end(), [this](const std::shared_ptr<AISquad> &squad) { return (squad.get() == this) ? true : false; });
	if(it != s_squads.end())
		s_squads.erase(it);
}

void AISquad::Join(pragma::ecs::BaseEntity *ent)
{
	auto it = std::find_if(members.begin(), members.end(), [ent](const EntityHandle &hEnt) { return (hEnt.get() == ent) ? true : false; });
	if(it != members.end())
		return;
	members.push_back(ent->GetHandle());
}
void AISquad::Leave(pragma::ecs::BaseEntity *ent)
{
	auto it = std::find_if(members.begin(), members.end(), [ent](const EntityHandle &hEnt) { return (hEnt.get() == ent) ? true : false; });
	if(it == members.end())
		return;
	members.erase(it);
}
