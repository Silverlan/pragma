// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities;

import :debug;
import pragma.client.entities.components;

using namespace pragma;

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
