// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASEWORLD_H__
#define __BASEWORLD_H__

#include "pragma/entities/components/base_entity_component.hpp"

#define WORLD_BASE_LOD 20
#define WORLD_LOD_OFFSET 4

class PolyMesh;
#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma {
	class DLLNETWORK BaseWorldComponent : public BaseEntityComponent {
	  public:
		virtual Con::c_cout &print(Con::c_cout &);
	  public:
		BaseWorldComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	  protected:
		void InitializePhysics();
	};
};
#pragma warning(pop)

#endif
