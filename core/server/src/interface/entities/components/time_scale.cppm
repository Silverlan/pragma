// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.entities.components.time_scale;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER STimeScaleComponent final : public BaseTimeScaleComponent {
	  public:
		STimeScaleComponent(BaseEntity &ent) : BaseTimeScaleComponent(ent) {}
		virtual void Initialize() override;
		virtual void SetTimeScale(float timeScale) override;

		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
