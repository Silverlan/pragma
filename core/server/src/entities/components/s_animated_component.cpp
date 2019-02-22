#include "stdafx_server.h"
#include "pragma/entities/components/s_animated_component.hpp"

using namespace pragma;

extern DLLSERVER ServerState *server;

void SAnimatedComponent::Initialize()
{
	BaseAnimatedComponent::Initialize();
}
luabind::object SAnimatedComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SAnimatedComponentHandleWrapper>(l);}
void SAnimatedComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BaseAnimatedComponent::RegisterEvents(componentManager);
}
void SAnimatedComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const {outTypeIndex = std::type_index(typeid(BaseAnimatedComponent));}
void SAnimatedComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<int>(GetAnimation());
	packet->Write<float>(GetCycle());
}

void SAnimatedComponent::PlayAnimation(int animation,FPlayAnim flags)
{
	BaseAnimatedComponent::PlayAnimation(animation,flags);

	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	if((flags &pragma::FPlayAnim::Transmit) != pragma::FPlayAnim::None)
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<int>(GetBaseAnimationInfo().animation);
		server->BroadcastUDP("ent_anim_play",p);
	}
}
void SAnimatedComponent::StopLayeredAnimation(int slot)
{
	auto it = m_animSlots.find(slot);
	if(it == m_animSlots.end())
		return;
	BaseAnimatedComponent::StopLayeredAnimation(slot);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	auto &animInfo = it->second;
	if((animInfo.flags &pragma::FPlayAnim::Transmit) != pragma::FPlayAnim::None)
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<int>(slot);
		server->BroadcastTCP("ent_anim_gesture_stop",p);
	}
}
void SAnimatedComponent::PlayLayeredAnimation(int slot,int animation,FPlayAnim flags)
{
	BaseAnimatedComponent::PlayLayeredAnimation(slot,animation,flags);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	if((flags &pragma::FPlayAnim::Transmit) != pragma::FPlayAnim::None)
	{
		auto it = m_animSlots.find(slot);
		if(it == m_animSlots.end())
			return;
		auto &animInfo = it->second;
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<int>(slot);
		p->Write<int>(animInfo.animation);
		server->BroadcastTCP("ent_anim_gesture_play",p);
	}
}
