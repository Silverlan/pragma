// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/func/basefuncportal.h"

export module pragma.server.entities.components.func.portal;

export {
	namespace pragma {
		class DLLSERVER SFuncPortalComponent final : public BaseFuncPortalComponent {
		public:
			SFuncPortalComponent(BaseEntity &ent) : BaseFuncPortalComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FuncPortal : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
