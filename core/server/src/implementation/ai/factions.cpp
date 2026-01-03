// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.factions;

Faction::Faction(const std::string &name) : std::enable_shared_from_this<Faction>(), m_name(name), m_defaultDisp(DISPOSITION::NEUTRAL) {}
void Faction::AddClass(std::string className)
{
	pragma::string::to_lower(className);
	if(HasClass(className))
		return;
	m_classes.push_back(className);
}
const std::string &Faction::GetName() const { return m_name; }
std::vector<std::string> &Faction::GetClasses() { return m_classes; }
void Faction::SetDisposition(Faction &faction, DISPOSITION disp, bool revert, int priority)
{
	if(revert == true)
		faction.SetDisposition(*this, disp, false, priority);
	for(char i = 0; i < 4; i++) {
		auto &disps = m_relationships[i];
		for(auto it = disps.begin(); it != disps.end();) {
			auto &fdisp = *it;
			if(fdisp->target == faction) {
				fdisp->priority = priority;
				if(static_cast<DISPOSITION>(i) == disp)
					return;
				disps.erase(it);
				m_relationships[static_cast<int>(disp)].push_back(fdisp);
				return;
			}
			++it;
		}
	}
	m_relationships[static_cast<int>(disp)].push_back(pragma::util::make_shared<FactionDisposition>(faction, priority));
}
void Faction::SetEnemyFaction(Faction &faction, bool revert, int priority) { SetDisposition(faction, DISPOSITION::HATE, revert, priority); }
void Faction::SetAlliedFaction(Faction &faction, bool revert, int priority) { SetDisposition(faction, DISPOSITION::LIKE, revert, priority); }
void Faction::SetNeutralFaction(Faction &faction, bool revert, int priority) { SetDisposition(faction, DISPOSITION::NEUTRAL, revert, priority); }
void Faction::SetFearsomeFaction(Faction &faction, bool revert, int priority) { SetDisposition(faction, DISPOSITION::FEAR, revert, priority); }
DISPOSITION Faction::GetDisposition(Faction &faction, int *priority)
{
	for(char i = 0; i < 4; i++) {
		auto &disps = m_relationships[i];
		for(unsigned int j = 0; j < disps.size(); j++) {
			auto &disp = disps[j];
			if(disp->target == faction) {
				if(priority != nullptr)
					*priority = disp->priority;
				return DISPOSITION(i);
			}
		}
	}
	if(priority != nullptr)
		*priority = 0;
	if(&faction == this) // Faction is allied to itself by default (Unless overwritten)
		return DISPOSITION::LIKE;
	return GetDefaultDisposition();
}
DISPOSITION Faction::GetDisposition(std::string className, int *priority)
{
	pragma::string::to_lower(className);
	if(HasClass(className))
		return DISPOSITION::LIKE;
	for(char i = 0; i < 4; i++) {
		auto &disps = m_relationships[i];
		for(unsigned int j = 0; j < disps.size(); j++) {
			auto &disp = disps[j];
			if(disp->target.HasClass(className)) {
				if(priority != nullptr)
					*priority = disp->priority;
				return DISPOSITION(i);
			}
		}
	}
	if(priority != nullptr)
		*priority = 0;
	return GetDefaultDisposition();
}
DISPOSITION Faction::GetDisposition(EntityHandle &hEnt, int *priority)
{
	if(!hEnt.valid()) {
		if(priority != nullptr)
			*priority = 0;
		return GetDefaultDisposition();
	}
	return GetDisposition(hEnt->GetClass(), priority);
}
DISPOSITION Faction::GetDisposition(pragma::ecs::BaseEntity *ent, int *priority)
{
	if(ent == nullptr) {
		if(priority != nullptr)
			*priority = 0;
		return GetDefaultDisposition();
	}
	return GetDisposition(ent->GetClass(), priority);
}
bool Faction::HasClass(std::string className)
{
	for(unsigned int i = 0; i < m_classes.size(); i++) {
		if(m_classes[i] == className)
			return true;
	}
	return false;
}
void Faction::SetDefaultDisposition(DISPOSITION disp) { m_defaultDisp = disp; }
DISPOSITION Faction::GetDefaultDisposition() { return m_defaultDisp; }
bool Faction::operator==(Faction &other) { return this == &other; }

//////////////////////////////

FactionManager::FactionManager() {}

std::shared_ptr<Faction> FactionManager::RegisterFaction(const std::string &name)
{
	auto lname = name;
	pragma::string::to_lower(lname);
	auto it = std::find_if(m_factions.begin(), m_factions.end(), [&lname](const std::shared_ptr<Faction> &faction) { return (faction->GetName() == lname) ? true : false; });
	if(it != m_factions.end())
		return *it;
	m_factions.push_back(std::shared_ptr<Faction>(new Faction(lname)));
	return m_factions.back();
}
const std::vector<std::shared_ptr<Faction>> &FactionManager::GetFactions() { return m_factions; }
std::shared_ptr<Faction> FactionManager::FindFactionByName(const std::string &name)
{
	auto lname = name;
	pragma::string::to_lower(lname);
	auto it = std::find_if(m_factions.begin(), m_factions.end(), [&lname](const std::shared_ptr<Faction> &faction) { return (faction->GetName() == lname) ? true : false; });
	return (it != m_factions.end()) ? *it : nullptr;
}
