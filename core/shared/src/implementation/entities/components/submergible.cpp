// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.submergible;

using namespace pragma;

void SubmergibleComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	submergibleComponent::EVENT_ON_WATER_SUBMERGED = registerEvent("ON_WATER_SUBMERGED", ComponentEventInfo::Type::Broadcast);
	submergibleComponent::EVENT_ON_WATER_EMERGED = registerEvent("ON_WATER_EMERGED", ComponentEventInfo::Type::Broadcast);
	submergibleComponent::EVENT_ON_WATER_ENTERED = registerEvent("ON_WATER_ENTERED", ComponentEventInfo::Type::Broadcast);
	submergibleComponent::EVENT_ON_WATER_EXITED = registerEvent("ON_WATER_EXITED", ComponentEventInfo::Type::Broadcast);
}
SubmergibleComponent::SubmergibleComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_submergedFraction(util::FloatProperty::Create(0.f)) {}
void SubmergibleComponent::Initialize() { BaseEntityComponent::Initialize(); }
void SubmergibleComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
bool SubmergibleComponent::IsSubmerged() const { return (GetSubmergedFraction() >= 0.6f) ? true : false; }
bool SubmergibleComponent::IsFullySubmerged() const { return (GetSubmergedFraction() >= 0.99f) ? true : false; }
bool SubmergibleComponent::IsInWater() const { return (GetSubmergedFraction() > 0.f) ? true : false; }
float SubmergibleComponent::GetSubmergedFraction() const { return *m_submergedFraction; }
void SubmergibleComponent::SetSubmergedFraction(ecs::BaseEntity &waterEntity, float fraction)
{
	m_waterEntity = (fraction > 0.f) ? waterEntity.GetHandle() : EntityHandle {};
	auto bInWater = IsInWater();
	auto bSubmerged = IsSubmerged();
	*m_submergedFraction = fraction;

	// Call appropriate callbacks
	if(bInWater == false) {
		if(IsInWater() == false)
			return;
		OnWaterEntered();
		if(bSubmerged == false) {
			if(IsSubmerged() == false)
				return;
			OnWaterSubmerged();
		}
		return;
	}
	if(IsInWater() == false) {
		if(bSubmerged == true && IsSubmerged() == false)
			OnWaterEmerged();
		OnWaterExited();
		return;
	}
	if(bSubmerged == true) {
		if(IsSubmerged() == false)
			OnWaterEmerged();
		return;
	}
	else if(IsSubmerged() == true)
		OnWaterSubmerged();
}
void SubmergibleComponent::OnWaterSubmerged() { BroadcastEvent(submergibleComponent::EVENT_ON_WATER_SUBMERGED); }
void SubmergibleComponent::OnWaterEmerged() { BroadcastEvent(submergibleComponent::EVENT_ON_WATER_EMERGED); }
void SubmergibleComponent::OnWaterEntered() { BroadcastEvent(submergibleComponent::EVENT_ON_WATER_ENTERED); }
void SubmergibleComponent::OnWaterExited() { BroadcastEvent(submergibleComponent::EVENT_ON_WATER_EXITED); }

const util::PFloatProperty &SubmergibleComponent::GetSubmergedFractionProperty() const { return m_submergedFraction; }

ecs::BaseEntity *SubmergibleComponent::GetWaterEntity() { return m_waterEntity.get(); }
const ecs::BaseEntity *SubmergibleComponent::GetWaterEntity() const { return const_cast<SubmergibleComponent *>(this)->GetWaterEntity(); }
