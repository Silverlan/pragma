// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.attachment;

import :entities;
import :server_state;

using namespace pragma;

void SAttachmentComponent::Initialize() { BaseAttachmentComponent::Initialize(); }
void SAttachmentComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SAttachmentComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseAttachmentComponent)); }

AttachmentData *SAttachmentComponent::SetupAttachment(ecs::BaseEntity *ent, const AttachmentInfo &attInfo)
{
	auto *attData = BaseAttachmentComponent::SetupAttachment(ent, attInfo);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared() && attData != nullptr) {
		NetPacket p;
		networking::write_entity(p, &entThis);
		networking::write_entity(p, ent);
		p->Write<FAttachmentMode>(attInfo.flags);
		p->Write<Vector3>(attData->offset);
		p->Write<Quat>(attData->rotation);
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETPARENT, p, networking::Protocol::SlowReliable);
	}
	return attData;
}

void SAttachmentComponent::SetAttachmentFlags(FAttachmentMode flags)
{
	BaseAttachmentComponent::SetAttachmentFlags(flags);
	auto &entThis = static_cast<SBaseEntity &>(GetEntity());
	if(entThis.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &entThis);
		p->Write<FAttachmentMode>(flags);
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETPARENTMODE, p, networking::Protocol::SlowReliable);
	}
}

void SAttachmentComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
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
	}
}
