// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_TIME_SCALE_COMPONENT_HPP__
#define __S_TIME_SCALE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_time_scale_component.hpp>

namespace pragma {
	class DLLSERVER STimeScaleComponent final : public BaseTimeScaleComponent {
	  public:
		STimeScaleComponent(BaseEntity &ent) : BaseTimeScaleComponent(ent) {}
		virtual void Initialize() override;
		virtual void SetTimeScale(float timeScale) override;

		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
