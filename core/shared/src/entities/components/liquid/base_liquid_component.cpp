/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/liquid/base_liquid_component.hpp"
#include "pragma/entities/components/liquid/base_liquid_surface_simulation_component.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/phys_water_buoyancy_simulator.hpp"
#include "pragma/physics/phys_water_surface_simulator.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/entities/trigger/trigger_spawnflags.h"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/physics/raytraces.h"
#include "pragma/physics/phys_water_buoyancy_simulator.hpp"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_surface_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/model/model.h"

// #define ENABLE_DEPRECATED_PHYSICS

using namespace pragma;

void BaseFuncLiquidComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}
void BaseFuncLiquidComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BasePhysicsComponent::EVENT_HANDLE_RAYCAST, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &raycastData = static_cast<CEHandleRaycast &>(evData.get());
		auto r = OnRayResultCallback(raycastData.rayCollisionGroup, raycastData.rayCollisionMask);
		if(r == false) {
			raycastData.hit = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("name");
	ent.AddComponent("render");
	ent.AddComponent("liquid_control");
	ent.AddComponent("liquid_volume");
	ent.AddComponent("liquid_surface");
	ent.AddComponent("buoyancy");
	m_surfaceC = ent.AddComponent("surface")->GetHandle<BaseSurfaceComponent>();
	m_surfSim = ent.AddComponent("liquid_surface_simulation")->GetHandle<BaseLiquidSurfaceSimulationComponent>();
	ent.AddComponent("model");
}

void BaseFuncLiquidComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }

void BaseFuncLiquidComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pRenderComponent = dynamic_cast<pragma::BaseRenderComponent *>(&component);
	auto *pSurfC = dynamic_cast<pragma::BaseSurfaceComponent *>(&component);
	auto *pSurfSim = dynamic_cast<pragma::BaseLiquidSurfaceSimulationComponent *>(&component);
	if(pRenderComponent != nullptr)
		pRenderComponent->SetCastShadows(false);
	else if(pSurfC)
		m_surfaceC = pSurfC->GetHandle<BaseSurfaceComponent>();
	else if(pSurfSim)
		m_surfSim = pSurfSim->GetHandle<BaseLiquidSurfaceSimulationComponent>();
}

bool BaseFuncLiquidComponent::OnRayResultCallback(CollisionMask rayCollisionGroup, CollisionMask rayCollisionMask)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	if(m_physSurfaceSim == nullptr || (rayCollisionMask & CollisionMask::WaterSurface) == CollisionMask::None)
		return true;
	auto rayFromWorldWs = uvec::create(rayFromWorld / PhysEnv::WORLD_SCALE);
	auto rayToWorldWs = uvec::create(rayToWorld / PhysEnv::WORLD_SCALE);
	auto dir = rayToWorldWs - rayFromWorldWs;
	auto dist = uvec::length(dir);
	if(dist > 0.f)
		dir /= dist;
	auto t = 0.0;
	auto bIntersect = CalcLineSurfaceIntersection(rayFromWorldWs, dir, &t);
	if(bIntersect == false || t > dist || t < 0.0)
		return false;
	hitNormalWorld = uvec::create_bt(m_waterPlane.GetNormal());
	rayResult.m_hitFraction = t / dist;
	rayResult.m_hitNormalLocal = hitNormalWorld;
	hitPointWorld = uvec::create_bt(rayFromWorldWs + dir * static_cast<float>(t)) * PhysEnv::WORLD_SCALE;
	return true;
#else
	return false;
#endif
}

void BaseFuncLiquidComponent::InitializeWaterSurface()
{
	ClearWaterSurface();
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return;
	if(m_surfaceC.expired())
		return;
	m_surfaceC->FindAndAssignMesh([](ModelMesh &mesh, ModelSubMesh &subMesh, Material &mat, const std::string &shader) -> uint32_t { return (shader == "water") ? 1 : 0; });
}

void BaseFuncLiquidComponent::ClearWaterSurface()
{
	if(m_surfaceC.valid())
		m_surfaceC->Clear();
	if(m_surfSim.valid())
		m_surfSim->ClearSurfaceSimulator();
}

bool BaseFuncLiquidComponent::CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT, double *outU, double *outV, bool bCull) const
{
	double t, u, v;
	auto *surfSim = m_surfSim.valid() ? m_surfSim->GetSurfaceSimulator() : nullptr;
	if(surfSim != nullptr)
		return m_surfSim->CalcLineSurfaceIntersection(lineOrigin, lineDir, outT, outU, outV, bCull);
	if(m_surfaceC.expired())
		return false;
	return m_surfaceC->CalcLineSurfaceIntersection(lineOrigin, lineDir, outT);
}
