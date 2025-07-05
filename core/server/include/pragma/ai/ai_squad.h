// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __AI_SQUAD_H__
#define __AI_SQUAD_H__

#include "pragma/serverdefinitions.h"
#include <string>
#include <vector>
#include <memory>

class BaseEntity;
class DLLSERVER AISquad : public std::enable_shared_from_this<AISquad> {
  private:
	static std::vector<std::shared_ptr<AISquad>> s_squads;
  protected:
	AISquad(const std::string &name);
  public:
	static const std::vector<std::shared_ptr<AISquad>> &GetAll();
  public:
	static std::shared_ptr<AISquad> Create(const std::string &name);
	~AISquad();
	std::string name;
	std::vector<EntityHandle> members;
	void Join(BaseEntity *ent);
	void Leave(BaseEntity *ent);
};

#endif
