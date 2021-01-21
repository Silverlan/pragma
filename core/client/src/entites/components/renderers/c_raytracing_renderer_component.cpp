/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_raytracing_renderer_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

LINK_ENTITY_TO_CLASS(raytracing_renderer,CRaytracingRenderer);

luabind::object CRaytracingRendererComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CRaytracingRendererComponentHandleWrapper>(l);}

////////

void CRaytracingRenderer::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRaytracingRendererComponent>();
}
