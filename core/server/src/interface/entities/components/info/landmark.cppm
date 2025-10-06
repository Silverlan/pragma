// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.server.entities.components.info.landmark;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SInfoLandmarkComponent final : public BaseInfoLandmarkComponent {
		public:
			SInfoLandmarkComponent(BaseEntity &ent) : BaseInfoLandmarkComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER InfoLandmark : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
