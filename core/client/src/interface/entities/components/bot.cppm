// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/basebot.h"
#include <pragma/entities/components/base_character_component.hpp>

export module pragma.client.entities.components.bot;

export {
	namespace pragma {
		class DLLCLIENT CBotComponent final : public BaseBotComponent {
		public:
			CBotComponent(BaseEntity &ent) : BaseBotComponent(ent) {}
			virtual void Initialize() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
			virtual void InitializeLuaObject(lua_State *l) override;
		protected:
			void OnFootStep(BaseCharacterComponent::FootType foot);
		};
	};

	class DLLCLIENT CBot : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
