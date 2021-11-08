/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/liquid/base_liquid_component.hpp"
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
#pragma optimize("",off)
ComponentEventId BaseFuncLiquidComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseFuncLiquidComponent::RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent)
{
	EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED = registerEvent("ON_WATER_SURFACE_SIMULATOR_CHANGED",EntityComponentManager::EventInfo::Type::Broadcast);
}
void BaseFuncLiquidComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvCreateSplash = SetupNetEvent("create_splash");
	m_netEvSetWaterPlane = SetupNetEvent("set_water_plane");

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"surface_material",false))
		{
			if(ustring::compare<std::string>(kvData.value,"default",false) == false)
			{
				m_kvSurfaceMaterial = kvData.value;
				SetSurfaceMaterial(kvData.value);
			}
		}
		else if(ustring::compare<std::string>(kvData.key,"max_wave_height",false))
			SetMaxWaveHeight(ustring::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnPhysicsInitialized();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		SimulateBuoyancy();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		static_cast<CEPostPhysicsSimulate&>(evData.get()).keepAwake = true;
	});
	BindEvent(BasePhysicsComponent::EVENT_HANDLE_RAYCAST,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &raycastData = static_cast<CEHandleRaycast&>(evData.get());
		auto r = OnRayResultCallback(raycastData.rayCollisionGroup,raycastData.rayCollisionMask);
		if(r == false)
		{
			raycastData.hit = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("name");
	ent.AddComponent("render");
	m_surfaceC = ent.AddComponent("surface")->GetHandle<BaseSurfaceComponent>();
	auto touchC = ent.AddComponent("touch");
	static_cast<BaseTouchComponent*>(touchC.get())->SetNeverDisablePhysicsCallbacks(true);
	ent.AddComponent("model");
}

void BaseFuncLiquidComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
}

void BaseFuncLiquidComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pRenderComponent = dynamic_cast<pragma::BaseRenderComponent*>(&component);
	auto *pPhysicsComponent = dynamic_cast<pragma::BasePhysicsComponent*>(&component);
	auto *pSurfC = dynamic_cast<pragma::BaseSurfaceComponent*>(&component);
	if(pRenderComponent != nullptr)
		pRenderComponent->SetCastShadows(false);
	else if(pPhysicsComponent != nullptr)
		pPhysicsComponent->SetRayResultCallbackEnabled(true);
	else if(pSurfC)
		m_surfaceC = pSurfC->GetHandle<BaseSurfaceComponent>();
}

util::EventReply BaseFuncLiquidComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseTouchComponent::EVENT_CAN_TRIGGER)
	{
		auto &triggerData = static_cast<CECanTriggerData&>(evData);
		if(triggerData.entity != nullptr)
		{
			auto pPhysComponent = triggerData.entity->GetPhysicsComponent();
			if(pPhysComponent != nullptr && (pPhysComponent->GetCollisionFilterMask() &CollisionMask::Water) == CollisionMask::None)
				triggerData.canTrigger = false;
		}
		return util::EventReply::Handled;
	}
	return util::EventReply::Unhandled;
}

void BaseFuncLiquidComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	pPhysComponent->SetCollisionFilterMask(CollisionMask::Dynamic | CollisionMask::Generic);
	pPhysComponent->SetCollisionFilterGroup(CollisionMask::Water | CollisionMask::WaterSurface);
	pPhysComponent->SetForcePhysicsAwakeCallbacksEnabled(true);
}

