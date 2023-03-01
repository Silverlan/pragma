/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"
#include <pragma/networking/enums.hpp>
#include <pragma/entities/components/base_parent_component.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include "pragma/entities/components/s_attachable_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

extern DLLSERVER ServerState *server;

void SAttachableComponent::Initialize() { BaseAttachableComponent::Initialize(); }
void SAttachableComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SAttachableComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseAttachableComponent)); }

AttachmentData *SAttachableComponent::SetupAttachment(BaseEntity *ent, const AttachmentInfo &attInfo)
{
	auto *attData = BaseAttachableComponent::SetupAttachment(ent, attInfo);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared() && attData != nullptr) {
		NetPacket p;
		nwm::write_entity(p, &entThis);
		nwm::write_entity(p, ent);
		p->Write<FAttachmentMode>(attInfo.flags);
		p->Write<Vector3>(attData->offset);
		p->Write<Quat>(attData->rotation);
		server->SendPacket("ent_setparent", p, pragma::networking::Protocol::SlowReliable);
	}
	return attData;
}

void SAttachableComponent::SetAttachmentFlags(FAttachmentMode flags)
{
	BaseAttachableComponent::SetAttachmentFlags(flags);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &entThis);
		p->Write<FAttachmentMode>(flags);
		server->SendPacket("ent_setparentmode", p, pragma::networking::Protocol::SlowReliable);
	}
}

void SAttachableComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	if(m_attachment == nullptr)
		packet->Write<Bool>(false);
	else {
		auto *info = m_attachment.get();
		packet->Write<Bool>(true);
		packet->Write<int>(info->attachment);
		packet->Write<int>(info->bone);
		packet->Write<FAttachmentMode>(info->flags);
		packet->Write<Vector3>(info->offset);
		packet->Write<Quat>(info->rotation);
		if(info->boneMapping.empty())
			packet->Write<Bool>(false);
		else {
			packet->Write<Bool>(true);
			packet->Write<UInt32>(CUInt32(info->boneMapping.size()));
			for(auto it = info->boneMapping.begin(); it != info->boneMapping.end(); ++it)
				packet->Write<int>(*it);
		}
		auto &hParent = info->parent;
		if(hParent.expired())
			nwm::write_unique_entity(packet, nullptr);
		else
			nwm::write_unique_entity(packet, &hParent->GetEntity());
	}
}
