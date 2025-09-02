// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"

export module pragma.entities.components.global_name;

export namespace pragma {
	// Note: 'GlobalComponent' cannot be used as name, otherwise there are conflicts with 'GlobalHandle' class of winbase.h
	class DLLNETWORK GlobalNameComponent final : public BaseEntityComponent {
	  public:
		GlobalNameComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		const std::string &GetGlobalName() const;
		void SetGlobalName(const std::string &name);

		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		std::string m_globalName = {};
	};
};
