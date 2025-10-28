// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.point_constraint_slider;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintSliderComponent final : public BasePointConstraintSliderComponent, public CBaseNetComponent {
		public:
			CPointConstraintSliderComponent(BaseEntity &ent) : BasePointConstraintSliderComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintSlider : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
