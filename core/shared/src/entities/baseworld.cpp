#include "stdafx_shared.h"
#include "pragma/entities/baseworld.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include <pragma/physics/collisiontypes.h>

using namespace pragma;

void BaseWorldComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent*>(whRenderComponent.get())->SetCastShadows(true);
	ent.AddComponent("model");
	auto whPhysComponent = ent.AddComponent("physics");
	if(whPhysComponent.valid())
		static_cast<pragma::BasePhysicsComponent*>(whPhysComponent.get())->SetCollisionType(COLLISIONTYPE::BRUSH);
}

void BaseWorldComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	ent.GetNetworkState()->GetGameState()->SetWorld(this);
}

void BaseWorldComponent::GetBrushes(std::vector<PolyMesh*> **brushes) {*brushes = &m_brushes;}
void BaseWorldComponent::SetBrushes(std::vector<PolyMesh*> brushes) {m_brushes = brushes;}

Con::c_cout &BaseWorldComponent::print(Con::c_cout &os)
{
	os<<"World";
	return os;
}