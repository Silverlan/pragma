// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_world;

import :console.output;
export import :entities.components.base;

#define WORLD_BASE_LOD 20
#define WORLD_LOD_OFFSET 4

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma {
	class DLLNETWORK BaseWorldComponent : public BaseEntityComponent {
	  public:
		virtual Con::c_cout &print(Con::c_cout &);
	  public:
		BaseWorldComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	  protected:
		void InitializePhysics();
	};
};
#pragma warning(pop)
