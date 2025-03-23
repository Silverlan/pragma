/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/liquid/base_liquid_control_component.hpp"
#include "pragma/entities/components/base_surface_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <material.h>

using namespace pragma;

ComponentEventId BaseLiquidControlComponent::EVENT_ON_SPLASH = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseLiquidControlComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_SPLASH = registerEvent("ON_SPLASH", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_PROPERTIES_CHANGED = registerEvent("ON_PROPERTIES_CHANGED", ComponentEventInfo::Type::Broadcast);
}

void BaseLiquidControlComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}

BaseLiquidControlComponent::BaseLiquidControlComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidControlComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvCreateSplash = SetupNetEvent("create_splash");
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "surface_material", false)) {
			if(ustring::compare<std::string>(kvData.value, "default", false) == false) {
				m_kvSurfaceMaterial = kvData.value;
				SetSurfaceMaterial(kvData.value);
			}
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(BaseSurfaceComponent::EVENT_ON_SURFACE_MESH_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &data = static_cast<CEOnSurfaceMeshChanged &>(evData.get());
		if(data.meshInfo.subMesh && m_kvSurfaceMaterial.empty() == true) {
			std::string surfaceMatIdentifier;
			if(data.meshInfo.material->GetProperty("surfacematerial", &surfaceMatIdentifier) == true)
				SetSurfaceMaterial(surfaceMatIdentifier);
		}
	});
}

void BaseLiquidControlComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }

void BaseLiquidControlComponent::SetSurfaceMaterial(const std::string &mat)
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	SetSurfaceMaterial(game->GetSurfaceMaterial(mat));
}

void BaseLiquidControlComponent::SetSurfaceMaterial(const SurfaceMaterial *mat)
{
	if(mat == nullptr)
		return;
	SetDensity(mat->GetDensity());
	SetLinearDragCoefficient(mat->GetLinearDragCoefficient());
	SetTorqueDragCoefficient(mat->GetTorqueDragCoefficient());
	SetStiffness(mat->GetWaveStiffness());
	SetPropagation(mat->GetWavePropagation());
}

const PhysLiquid &BaseLiquidControlComponent::GetLiquidDescription() const { return const_cast<BaseLiquidControlComponent *>(this)->GetLiquidDescription(); }
PhysLiquid &BaseLiquidControlComponent::GetLiquidDescription() { return m_liquid; }

double BaseLiquidControlComponent::GetDensity() const { return m_liquid.density; }
void BaseLiquidControlComponent::SetDensity(double density)
{
	m_liquid.density = density;

	BroadcastEvent(EVENT_ON_PROPERTIES_CHANGED);
}

double BaseLiquidControlComponent::GetLinearDragCoefficient() const { return m_liquid.linearDragCoefficient; }
void BaseLiquidControlComponent::SetLinearDragCoefficient(double coefficient)
{
	m_liquid.linearDragCoefficient = coefficient;
	BroadcastEvent(EVENT_ON_PROPERTIES_CHANGED);
}

double BaseLiquidControlComponent::GetTorqueDragCoefficient() const { return m_liquid.torqueDragCoefficient; }
void BaseLiquidControlComponent::SetTorqueDragCoefficient(double coefficient)
{
	m_liquid.torqueDragCoefficient = coefficient;
	BroadcastEvent(EVENT_ON_PROPERTIES_CHANGED);
}

float BaseLiquidControlComponent::GetStiffness() const { return m_liquid.stiffness; }
void BaseLiquidControlComponent::SetStiffness(float stiffness)
{
	m_liquid.stiffness = stiffness;
	BroadcastEvent(EVENT_ON_PROPERTIES_CHANGED);
}
float BaseLiquidControlComponent::GetPropagation() const { return m_liquid.propagation; }
void BaseLiquidControlComponent::SetPropagation(float propagation)
{
	m_liquid.propagation = propagation;
	BroadcastEvent(EVENT_ON_PROPERTIES_CHANGED);
}

const Vector3 &BaseLiquidControlComponent::GetLiquidVelocity() const { return m_liquidVelocity; }
void BaseLiquidControlComponent::SetLiquidVelocity(const Vector3 &velocity) { m_liquidVelocity = velocity; }

void BaseLiquidControlComponent::CreateSplash(const Vector3 &origin, float radius, float force)
{
	m_splashes.push({});
	auto &splashInfo = m_splashes.back();
	splashInfo.origin = origin;
	splashInfo.radius = radius;
	splashInfo.force = force;

	BroadcastEvent(EVENT_ON_SPLASH, CEOnSplash {splashInfo});
}

bool BaseLiquidControlComponent::OnBulletHit(const BulletInfo &bulletInfo, const TraceData &data, PhysObj *phys, pragma::physics::ICollisionObject *col, const LocalRayResult &result)
{
	/*if(m_physSurfaceSim != nullptr)
	{
		static auto radius = 10.f;
		static auto force = 2.f;
		auto srcOrigin = data.GetSourceOrigin();
		auto dir = data.GetDirection();
		auto dist = data.GetDistance();
		auto hitPos = srcOrigin +dir *(dist *result.fraction);
		CreateSplash(hitPos,radius,force);
	}*/
	return false;
}

/////////

CEOnSplash::CEOnSplash(const BaseLiquidControlComponent::SplashInfo &splashInfo) : splashInfo {splashInfo} {}
void CEOnSplash::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l, splashInfo.origin);
	Lua::PushNumber(l, splashInfo.radius);
	Lua::PushNumber(l, splashInfo.force);
}
