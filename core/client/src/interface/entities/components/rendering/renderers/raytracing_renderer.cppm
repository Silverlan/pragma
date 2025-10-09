// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <unordered_set>
#include "pragma/lua/luaapi.h"


export module pragma.client:entities.components.raytracing_renderer;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CRaytracingRendererComponent final : public BaseEntityComponent {
		public:
			CRaytracingRendererComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		private:
		};
	};

	class DLLCLIENT CRaytracingRenderer : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