bool BaseFuncLiquidComponent::OnRayResultCallback(CollisionMask rayCollisionGroup,CollisionMask rayCollisionMask)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	if(m_physSurfaceSim == nullptr || (rayCollisionMask &CollisionMask::WaterSurface) == CollisionMask::None)
		return true;
	auto rayFromWorldWs = uvec::create(rayFromWorld /PhysEnv::WORLD_SCALE);
	auto rayToWorldWs = uvec::create(rayToWorld /PhysEnv::WORLD_SCALE);
	auto dir = rayToWorldWs -rayFromWorldWs;
	auto dist = uvec::length(dir);
	if(dist > 0.f)
		dir /= dist;
	auto t = 0.0;
	auto bIntersect = CalcLineSurfaceIntersection(rayFromWorldWs,dir,&t);
	if(bIntersect == false || t > dist || t < 0.0)
		return false;
	hitNormalWorld = uvec::create_bt(m_waterPlane.GetNormal());
	rayResult.m_hitFraction = t /dist;
	rayResult.m_hitNormalLocal = hitNormalWorld;
	hitPointWorld = uvec::create_bt(rayFromWorldWs +dir *static_cast<float>(t)) *PhysEnv::WORLD_SCALE;
	return true;
#else
	return false;
#endif
}

void BaseFuncLiquidComponent::SetMaxWaveHeight(float height) {m_kvMaxWaveHeight = height;}

const PhysWaterSurfaceSimulator *BaseFuncLiquidComponent::GetSurfaceSimulator() const {return const_cast<BaseFuncLiquidComponent*>(this)->GetSurfaceSimulator();}
PhysWaterSurfaceSimulator *BaseFuncLiquidComponent::GetSurfaceSimulator() {return m_physSurfaceSim.get();}

bool BaseFuncLiquidComponent::OnBulletHit(const BulletInfo &bulletInfo,const TraceData &data,PhysObj *phys,pragma::physics::ICollisionObject *col,const LocalRayResult &result)
{
	if(m_physSurfaceSim != nullptr)
	{
		static auto radius = 10.f;
		static auto force = 2.f;
		auto srcOrigin = data.GetSourceOrigin();
		auto dir = data.GetDirection();
		auto dist = data.GetDistance();
		auto hitPos = srcOrigin +dir *(dist *result.fraction);
		CreateSplash(hitPos,radius,force);
	}
	return false;
}

const Vector3 &BaseFuncLiquidComponent::GetWaterVelocity() const {return m_waterVelocity;}
void BaseFuncLiquidComponent::SetWaterVelocity(const Vector3 &velocity) {m_waterVelocity = velocity;}

const PhysLiquid &BaseFuncLiquidComponent::GetLiquidDescription() const {return const_cast<BaseFuncLiquidComponent*>(this)->GetLiquidDescription();}
PhysLiquid &BaseFuncLiquidComponent::GetLiquidDescription() {return m_liquid;}

double BaseFuncLiquidComponent::GetDensity() const {return m_liquid.density;}
void BaseFuncLiquidComponent::SetDensity(double density) {m_liquid.density = density;}

double BaseFuncLiquidComponent::GetLinearDragCoefficient() const {return m_liquid.linearDragCoefficient;}
void BaseFuncLiquidComponent::SetLinearDragCoefficient(double coefficient) {m_liquid.linearDragCoefficient = coefficient;}

double BaseFuncLiquidComponent::GetTorqueDragCoefficient() const {return m_liquid.torqueDragCoefficient;}
void BaseFuncLiquidComponent::SetTorqueDragCoefficient(double coefficient) {m_liquid.torqueDragCoefficient = coefficient;}

float BaseFuncLiquidComponent::GetStiffness() const {return m_liquid.stiffness;}
void BaseFuncLiquidComponent::SetStiffness(float stiffness) {m_liquid.stiffness = stiffness;}
float BaseFuncLiquidComponent::GetPropagation() const {return m_liquid.propagation;}
void BaseFuncLiquidComponent::SetPropagation(float propagation) {m_liquid.propagation = propagation;}
uint32_t BaseFuncLiquidComponent::GetSpacing() const {return m_spacing;}
void BaseFuncLiquidComponent::SetSpacing(uint32_t spacing) {m_spacing = spacing;}

