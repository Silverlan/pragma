/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_RAYTRACING_RENDERER_COMPONENT_HPP__
#define __C_RAYTRACING_RENDERER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <unordered_set>

namespace pragma
{
	class DLLCLIENT CRaytracingRendererComponent final
		: public BaseEntityComponent
	{
	public:
		CRaytracingRendererComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	private:
	};
};

class EntityHandle;
class DLLCLIENT CRaytracingRenderer
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
