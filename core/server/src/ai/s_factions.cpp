/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/s_factions.h"
#include "pragma/ai/s_disposition.h"
#include <pragma/entities/baseentity.h>
#include <pragma/entities/baseentity_handle.h>
#include <algorithm>

Faction::Faction(const std::string &name) : std::enable_shared_from_this<Faction>(), m_name(name), m_defaultDisp(DISPOSITION::NEUTRAL) {}
void Faction::AddClass(std::string className)
{
	std::transform(className.begin(), className.end(), className.begin(), ::tolower);
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
	m_relationships[static_cast<int>(disp)].push_back(std::make_shared<FactionDisposition>(faction, priority));
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
				if(priority != NULL)
					*priority = disp->priority;
				return DISPOSITION(i);
			}
		}
	}
	if(priority != NULL)
		*priority = 0;
	if(&faction == this) // Faction is allied to itself by default (Unless overwritten)
		return DISPOSITION::LIKE;
	return GetDefaultDisposition();
}
DISPOSITION Faction::GetDisposition(std::string className, int *priority)
{
	std::transform(className.begin(), className.end(), className.begin(), ::tolower);
	if(HasClass(className))
		return DISPOSITION::LIKE;
	for(char i = 0; i < 4; i++) {
		auto &disps = m_relationships[i];
		for(unsigned int j = 0; j < disps.size(); j++) {
			auto &disp = disps[j];
			if(disp->target.HasClass(className)) {
				if(priority != NULL)
					*priority = disp->priority;
				return DISPOSITION(i);
			}
		}
	}
	if(priority != NULL)
		*priority = 0;
	return GetDefaultDisposition();
}
DISPOSITION Faction::GetDisposition(EntityHandle &hEnt, int *priority)
{
	if(!hEnt.valid()) {
		if(priority != NULL)
			*priority = 0;
		return GetDefaultDisposition();
	}
	return GetDisposition(hEnt->GetClass(), priority);
}
DISPOSITION Faction::GetDisposition(BaseEntity *ent, int *priority)
{
	if(ent == NULL) {
		if(priority != NULL)
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
	ustring::to_lower(lname);
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
	ustring::to_lower(lname);
	auto it = std::find_if(m_factions.begin(), m_factions.end(), [&lname](const std::shared_ptr<Faction> &faction) { return (faction->GetName() == lname) ? true : false; });
	return (it != m_factions.end()) ? *it : nullptr;
}
