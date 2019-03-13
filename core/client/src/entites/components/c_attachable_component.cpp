#include "stdafx_client.h"
#include "pragma/entities/components/c_attachable_component.hpp"
#include "pragma/networking/c_nwm_util.h"

using namespace pragma;

extern DLLCLIENT CGame *c_game;

luabind::object CAttachableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CAttachableComponentHandleWrapper>(l);}
void CAttachableComponent::ReceiveData(NetPacket &packet)
{
	auto bParent = packet->Read<Bool>();
	if(bParent == true)
	{
		auto att = packet->Read<int>();
		auto bone = packet->Read<int>();
		auto flags = packet->Read<FAttachmentMode>();
		auto offset = packet->Read<Vector3>();
		auto rot = packet->Read<Quat>();
		auto bBoneMapping = packet->Read<Bool>();
		std::vector<int> boneMapping;
		if(bBoneMapping == true)
		{
			auto numBoneMapping = packet->Read<UInt32>();
			boneMapping.reserve(numBoneMapping);
			for(UInt32 i=0;i<numBoneMapping;i++)
			{
				auto boneId = packet->Read<int>();
				boneMapping.push_back(boneId);
			}
		}

		auto hThis = GetHandle();
		nwm::read_unique_entity(packet,[this,hThis,att,bone,flags,offset,rot,bBoneMapping,boneMapping](BaseEntity *ent) {
			if(hThis.expired())
				return;
			if(m_attachment == nullptr)
				m_attachment = std::make_unique<AttachmentData>();
			if(ent == nullptr)
				m_attachment->parent = {};
			else
			{
				auto pParentComponent = ent->AddComponent<CParentComponent>();
				m_attachment->parent = pParentComponent.valid() ? 
					util::WeakHandle<BaseParentComponent>{std::static_pointer_cast<CParentComponent>(pParentComponent->shared_from_this())} :
					util::WeakHandle<BaseParentComponent>{};
			}
			m_attachment->attachment = att;
			m_attachment->bone = bone;
			m_attachment->flags = flags;
			m_attachment->offset = offset;
			m_attachment->rotation = rot;
			m_attachment->boneMapping = boneMapping;
		});
	}
}

void CAttachableComponent::UpdateViewAttachmentOffset(BaseEntity *ent,pragma::BaseCharacterComponent &pl,Vector3 &pos,Quat &rot,Bool bYawOnly)
{
	auto &scene = c_game->GetScene();
	auto &cam = scene->camera;
	auto &forward = cam->GetForward();
	auto &up = cam->GetUp();
	if(bYawOnly == false)
	{
		pos = cam->GetPos();
		auto right = uvec::cross(forward,up);
		rot = uquat::create(forward,right,up);
	}
	else
	{
		/*BaseEntity::UpdateViewParentOffset(ent,pl,pos,rot,bYawOnly);
		auto &posCam = cam->GetPos();
		pos.x = posCam.x;
		pos.z = posCam.z;*/

		auto pTrComponentEnt = ent->GetTransformComponent();
		if(pTrComponentEnt.valid())
		{
			pos = pTrComponentEnt->GetPosition();//cam->GetPos();
			rot = pTrComponentEnt->GetOrientation();//cam->GetRotation();
		}

		auto &rotRef = pl.GetOrientationAxesRotation();
		auto viewRot = rotRef *cam->GetRotation();//pl->GetViewOrientation();
		auto viewAng = EulerAngles(viewRot);
		auto viewYaw = viewAng.y;

		rot = rotRef *rot;
		auto ang = EulerAngles(rot);
		ang.y = viewYaw;
		rot = uquat::get_inverse(rotRef) *uquat::create(ang);

		/*auto ang = ent->GetAngles();
		ang.y = pl->GetViewAngles().y;
		rot = uquat::create(ang);*/
	}
}
