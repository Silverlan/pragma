// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/components/s_animated_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SAnimatedComponent::Initialize() { BaseAnimatedComponent::Initialize(); }
void SAnimatedComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SAnimatedComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BaseAnimatedComponent::RegisterEvents(componentManager, registerEvent); }
void SAnimatedComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseAnimatedComponent)); }
void SAnimatedComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<int>(GetAnimation());
	packet->Write<float>(GetCycle());
}

void SAnimatedComponent::PlayAnimation(int animation, FPlayAnim flags)
{
	BaseAnimatedComponent::PlayAnimation(animation, flags);

	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	if((flags & pragma::FPlayAnim::Transmit) != pragma::FPlayAnim::None) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<int>(GetBaseAnimationInfo().animation);
		server->SendPacket("ent_anim_play", p, pragma::networking::Protocol::FastUnreliable);
	}
}
void SAnimatedComponent::StopLayeredAnimation(int slot)
{
	auto it = m_animSlots.find(slot);
	if(it == m_animSlots.end())
		return;
	BaseAnimatedComponent::StopLayeredAnimation(slot);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	auto &animInfo = it->second;
	if((animInfo.flags & pragma::FPlayAnim::Transmit) != pragma::FPlayAnim::None) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<int>(slot);
		server->SendPacket("ent_anim_gesture_stop", p, pragma::networking::Protocol::SlowReliable);
	}
}
void SAnimatedComponent::PlayLayeredAnimation(int slot, int animation, FPlayAnim flags)
{
	BaseAnimatedComponent::PlayLayeredAnimation(slot, animation, flags);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	if((flags & pragma::FPlayAnim::Transmit) != pragma::FPlayAnim::None) {
		auto it = m_animSlots.find(slot);
		if(it == m_animSlots.end())
			return;
		auto &animInfo = it->second;
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<int>(slot);
		p->Write<int>(animInfo.animation);
		server->SendPacket("ent_anim_gesture_play", p, pragma::networking::Protocol::SlowReliable);
	}
}
