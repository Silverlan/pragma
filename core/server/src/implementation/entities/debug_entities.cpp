// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.debug;

import :entities.components;

using namespace pragma;

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
