// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.squad;

export import pragma.shared;

export class DLLSERVER AISquad : public std::enable_shared_from_this<AISquad> {
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
	void Join(pragma::ecs::BaseEntity *ent);
	void Leave(pragma::ecs::BaseEntity *ent);
};
