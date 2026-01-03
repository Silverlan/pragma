// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.components.liquid.base_surface_simulation;

using namespace pragma;

void BaseLiquidSurfaceSimulationComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED = registerEvent("ON_WATER_SURFACE_SIMULATOR_CHANGED", ComponentEventInfo::Type::Broadcast);
}

void BaseLiquidSurfaceSimulationComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}

BaseLiquidSurfaceSimulationComponent::BaseLiquidSurfaceSimulationComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidSurfaceSimulationComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "max_wave_height", false))
			SetMaxWaveHeight(string::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(baseSurfaceComponent::EVENT_ON_SURFACE_MESH_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto *surfC = static_cast<BaseSurfaceComponent *>(GetEntity().FindComponent("surface").get());
		assert(surfC);
		if(surfC)
			m_originalWaterPlaneDistance = surfC->GetPlaneDistance();
	});
}

void BaseLiquidSurfaceSimulationComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }

uint32_t BaseLiquidSurfaceSimulationComponent::GetSpacing() const { return m_spacing; }
void BaseLiquidSurfaceSimulationComponent::SetSpacing(uint32_t spacing) { m_spacing = spacing; }

bool BaseLiquidSurfaceSimulationComponent::ShouldSimulateSurface() const { return ((GetEntity().GetSpawnFlags() & math::to_integral(SpawnFlags::SurfaceSimulation)) != 0) ? true : false; }
void BaseLiquidSurfaceSimulationComponent::SetMaxWaveHeight(float height) { m_kvMaxWaveHeight = height; }

const physics::PhysWaterSurfaceSimulator *BaseLiquidSurfaceSimulationComponent::GetSurfaceSimulator() const { return const_cast<BaseLiquidSurfaceSimulationComponent *>(this)->GetSurfaceSimulator(); }
physics::PhysWaterSurfaceSimulator *BaseLiquidSurfaceSimulationComponent::GetSurfaceSimulator() { return m_physSurfaceSim.get(); }

std::shared_ptr<physics::PhysWaterSurfaceSimulator> BaseLiquidSurfaceSimulationComponent::InitializeSurfaceSimulator(const Vector2 &min, const Vector2 &max, float originY)
{
	auto *controlC = static_cast<BaseLiquidControlComponent *>(GetEntity().FindComponent("liquid_control").get());
	if(!controlC)
		return nullptr;
	return pragma::util::make_shared<physics::PhysWaterSurfaceSimulator>(min, max, originY, GetSpacing(), controlC->GetStiffness(), controlC->GetPropagation());
}

void BaseLiquidSurfaceSimulationComponent::ClearSurfaceSimulator()
{
	m_physSurfaceSim = nullptr;
	m_originalWaterPlaneDistance = 0.f;
	BroadcastEvent(baseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
}

bool BaseLiquidSurfaceSimulationComponent::CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT, double *outU, double *outV, bool bCull) const
{
	double t, u, v;
	if(m_physSurfaceSim != nullptr) {
		auto width = m_physSurfaceSim->GetWidth();
		auto length = m_physSurfaceSim->GetLength();
		for(auto i = decltype(width) {0}; i < (width - 1); ++i) {
			for(auto j = decltype(length) {0}; j < (length - 1); ++j) {
				auto ptIdx0 = m_physSurfaceSim->GetParticleIndex(i, j);
				auto ptIdx1 = m_physSurfaceSim->GetParticleIndex(i + 1, j);
				auto ptIdx2 = m_physSurfaceSim->GetParticleIndex(i, j + 1);
				auto v0 = m_physSurfaceSim->CalcParticlePosition(ptIdx0);
				auto v1 = m_physSurfaceSim->CalcParticlePosition(ptIdx1);
				auto v2 = m_physSurfaceSim->CalcParticlePosition(ptIdx2);
				//m_triangleIndices.push_back(ptIdx0);
				//m_triangleIndices.push_back(ptIdx1);
				//m_triangleIndices.push_back(ptIdx2);

				auto ptIdx3 = m_physSurfaceSim->GetParticleIndex(i + 1, j + 1);
				auto v3 = m_physSurfaceSim->CalcParticlePosition(ptIdx3);
				if(math::intersection::line_triangle(lineOrigin, lineDir, v0, v1, v2, t, u, v, bCull) == true || math::intersection::line_triangle(lineOrigin, lineDir, v3, v2, v1, t, u, v, bCull) == true) {
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
	else {
		auto *surfC = GetSurfaceComponent();
		if(!surfC)
			return false;
		return surfC->CalcLineSurfaceIntersection(lineOrigin, lineDir, outT);
	}
	return false;
}

BaseSurfaceComponent *BaseLiquidSurfaceSimulationComponent::GetSurfaceComponent() const { return static_cast<BaseSurfaceComponent *>(GetEntity().FindComponent("surface").get()); }

void BaseLiquidSurfaceSimulationComponent::ReloadSurfaceSimulator()
{
	m_physSurfaceSim = nullptr;
	auto &ent = GetEntity();
	auto *surfC = GetSurfaceComponent();
	auto *mesh = surfC ? surfC->GetMesh() : nullptr;
	if(ShouldSimulateSurface() == false || !mesh) {
		BroadcastEvent(baseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
		return;
	}
	auto pTrComponent = ent.GetTransformComponent();
	Vector3 min, max;
	mesh->GetBounds(min, max);
	if(pTrComponent != nullptr) {
		pTrComponent->LocalToWorld(&min);
		pTrComponent->LocalToWorld(&max);
	}

	surfC->SetPlaneDistance(m_originalWaterPlaneDistance + m_kvMaxWaveHeight);

	Vector3 n;
	float d;
	surfC->GetPlaneWs(n, d);
	m_physSurfaceSim = InitializeSurfaceSimulator(Vector2(min.x, min.z), Vector2(max.x, max.z), (n * static_cast<float>(d)).y); // TODO
	m_physSurfaceSim->SetMaxWaveHeight(m_kvMaxWaveHeight);
	m_physSurfaceSim->Initialize();
	BroadcastEvent(baseLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED);
}
