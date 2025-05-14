/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_ent_debug.hpp"
#include "pragma/entities/components/c_debug_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(debug_text, CEntDebugText);
LINK_ENTITY_TO_CLASS(debug_point, CEntDebugPoint);
LINK_ENTITY_TO_CLASS(debug_line, CEntDebugLine);
LINK_ENTITY_TO_CLASS(debug_box, CEntDebugBox);
LINK_ENTITY_TO_CLASS(debug_sphere, CEntDebugSphere);
LINK_ENTITY_TO_CLASS(debug_cone, CEntDebugCone);
LINK_ENTITY_TO_CLASS(debug_cylinder, CEntDebugCylinder);
LINK_ENTITY_TO_CLASS(debug_plane, CEntDebugPlane);

void CEntDebugText::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugTextComponent>();
}

////////////////

void CEntDebugPoint::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugPointComponent>();
}

////////////////

void CEntDebugLine::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugLineComponent>();
}

////////////////

void CEntDebugBox::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugBoxComponent>();
}

////////////////

void CEntDebugSphere::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugSphereComponent>();
}

////////////////

void CEntDebugCone::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugConeComponent>();
}

////////////////

void CEntDebugCylinder::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugCylinderComponent>();
}

////////////////

void CEntDebugPlane::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CDebugPlaneComponent>();
}
