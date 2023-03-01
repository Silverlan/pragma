/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
