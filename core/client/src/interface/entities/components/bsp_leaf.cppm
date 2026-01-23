// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.bsp_leaf;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CBSPLeafComponent final : public BaseEntityComponent {
	  public:
		CBSPLeafComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;

		void SetLeaves(const std::vector<uint16_t> &leaves);
		bool GetLeafVisibility(uint16_t leafIdx) const;
	  private:
		std::unordered_set<uint16_t> m_leaves;
	};
};
