// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.liquid.base_control;

using namespace pragma;

void BaseLiquidControlComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseLiquidControlComponent::EVENT_ON_SPLASH = registerEvent("ON_SPLASH", ComponentEventInfo::Type::Broadcast);
	baseLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED = registerEvent("ON_PROPERTIES_CHANGED", ComponentEventInfo::Type::Broadcast);
}

void BaseLiquidControlComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}

BaseLiquidControlComponent::BaseLiquidControlComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseLiquidControlComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvCreateSplash = SetupNetEvent("create_splash");
	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "surface_material", false)) {
			if(pragma::string::compare<std::string>(kvData.value, "default", false) == false) {
				m_kvSurfaceMaterial = kvData.value;
				SetSurfaceMaterial(kvData.value);
			}
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(baseSurfaceComponent::EVENT_ON_SURFACE_MESH_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
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

void BaseLiquidControlComponent::SetSurfaceMaterial(const physics::SurfaceMaterial *mat)
{
	if(mat == nullptr)
		return;
	SetDensity(mat->GetDensity());
	SetLinearDragCoefficient(mat->GetLinearDragCoefficient());
	SetTorqueDragCoefficient(mat->GetTorqueDragCoefficient());
	SetStiffness(mat->GetWaveStiffness());
	SetPropagation(mat->GetWavePropagation());
}

const physics::PhysLiquid &BaseLiquidControlComponent::GetLiquidDescription() const { return const_cast<BaseLiquidControlComponent *>(this)->GetLiquidDescription(); }
physics::PhysLiquid &BaseLiquidControlComponent::GetLiquidDescription() { return m_liquid; }

double BaseLiquidControlComponent::GetDensity() const { return m_liquid.density; }
void BaseLiquidControlComponent::SetDensity(double density)
{
	m_liquid.density = density;

	BroadcastEvent(baseLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED);
}

double BaseLiquidControlComponent::GetLinearDragCoefficient() const { return m_liquid.linearDragCoefficient; }
void BaseLiquidControlComponent::SetLinearDragCoefficient(double coefficient)
{
	m_liquid.linearDragCoefficient = coefficient;
	BroadcastEvent(baseLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED);
}

double BaseLiquidControlComponent::GetTorqueDragCoefficient() const { return m_liquid.torqueDragCoefficient; }
void BaseLiquidControlComponent::SetTorqueDragCoefficient(double coefficient)
{
	m_liquid.torqueDragCoefficient = coefficient;
	BroadcastEvent(baseLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED);
}

float BaseLiquidControlComponent::GetStiffness() const { return m_liquid.stiffness; }
void BaseLiquidControlComponent::SetStiffness(float stiffness)
{
	m_liquid.stiffness = stiffness;
	BroadcastEvent(baseLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED);
}
float BaseLiquidControlComponent::GetPropagation() const { return m_liquid.propagation; }
void BaseLiquidControlComponent::SetPropagation(float propagation)
{
	m_liquid.propagation = propagation;
	BroadcastEvent(baseLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED);
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

	BroadcastEvent(baseLiquidControlComponent::EVENT_ON_SPLASH, CEOnSplash {splashInfo});
}

bool BaseLiquidControlComponent::OnBulletHit(const game::BulletInfo &bulletInfo, const physics::TraceData &data, physics::PhysObj *phys, physics::ICollisionObject *col, const LocalRayResult &result)
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
void CEOnSplash::PushArguments(lua::State *l)
{
	Lua::Push<Vector3>(l, splashInfo.origin);
	Lua::PushNumber(l, splashInfo.radius);
	Lua::PushNumber(l, splashInfo.force);
}
