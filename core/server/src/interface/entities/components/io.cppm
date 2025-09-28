// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.entities.components.io;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SIOComponent final : public BaseIOComponent {
	  public:
		SIOComponent(BaseEntity &ent) : BaseIOComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
