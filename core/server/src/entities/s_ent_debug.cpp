/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/s_ent_debug.hpp"
#include "pragma/entities/components/s_debug_component.hpp"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(debug_text, EntDebugText);
LINK_ENTITY_TO_CLASS(debug_point, EntDebugPoint);
LINK_ENTITY_TO_CLASS(debug_line, EntDebugLine);
LINK_ENTITY_TO_CLASS(debug_box, EntDebugBox);
LINK_ENTITY_TO_CLASS(debug_sphere, EntDebugSphere);
LINK_ENTITY_TO_CLASS(debug_cone, EntDebugCone);
LINK_ENTITY_TO_CLASS(debug_cylinder, EntDebugCylinder);
LINK_ENTITY_TO_CLASS(debug_plane, EntDebugPlane);

void EntDebugText::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugTextComponent>();
}

////////////////

void EntDebugPoint::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugPointComponent>();
}

////////////////

void EntDebugLine::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugLineComponent>();
}

////////////////

void EntDebugBox::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugBoxComponent>();
}

////////////////

void EntDebugSphere::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugSphereComponent>();
}

////////////////

void EntDebugCone::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugConeComponent>();
}

////////////////

void EntDebugCylinder::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugCylinderComponent>();
}

////////////////

void EntDebugPlane::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDebugPlaneComponent>();
}
