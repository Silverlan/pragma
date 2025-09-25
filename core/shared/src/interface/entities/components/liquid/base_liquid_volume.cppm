// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/plane.hpp>

export module pragma.shared:entities.components.liquid.base_volume;

export namespace pragma {
	class DLLNETWORK BaseLiquidVolumeComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
	  protected:
		BaseLiquidVolumeComponent(BaseEntity &ent);
	};
};
