// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.bot;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CBotComponent final : public BaseBotComponent {
		  public:
			CBotComponent(pragma::ecs::BaseEntity &ent) : BaseBotComponent(ent) {}
			virtual void Initialize() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			void OnFootStep(BaseCharacterComponent::FootType foot);
		};
	};

	class DLLCLIENT CBot : public CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
