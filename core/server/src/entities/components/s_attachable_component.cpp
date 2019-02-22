#include "stdafx_server.h"
#include "pragma/entities/components/s_attachable_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

extern DLLSERVER ServerState *server;

void SAttachableComponent::Initialize()
{
	BaseAttachableComponent::Initialize();
}
luabind::object SAttachableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SAttachableComponentHandleWrapper>(l);}

AttachmentData *SAttachableComponent::SetupAttachment(BaseEntity *ent,const AttachmentInfo &attInfo)
{
	auto *attData = BaseAttachableComponent::SetupAttachment(ent,attInfo);
	auto &entThis = static_cast<SBaseEntity&>(GetEntity());
	if(entThis.IsShared() && attData != nullptr)
	{
		NetPacket p;
		nwm::write_entity(p,&entThis);
		nwm::write_entity(p,ent);
		p->Write<FAttachmentMode>(attInfo.flags);
		p->Write<Vector3>(attData->offset);
		p->Write<Quat>(attData->rotation);
		server->BroadcastTCP("ent_setparent",p);
	}
	return attData;
}

void SAttachableComponent::SetAttachmentFlags(FAttachmentMode flags)
{
	BaseAttachableComponent::SetAttachmentFlags(flags);
	auto &entThis = static_cast<SBaseEntity&>(GetEntity());
	if(entThis.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&entThis);
		p->Write<FAttachmentMode>(flags);
		server->BroadcastTCP("ent_setparentmode",p);
	}
}

void SAttachableComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	if(m_attachment == nullptr)
		packet->Write<Bool>(false);
	else
	{
		auto *info = m_attachment.get();
		packet->Write<Bool>(true);
		auto &hParent = info->parent;
		if(hParent.expired())
			nwm::write_entity(packet,nullptr);
		else
			nwm::write_entity(packet,&hParent->GetEntity());
		packet->Write<int>(info->attachment);
		packet->Write<int>(info->bone);
		packet->Write<FAttachmentMode>(info->flags);
		packet->Write<Vector3>(info->offset);
		packet->Write<Quat>(info->rotation);
		if(info->boneMapping.empty())
			packet->Write<Bool>(false);
		else
		{
			packet->Write<Bool>(true);
			packet->Write<UInt32>(CUInt32(info->boneMapping.size()));
			for(auto it=info->boneMapping.begin();it!=info->boneMapping.end();++it)
				packet->Write<int>(*it);
		}
	}
}
