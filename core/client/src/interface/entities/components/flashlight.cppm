// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"




export module pragma.client:entities.components.flashlight;

import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CFlashlightComponent final : public BaseFlashlightComponent {
		public:
			CFlashlightComponent(pragma::ecs::BaseEntity &ent) : BaseFlashlightComponent(ent) {}
			virtual void Initialize() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLCLIENT CFlashlight : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
