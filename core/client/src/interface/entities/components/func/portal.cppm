// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/func/basefuncportal.h"

import pragma.entities.components;

export module pragma.client.entities.components.func_portal;

export {
	namespace pragma {
		class DLLCLIENT CFuncPortalComponent final : public BaseFuncPortalComponent {
		public:
			CFuncPortalComponent(BaseEntity &ent) : BaseFuncPortalComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLCLIENT CFuncPortal : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
