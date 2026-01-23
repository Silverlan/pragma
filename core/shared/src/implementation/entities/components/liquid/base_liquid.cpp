// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.liquid.base_liquid;

// #define ENABLE_DEPRECATED_PHYSICS

using namespace pragma;

void BaseFuncLiquidComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}
void BaseFuncLiquidComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(basePhysicsComponent::EVENT_HANDLE_RAYCAST, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
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
	auto *pRenderComponent = dynamic_cast<BaseRenderComponent *>(&component);
	auto *pSurfC = dynamic_cast<BaseSurfaceComponent *>(&component);
	auto *pSurfSim = dynamic_cast<BaseLiquidSurfaceSimulationComponent *>(&component);
	if(pRenderComponent != nullptr)
		pRenderComponent->SetCastShadows(false);
	else if(pSurfC)
		m_surfaceC = pSurfC->GetHandle<BaseSurfaceComponent>();
	else if(pSurfSim)
		m_surfSim = pSurfSim->GetHandle<BaseLiquidSurfaceSimulationComponent>();
}

bool BaseFuncLiquidComponent::OnRayResultCallback(physics::CollisionMask rayCollisionGroup, physics::CollisionMask rayCollisionMask)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	if(m_physSurfaceSim == nullptr || (rayCollisionMask & pragma::physics::CollisionMask::WaterSurface) == pragma::physics::CollisionMask::None)
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
	m_surfaceC->FindAndAssignMesh([](geometry::ModelMesh &mesh, geometry::ModelSubMesh &subMesh, material::Material &mat, const std::string &shader) -> uint32_t { return (shader == "water") ? 1 : 0; });
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
