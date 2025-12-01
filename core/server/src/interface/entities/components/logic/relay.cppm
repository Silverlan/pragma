// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.server:entities.components.logic.relay;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SLogicRelayComponent final : public BaseLogicRelayComponent {
		  public:
			SLogicRelayComponent(pragma::ecs::BaseEntity &ent) : BaseLogicRelayComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			void Trigger(pragma::ecs::BaseEntity *activator);
		};
	};

	class DLLSERVER LogicRelay : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
