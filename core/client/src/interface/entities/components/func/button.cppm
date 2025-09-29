// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:entities.components.func_button;

import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CButtonComponent final : public BaseFuncButtonComponent {
		public:
			CButtonComponent(BaseEntity &ent) : BaseFuncButtonComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLCLIENT CFuncButton : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
