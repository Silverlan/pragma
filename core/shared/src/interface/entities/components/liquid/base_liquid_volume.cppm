// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.liquid.base_volume;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseLiquidVolumeComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
	  protected:
		BaseLiquidVolumeComponent(ecs::BaseEntity &ent);
	};
};