void BaseFuncLiquidComponent::InitializeWaterSurface()
{
	ClearWaterSurface();
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return;
	if(m_surfaceC.expired())
		return;
	auto meshInfo = m_surfaceC->FindAndAssignMesh([](ModelMesh &mesh,ModelSubMesh &subMesh,Material &mat,const std::string &shader) -> uint32_t {
		return (shader == "water") ? 1 : 0;
	});
	m_originalWaterPlaneDistance = meshInfo.has_value() ? m_surfaceC->GetPlaneDistance() : 0;
	if(meshInfo.has_value() && m_kvSurfaceMaterial.empty() == true)
	{
		auto &data = meshInfo->material->GetDataBlock();
		if(data != nullptr)
		{
			std::string surfaceMatIdentifier;
			if(data->GetString("surfacematerial",&surfaceMatIdentifier) == true)
				SetSurfaceMaterial(surfaceMatIdentifier);
		}
	}
}

void BaseFuncLiquidComponent::SetSurfaceMaterial(const std::string &mat)
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	SetSurfaceMaterial(game->GetSurfaceMaterial(mat));
}

void BaseFuncLiquidComponent::SetSurfaceMaterial(const SurfaceMaterial *mat)
{
	if(mat == nullptr)
		return;
	SetDensity(mat->GetDensity());
	SetLinearDragCoefficient(mat->GetLinearDragCoefficient());
	SetTorqueDragCoefficient(mat->GetTorqueDragCoefficient());
	SetStiffness(mat->GetWaveStiffness());
	SetPropagation(mat->GetWavePropagation());
}

