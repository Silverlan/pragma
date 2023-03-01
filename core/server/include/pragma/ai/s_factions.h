/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_FACTIONS_H__
#define __S_FACTIONS_H__

#include "pragma/networkdefinitions.h"
#include "pragma/serverdefinitions.h"
#include <string>
#include <vector>

enum class DISPOSITION : uint32_t;
class BaseEntity;
class FactionManager;
struct FactionDisposition;
class DLLSERVER Faction : public std::enable_shared_from_this<Faction> {
  protected:
	friend FactionManager;
	DISPOSITION m_defaultDisp;
	Faction(const std::string &name);
	std::string m_name;
	std::vector<std::string> m_classes;
	std::array<std::vector<std::shared_ptr<FactionDisposition>>, 4> m_relationships;
  public:
	void AddClass(std::string className);
	std::vector<std::string> &GetClasses();
	void SetDisposition(Faction &faction, DISPOSITION disp, bool revert = false, int priority = 0);
	void SetEnemyFaction(Faction &faction, bool revert = false, int priority = 0);
	void SetAlliedFaction(Faction &faction, bool revert = false, int priority = 0);
	void SetNeutralFaction(Faction &faction, bool revert = false, int priority = 0);
	void SetFearsomeFaction(Faction &faction, bool revert = false, int priority = 0);
	DISPOSITION GetDisposition(Faction &faction, int *priority = nullptr);
	DISPOSITION GetDisposition(std::string className, int *priority = nullptr);
	DISPOSITION GetDisposition(EntityHandle &hEnt, int *priority = nullptr);
	DISPOSITION GetDisposition(BaseEntity *ent, int *priority = nullptr);
	bool HasClass(std::string className);
	void SetDefaultDisposition(DISPOSITION disp);
	DISPOSITION GetDefaultDisposition();
	const std::string &GetName() const;
	bool operator==(Faction &other);
};

struct DLLSERVER FactionDisposition {
	FactionDisposition(Faction &faction, int prio = 0) : target(faction), priority(prio) {}
	Faction &target;
	int priority;
	FactionDisposition &operator=(const FactionDisposition &other) = delete;
};

class DLLSERVER FactionManager {
  protected:
	std::vector<std::shared_ptr<Faction>> m_factions;
  public:
	FactionManager();
	std::shared_ptr<Faction> RegisterFaction(const std::string &name);
	const std::vector<std::shared_ptr<Faction>> &GetFactions();
	std::shared_ptr<Faction> FindFactionByName(const std::string &name);
};

#endif
