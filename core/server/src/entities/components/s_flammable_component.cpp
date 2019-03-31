#include "stdafx_server.h"
#include "pragma/entities/components/s_flammable_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>
#include <pragma/lua/libraries/ltimer.h>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

extern DLLSERVER SGame *s_game;

SFlammableComponent::IgniteInfo::IgniteInfo()
	: damageTimer()
{}

SFlammableComponent::IgniteInfo::~IgniteInfo()
{
	Clear();
}

void SFlammableComponent::IgniteInfo::Clear()
{
	if(damageTimer != nullptr && damageTimer->IsValid())
		damageTimer->GetTimer()->Remove(s_game);
	hAttacker = {};
	hInflictor = {};
}

/////////////////////////

void SFlammableComponent::ApplyIgnitionDamage()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pDamageableComponent = ent.GetComponent<pragma::DamageableComponent>();
	if(pTrComponent.expired() || pDamageableComponent.expired())
		return;
	auto pos = ent.GetCenter();
	DamageInfo info {};
	info.SetAttacker(m_igniteInfo.hAttacker.get());
	info.SetInflictor(m_igniteInfo.hInflictor.get());
	info.SetDamageType(DAMAGETYPE::IGNITE);
	info.SetHitPosition(pos);
	info.SetSource(pos);
	info.SetDamage(5);
	pDamageableComponent->TakeDamage(info);
}
luabind::object SFlammableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFlammableComponentHandleWrapper>(l);}
void SFlammableComponent::Ignite(float duration,BaseEntity *attacker,BaseEntity *inflictor)
{
	if(!IsIgnitable())
		return;
	NetPacket p {};
	p->Write<float>(duration);
	nwm::write_entity(p,attacker);
	nwm::write_entity(p,inflictor);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvIgnite,p);

	auto reps = static_cast<uint32_t>(umath::floor(duration /0.5f));
	Timer *t = nullptr;
	if(IsOnFire() && m_igniteInfo.damageTimer != nullptr && m_igniteInfo.damageTimer->IsValid())
		t = m_igniteInfo.damageTimer->GetTimer();
	if(t == nullptr)
	{
		t = s_game->CreateTimer(0.5f,reps,FunctionCallback<void>::Create([this]() {
			ApplyIgnitionDamage();
		}));
		t->Start(s_game);
	}
	else if(reps > t->GetRepetitionsLeft())
		t->SetRepetitions(reps);

	m_igniteInfo.damageTimer = t->CreateHandle();
	m_igniteInfo.hAttacker = (attacker != nullptr) ? attacker->GetHandle() : EntityHandle{};
	m_igniteInfo.hInflictor = (inflictor != nullptr) ? inflictor->GetHandle() : EntityHandle{};

	BaseFlammableComponent::Ignite(duration,attacker,inflictor);
}
void SFlammableComponent::Extinguish()
{
	if(!IsOnFire())
		return;
	BaseFlammableComponent::Extinguish();
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvExtinguish);
	m_igniteInfo.Clear();
}
void SFlammableComponent::SetIgnitable(bool b)
{
	if(b == IsIgnitable())
		return;
	BaseFlammableComponent::SetIgnitable(b);
	NetPacket p {};
	p->Write<bool>(b);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetIgnitable,p);
}
void SFlammableComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<bool>(*m_bIgnitable);
	packet->Write<bool>(*m_bIsOnFire);
	if(*m_bIsOnFire == true)
	{
		auto dur = (m_igniteInfo.damageTimer->IsValid()) ? m_igniteInfo.damageTimer->GetTimer()->GetTimeLeft() : 0.f;
		packet->Write<float>(dur);
		nwm::write_entity(packet,m_igniteInfo.hAttacker.get());
		nwm::write_entity(packet,m_igniteInfo.hInflictor.get());
	}
}
