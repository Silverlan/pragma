// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_flammable;

using namespace pragma;

ComponentEventId baseFlammableComponent::EVENT_ON_IGNITED = pragma::INVALID_COMPONENT_ID;
ComponentEventId baseFlammableComponent::EVENT_ON_EXTINGUISHED = pragma::INVALID_COMPONENT_ID;
void BaseFlammableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseFlammableComponent::EVENT_ON_IGNITED = registerEvent("ON_IGNITED", ComponentEventInfo::Type::Broadcast);
	baseFlammableComponent::EVENT_ON_EXTINGUISHED = registerEvent("ON_EXTINGUISHED", ComponentEventInfo::Type::Broadcast);
}
BaseFlammableComponent::BaseFlammableComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_bIsOnFire(pragma::util::BoolProperty::Create(false)), m_bIgnitable(pragma::util::BoolProperty::Create(true)) {}
BaseFlammableComponent::~BaseFlammableComponent() { Extinguish(); }
void BaseFlammableComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "flammable", false))
			*m_bIgnitable = pragma::util::to_boolean(kvData.value);
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "setflammable", false))
			*m_bIgnitable = pragma::util::to_boolean(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "ignite", false))
			Ignite(pragma::util::to_float(inputData.data));
		else
			return pragma::util::EventReply::Unhandled;
		return pragma::util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	m_netEvIgnite = SetupNetEvent("ignite");
	m_netEvExtinguish = SetupNetEvent("extinguish");
	m_netEvSetIgnitable = SetupNetEvent("set_ignitable");
}
void BaseFlammableComponent::OnTick(double dt)
{
	if(IsOnFire() && m_tExtinguishTime > 0.f) {
		auto &t = GetEntity().GetNetworkState()->GetGameState()->CurTime();
		if(t >= m_tExtinguishTime)
			Extinguish();
	}
}
pragma::util::EventReply BaseFlammableComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == pragma::util::EventReply::Handled)
		return pragma::util::EventReply::Handled;
	if(eventId == pragma::submergibleComponent::EVENT_ON_WATER_SUBMERGED)
		Extinguish();
	return pragma::util::EventReply::Unhandled;
}
void BaseFlammableComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["ignitable"] = **m_bIgnitable;
	udm["isOnFire"] = **m_bIsOnFire;
	auto tCur = GetEntity().GetNetworkState()->GetGameState()->CurTime();
	auto tExtinguish = m_tExtinguishTime;
	if(tExtinguish != 0.f)
		tExtinguish -= tCur;
	udm["timeUntilExtinguish"] = tExtinguish;
}
void BaseFlammableComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto ignitable = IsIgnitable();
	udm["ignitable"](ignitable);
	SetIgnitable(ignitable);

	auto isOnFire = IsOnFire();
	udm["isOnFire"](isOnFire);

	auto tExtinguish = 0.f;
	if(isOnFire == true) {
		udm["timeUntilExtinguish"](tExtinguish);
		Ignite(tExtinguish); // TODO: Attacker, inflictor?
	}
}
const pragma::util::PBoolProperty &BaseFlammableComponent::GetOnFireProperty() const { return m_bIsOnFire; }
const pragma::util::PBoolProperty &BaseFlammableComponent::GetIgnitableProperty() const { return m_bIgnitable; }
bool BaseFlammableComponent::IsOnFire() const { return *m_bIsOnFire; }
bool BaseFlammableComponent::IsIgnitable() const { return *m_bIgnitable; }
pragma::util::EventReply BaseFlammableComponent::Ignite(float duration, pragma::ecs::BaseEntity *attacker, pragma::ecs::BaseEntity *inflictor)
{
	auto &ent = GetEntity();
	auto pSubmergibleComponent = ent.GetComponent<pragma::SubmergibleComponent>();
	if(pSubmergibleComponent.valid() && pSubmergibleComponent->IsSubmerged() == true)
		return pragma::util::EventReply::Handled;
	*m_bIsOnFire = true;
	SetTickPolicy(TickPolicy::Always);
	if(duration == 0.f)
		m_tExtinguishTime = 0.f;
	else {
		auto tNew = ent.GetNetworkState()->GetGameState()->CurTime() + duration;
		if(tNew > m_tExtinguishTime)
			m_tExtinguishTime = static_cast<float>(tNew);
	}
	CEOnIgnited igniteData {duration, attacker, inflictor};
	return BroadcastEvent(baseFlammableComponent::EVENT_ON_IGNITED, igniteData);
}
void BaseFlammableComponent::Extinguish()
{
	if(*m_bIsOnFire == false)
		return;
	*m_bIsOnFire = false;
	SetTickPolicy(TickPolicy::Never);
	m_tExtinguishTime = 0.f;
	BroadcastEvent(baseFlammableComponent::EVENT_ON_EXTINGUISHED);
}
void BaseFlammableComponent::SetIgnitable(bool b)
{
	*m_bIgnitable = b;
	if(b == false)
		Extinguish();
}

////////////

CEOnIgnited::CEOnIgnited(float duration, pragma::ecs::BaseEntity *attacker, pragma::ecs::BaseEntity *inflictor) : duration {duration}, attacker {attacker ? attacker->GetHandle() : EntityHandle {}}, inflictor {inflictor ? inflictor->GetHandle() : EntityHandle {}} {}
void CEOnIgnited::PushArguments(lua::State *l)
{
	Lua::PushNumber(l, duration);

	if(attacker.valid())
		attacker->GetLuaObject().push(l);
	else
		Lua::PushNil(l);

	if(inflictor.valid())
		inflictor->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}
