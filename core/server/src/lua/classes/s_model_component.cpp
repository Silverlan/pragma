/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/model/s_modelmanager.h"
#include "pragma/entities/components/s_model_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SModelComponent::Initialize()
{
	BaseModelComponent::Initialize();
}
luabind::object SModelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SModelComponentHandleWrapper>(l);}

void SModelComponent::OnModelChanged(const std::shared_ptr<Model> &model)
{
	BaseModelComponent::OnModelChanged(model);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->WriteString(GetModelName());
		server->SendPacket("ent_model",p,pragma::networking::Protocol::SlowReliable);
	}
}

bool SModelComponent::SetBodyGroup(UInt32 groupId,UInt32 id)
{
	if(GetBodyGroup(groupId) == id)
		return true;
	auto r = BaseModelComponent::SetBodyGroup(groupId,id);
	if(r == false)
		return r;
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return r;
	NetPacket p;
	p->Write<UInt32>(groupId);
	p->Write<UInt32>(id);
	ent.SendNetEvent(m_netEvSetBodyGroup,p,pragma::networking::Protocol::SlowReliable);
	return r;
}

void SModelComponent::SetSkin(unsigned int skin)
{
	if(skin == GetSkin())
		return;
	BaseModelComponent::SetSkin(skin);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->Write<unsigned int>(skin);
	server->SendPacket("ent_skin",p,pragma::networking::Protocol::SlowReliable);
}

void SModelComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	std::string mdl = GetModelName();
	packet->WriteString(mdl);
	packet->Write<unsigned int>(GetSkin());

	auto &bodyGroups = GetBodyGroups();
	packet->Write<uint32_t>(static_cast<uint32_t>(bodyGroups.size()));
	for(auto bg : bodyGroups)
		packet->Write<uint32_t>(bg);
}
