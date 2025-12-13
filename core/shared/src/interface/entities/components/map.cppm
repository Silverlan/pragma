// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.map;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK MapComponent final : public BaseEntityComponent {
	  public:
		MapComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		void SetMapIndex(unsigned int idx);
		unsigned int GetMapIndex() const;

		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		uint32_t m_mapIndex = 0u;
	};
};
