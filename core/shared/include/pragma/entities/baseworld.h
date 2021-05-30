/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEWORLD_H__
#define __BASEWORLD_H__

#include "pragma/entities/components/base_entity_component.hpp"

#define WORLD_BASE_LOD 20
#define WORLD_LOD_OFFSET 4

class PolyMesh;
#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma
{
	class DLLNETWORK BaseWorldComponent
		: public BaseEntityComponent
	{
	public:
		virtual Con::c_cout &print(Con::c_cout&);
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
