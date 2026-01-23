// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.global_name;

export import :entities.components.base;

export namespace pragma {
	// Note: 'GlobalComponent' cannot be used as name, otherwise there are conflicts with 'GlobalHandle' class of winbase.h
	class DLLNETWORK GlobalNameComponent final : public BaseEntityComponent {
	  public:
		GlobalNameComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		const std::string &GetGlobalName() const;
		void SetGlobalName(const std::string &name);

		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		std::string m_globalName = {};
	};
};