void BaseFuncLiquidComponent::ReloadSurfaceSimulator()
{
	m_physSurfaceSim = nullptr;
	auto &ent = GetEntity();
	auto *surfC = static_cast<BaseSurfaceComponent*>(m_surfaceC.get());
	auto *mesh = surfC ? surfC->GetMesh() : nullptr;
	if(ShouldSimulateSurface() == false || !mesh)
	{
		BroadcastEvent(EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
		return;
	}
	auto pTrComponent = ent.GetTransformComponent();
	Vector3 min,max;
	mesh->GetBounds(min,max);
	if(pTrComponent != nullptr)
	{
		pTrComponent->LocalToWorld(&min);
		pTrComponent->LocalToWorld(&max);
	}

	surfC->SetPlaneDistance(m_originalWaterPlaneDistance +m_kvMaxWaveHeight);

	Vector3 n;
	float d;
	surfC->GetPlaneWs(n,d);
	m_physSurfaceSim = InitializeSurfaceSimulator(Vector2(min.x,min.z),Vector2(max.x,max.z),(n *static_cast<float>(d)).y); // TODO
	m_physSurfaceSim->SetMaxWaveHeight(m_kvMaxWaveHeight);
	m_physSurfaceSim->Initialize();
	BroadcastEvent(EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
}

void BaseFuncLiquidComponent::CreateSplash(const Vector3 &origin,float radius,float force)
{
	m_splashes.push({});
	auto &splashInfo = m_splashes.back();
	splashInfo.origin = origin;
	splashInfo.radius = radius;
	splashInfo.force = force;
}

bool BaseFuncLiquidComponent::ShouldSimulateSurface() const {return ((GetEntity().GetSpawnFlags() &umath::to_integral(SpawnFlags::SurfaceSimulation)) != 0) ? true : false;}

void BaseFuncLiquidComponent::ClearWaterSurface()
{
	if(m_surfaceC.valid())
	{
		m_surfaceC->Clear();
		m_originalWaterPlaneDistance = m_surfaceC->GetPlaneDistance();
	}
	m_physSurfaceSim = nullptr;
	BroadcastEvent(EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
}

std::shared_ptr<PhysWaterSurfaceSimulator> BaseFuncLiquidComponent::InitializeSurfaceSimulator(const Vector2 &min,const Vector2 &max,float originY) {return std::make_shared<PhysWaterSurfaceSimulator>(min,max,originY,GetSpacing(),GetStiffness(),GetPropagation());}
void BaseFuncLiquidComponent::OnEndTouch(BaseEntity *ent,PhysObj *phys)
{
	auto pSubmergibleComponent = ent->GetComponent<pragma::SubmergibleComponent>();
	if(pSubmergibleComponent.valid())
		pSubmergibleComponent->SetSubmergedFraction(GetEntity(),0.f);
}

void BaseFuncLiquidComponent::SimulateBuoyancy() const
{
	if(m_surfaceC.expired())
		return;
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto &buoyancySim = physEnv->GetWaterBuoyancySimulator();
	auto *touchComponent = static_cast<BaseTouchComponent*>(ent.FindComponent("touch").get());

	Vector3 n;
	float d;
	m_surfaceC->GetPlaneWs(n,d);

	if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->LockParticleHeights();
	//const std::vector<PhysTouch> &BaseTouchComponent::GetTouchingInfo() const {return m_touching;}
	if(touchComponent != nullptr)
	{
		for(auto &touchInfo : touchComponent->GetTouchingInfo())
		{
			if(touchInfo.touch.entity.valid() == false || touchInfo.triggered == false)
				continue;
			buoyancySim.Simulate(const_cast<BaseEntity&>(ent),m_liquid,const_cast<BaseEntity&>(*touchInfo.touch.entity.get()),n,d,m_waterVelocity,m_physSurfaceSim.get());
		} // TODO: Trigger has to be higher than max surface height
	}
	if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->UnlockParticleHeights();
	/*if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->LockParticleHeights();
	auto *ent = m_entity->GetNetworkState()->GetGameState()->FindEntityByClass("prop_physics");
	if(ent != nullptr)
		buoyancySim.Simulate(m_liquid,*ent,n,d,m_waterVelocity,m_physSurfaceSim.get());
	if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->UnlockParticleHeights();*/
}

bool BaseFuncLiquidComponent::CalcLineSurfaceIntersection(const Vector3 &lineOrigin,const Vector3 &lineDir,double *outT,double *outU,double *outV,bool bCull) const
{
	double t,u,v;
	if(m_physSurfaceSim != nullptr)
	{
		auto width = m_physSurfaceSim->GetWidth();
		auto length = m_physSurfaceSim->GetLength();
		for(auto i=decltype(width){0};i<(width -1);++i)
		{
			for(auto j=decltype(length){0};j<(length -1);++j)
			{
				auto ptIdx0 = m_physSurfaceSim->GetParticleIndex(i,j);
				auto ptIdx1 = m_physSurfaceSim->GetParticleIndex(i +1,j);
				auto ptIdx2 = m_physSurfaceSim->GetParticleIndex(i,j +1);
				auto v0 = m_physSurfaceSim->CalcParticlePosition(ptIdx0);
				auto v1 = m_physSurfaceSim->CalcParticlePosition(ptIdx1);
				auto v2 = m_physSurfaceSim->CalcParticlePosition(ptIdx2);
				//m_triangleIndices.push_back(ptIdx0);
				//m_triangleIndices.push_back(ptIdx1);
				//m_triangleIndices.push_back(ptIdx2);

				auto ptIdx3 = m_physSurfaceSim->GetParticleIndex(i +1,j +1);
				auto v3 = m_physSurfaceSim->CalcParticlePosition(ptIdx3);
				if(
					umath::intersection::line_triangle(lineOrigin,lineDir,v0,v1,v2,t,u,v,bCull) == true ||
					umath::intersection::line_triangle(lineOrigin,lineDir,v3,v2,v1,t,u,v,bCull) == true
				)
				{
					if(outT != nullptr)
						*outT = t;
					if(outU != nullptr)
						*outU = u;
					if(outV != nullptr)
						*outV = v;
					return true;
				}
				//m_triangleIndices.push_back(ptIdx3);
				//m_triangleIndices.push_back(ptIdx2);
				//m_triangleIndices.push_back(ptIdx1);
			}
		}
	}
	else
	{
		if(m_surfaceC.expired())
			return false;
		return m_surfaceC->CalcLineSurfaceIntersection(lineOrigin,lineDir,outT);
	}
	return false;
}
#pragma optimize("",on)
