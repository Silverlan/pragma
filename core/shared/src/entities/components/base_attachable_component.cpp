/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_attachable_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_parent_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/parentinfo.h"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/model/model.h"

using namespace pragma;


ComponentEventId BaseAttachableComponent::EVENT_ON_ATTACHMENT_UPDATE = INVALID_COMPONENT_ID;
void BaseAttachableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	auto componentType = std::type_index(typeid(BaseAttachableComponent));
	EVENT_ON_ATTACHMENT_UPDATE = componentManager.RegisterEvent("ON_ATTACHMENT_UPDATE",componentType);
}
BaseAttachableComponent::BaseAttachableComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void BaseAttachableComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"parent",false) || ustring::compare(kvData.key,"parentname",false))
			m_parentName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		if(m_attachment != nullptr && (m_attachment->flags &FAttachmentMode::BoneMerge) != FAttachmentMode::None)
		{
			static_cast<CEShouldUpdateBones&>(evData.get()).shouldUpdate = true;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateAttachmentOffset();
	});

	GetEntity().AddComponent<LogicComponent>();
}
void BaseAttachableComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_attachment != nullptr)
		ClearAttachment();
}
void BaseAttachableComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_parentName.empty() == false)
	{
		EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(),EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_parentName);
		auto it = entIt.begin();
		if(it != entIt.end())
		{
			AttachmentInfo attInfo {};
			attInfo.flags = /*FAttachmentMode::SnapToOrigin | */FAttachmentMode::UpdateEachFrame;
			AttachToEntity(*it,attInfo);
		}
	}
}
void BaseAttachableComponent::ClearAttachment() {AttachToEntity(nullptr);}
std::optional<physics::Transform> BaseAttachableComponent::GetLocalPose() const
{
	auto *attData = GetAttachmentData();
	if(attData == nullptr)
		return {};
	return physics::Transform{attData->offset,attData->rotation};
}
void BaseAttachableComponent::SetLocalPose(const physics::Transform &pose)
{
	auto *attData = GetAttachmentData();
	if(attData == nullptr)
		return;
	attData->offset = pose.GetOrigin();
	attData->rotation = pose.GetRotation();
}
void BaseAttachableComponent::UpdateAttachmentData(bool bForceReload)
{
	if(m_attachment == nullptr || m_attachment->parent.expired())
		return;
	auto *parent = m_attachment->parent.get();
	auto &entParent = parent->GetEntity();
	if(!((m_attachment->flags &FAttachmentMode::PositionOnly) != FAttachmentMode::None) && (m_attachment->flags &FAttachmentMode::BoneMerge) != FAttachmentMode::None)
	{
		if(!m_attachment->boneMapping.empty())
		{
			if(bForceReload == true)
				m_attachment->boneMapping.clear();
			else
				return;
		}
		auto mdlComponent = GetEntity().GetModelComponent();
		auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(hMdl != nullptr)
		{
			Skeleton &skel = hMdl->GetSkeleton();
			auto mdlComponentParent = entParent.GetModelComponent();
			auto hMdlParent = mdlComponentParent.valid() ? mdlComponentParent->GetModel() : nullptr;
			if(hMdlParent != nullptr)
			{
				auto *mdlParent = hMdlParent.get();
				Skeleton &skelParent = mdlParent->GetSkeleton();
				auto &bones = skel.GetBones();
				auto &bonesParent = skelParent.GetBones();
				m_attachment->boneMapping.resize(bones.size());
				std::vector<int> &boneMapping = m_attachment->boneMapping;
				for(unsigned int i=0;i<bones.size();i++)
				{
					auto &bone = bones[i];
					bool bExists = false;
					for(unsigned int j=0;j<bonesParent.size();j++)
					{
						auto &boneParent = bonesParent[j];
						if(bone->name == boneParent->name)
						{
							boneMapping[i] = j;
							bExists = true;
							break;
						}
					}
					if(bExists == false)
						boneMapping[i] = -1;
				}
			}
		}
	}
	else if(!m_attachment->boneMapping.empty())
		m_attachment->boneMapping.clear();
}
AttachmentData *BaseAttachableComponent::SetupAttachment(BaseEntity *ent,const AttachmentInfo &attInfo)
{
	if(m_attachment != NULL)
	{
		auto &parent = m_attachment->parent;
		if(parent.valid())
		{
			auto *entParent = parent.get();
			entParent->RemoveChild(*this);
		}
		m_attachment = nullptr;
		if(m_posChangeCallback.IsValid())
			m_posChangeCallback.Remove();
		if(m_rotChangeCallback.IsValid())
			m_rotChangeCallback.Remove();
	}
	if(ent != nullptr)
	{
		auto *pParentComponent = static_cast<BaseParentComponent*>(ent->AddComponent("parent").get());
		if(pParentComponent == nullptr)
		{
			m_attachment = nullptr;
			return nullptr;
		}
		m_attachment = std::make_unique<AttachmentData>();
		m_attachment->parent = pParentComponent->GetHandle<BaseParentComponent>();
		m_attachment->offset = attInfo.offset.has_value() ? *attInfo.offset : Vector3{};
		m_attachment->rotation = attInfo.rotation.has_value() ? *attInfo.rotation : uquat::identity();
		m_attachment->flags = attInfo.flags;
		Vector3 pos = {};
		auto orientation = uquat::identity();
		if((attInfo.flags &FAttachmentMode::PlayerView) != FAttachmentMode::None && ent->IsCharacter())
		{
			auto &charComponent = *ent->GetCharacterComponent();
			pos = charComponent.GetEyePosition();
			orientation = charComponent.GetViewOrientation();
		}
		else
		{
			auto pTrComponentEnt = ent->GetTransformComponent();
			pos = pTrComponentEnt.valid() ? pTrComponentEnt->GetPosition() : Vector3{};
			orientation = pTrComponentEnt.valid() ? pTrComponentEnt->GetOrientation() : uquat::identity();
		}
		auto pTrComponent = GetEntity().GetTransformComponent();
		if((attInfo.flags &FAttachmentMode::SnapToOrigin) != FAttachmentMode::None)
		{
			if(pTrComponent.valid())
			{
				pTrComponent->SetPosition(pos);
				pTrComponent->SetOrientation(orientation);
			}
		}
		else
		{
			auto invRot = uquat::get_inverse(orientation);
			auto offset = (pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{}) -pos;
			uvec::rotate(&offset,invRot);
			if(attInfo.offset.has_value() == false)
				m_attachment->offset = offset;
			if(attInfo.rotation.has_value() == false)
				m_attachment->rotation = uquat::get_inverse(orientation) *(pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity());
		}
		UpdateAttachmentData();
		pParentComponent->AddChild(*this);

		if(pTrComponent.valid())
		{
			// Update local pose (relative to parent) if absolute pose
			// has been changed externally
			auto &posProp = pTrComponent->GetPosProperty();
			m_posChangeCallback = posProp->AddCallback([this](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> newPos) {
				if(umath::is_flag_set(m_stateFlags,StateFlags::UpdatingPosition))
					return;
				auto parentPose = GetParentPose();
				if(parentPose.has_value() == false)
					return;
				auto localOffset = parentPose->GetInverse() *newPos.get();
				auto *attData = GetAttachmentData();
				if(attData == nullptr)
					return;
				attData->offset = localOffset;
			});
			auto &rotProp = pTrComponent->GetOrientationProperty();
			m_rotChangeCallback = rotProp->AddCallback([this](std::reference_wrapper<const Quat> oldRot,std::reference_wrapper<const Quat> newRot) {
				if(umath::is_flag_set(m_stateFlags,StateFlags::UpdatingRotation))
					return;
				auto parentPose = GetParentPose();
				if(parentPose.has_value() == false)
					return;
				auto localRot = parentPose->GetInverse() *newRot.get();
				auto *attData = GetAttachmentData();
				if(attData == nullptr)
					return;
				attData->rotation = localRot;
			});
		}
	}
	return m_attachment.get();
}
AttachmentData *BaseAttachableComponent::AttachToEntity(BaseEntity *ent,const AttachmentInfo &attInfo)
{
	auto *attData = SetupAttachment(ent,attInfo);
	UpdateAttachmentOffset();
	return attData;
}
AttachmentData *BaseAttachableComponent::AttachToBone(BaseEntity *ent,uint32_t boneID,const AttachmentInfo &attInfo)
{
	SetupAttachment(ent,attInfo);
	if(m_attachment == nullptr)
		return nullptr;
	m_attachment->bone = boneID;
	if((attInfo.flags &FAttachmentMode::SnapToOrigin) != FAttachmentMode::None)
	{
		auto *parent = m_attachment->parent.get();
		auto &entParent = parent->GetEntity();
		auto animComponentParent = entParent.GetAnimatedComponent();
		if(animComponentParent.expired())
			return nullptr;
		Vector3 pos;
		auto rot = uquat::identity();
		animComponentParent->GetBonePosition(m_attachment->bone,pos,rot);
		auto pTrComponentParent = entParent.GetTransformComponent();
		if(pTrComponentParent.valid())
			pTrComponentParent->LocalToWorld(&pos,&rot);

		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
		{
			pTrComponent->SetPosition(pos);
			pTrComponent->SetOrientation(rot);
		}
	}
	UpdateAttachmentOffset();
	return m_attachment.get();
}
AttachmentData *BaseAttachableComponent::AttachToBone(BaseEntity *ent,std::string bone,const AttachmentInfo &attInfo)
{
	if(ent == nullptr)
	{
		ClearAttachment();
		return nullptr;
	}
	auto mdlComponent = ent->GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
	{
		SetupAttachment(ent,attInfo);
		return nullptr;
	}
	int boneID = hMdl->LookupBone(bone);
	if(boneID == -1)
	{
		ClearAttachment();
		return nullptr;
	}
	return AttachToBone(ent,boneID,attInfo);
}
AttachmentData *BaseAttachableComponent::AttachToAttachment(BaseEntity *ent,uint32_t attachmentID,const AttachmentInfo &attInfo)
{
	SetupAttachment(ent,attInfo);
	if(m_attachment == NULL)
		return nullptr;
	m_attachment->attachment = attachmentID;
	if((attInfo.flags &FAttachmentMode::SnapToOrigin) != FAttachmentMode::None)
	{
		auto *parent = m_attachment->parent.get();
		auto &entParent = parent->GetEntity();
		Vector3 pos {};
		auto rot = uquat::identity();
		auto mdlCParent = entParent.GetModelComponent();
		if(mdlCParent.valid())
			mdlCParent->GetAttachment(m_attachment->attachment,&pos,&rot);
		auto pTrComponentParent = entParent.GetTransformComponent();
		if(pTrComponentParent.valid())
			pTrComponentParent->LocalToWorld(&pos,&rot);

		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
		{
			pTrComponent->SetPosition(pos);
			pTrComponent->SetOrientation(rot);
		}
	}
	UpdateAttachmentOffset();
	return m_attachment.get();
}
AttachmentData *BaseAttachableComponent::AttachToAttachment(BaseEntity *ent,std::string attachment,const AttachmentInfo &attInfo)
{
	if(ent == nullptr)
	{
		ClearAttachment();
		return nullptr;
	}
	auto pMdlComponent = ent->GetModelComponent();
	if(pMdlComponent.expired())
		return nullptr;
	int attachmentID = pMdlComponent->LookupAttachment(attachment);
	if(attachmentID == -1)
	{
		ClearAttachment();
		return nullptr;
	}
	return AttachToAttachment(ent,attachmentID,attInfo);
}
void BaseAttachableComponent::SetAttachmentFlags(FAttachmentMode flags)
{
	if(m_attachment == NULL)
		return;
	m_attachment->flags = flags;
	UpdateAttachmentData();
}
void BaseAttachableComponent::AddAttachmentFlags(FAttachmentMode flags)
{
	if(m_attachment == NULL)
		return;
	m_attachment->flags |= flags;
	UpdateAttachmentData();
}
void BaseAttachableComponent::RemoveAttachmentFlags(FAttachmentMode flags)
{
	if(m_attachment == NULL)
		return;
	m_attachment->flags &= ~flags;
	UpdateAttachmentData();
}
FAttachmentMode BaseAttachableComponent::GetAttachmentFlags() const
{
	if(m_attachment == NULL)
		return FAttachmentMode::None;
	return m_attachment->flags;
}
bool BaseAttachableComponent::HasAttachmentFlag(FAttachmentMode flag) const
{
	if(m_attachment == NULL)
		return (flag == FAttachmentMode::None) ? true : false;
	return ((m_attachment->flags &flag) == flag) ? true : false;
}

BaseParentComponent *BaseAttachableComponent::GetParent() const
{
	if(m_attachment == NULL || m_attachment->parent.expired())
		return NULL;
	return m_attachment->parent.get();
}
AttachmentData *BaseAttachableComponent::GetAttachmentData() const {return m_attachment.get();}
void BaseAttachableComponent::UpdateViewAttachmentOffset(BaseEntity *ent,pragma::BaseCharacterComponent &pl,Vector3 &pos,Quat &rot,Bool bYawOnly) const
{
	if(bYawOnly == false)
	{
		pos = pl.GetEyePosition();
		rot = pl.GetViewOrientation();
	}
	else
	{
		auto pTrComponentEnt = ent->GetTransformComponent();
		pos = pTrComponentEnt.valid() ? pTrComponentEnt->GetPosition() : Vector3{};
		rot = pTrComponentEnt.valid() ? pTrComponentEnt->GetOrientation() : uquat::identity();

		auto &rotRef = pl.GetOrientationAxesRotation();
		auto viewRot = rotRef *pl.GetViewOrientation();
		auto viewAng = EulerAngles(viewRot);
		auto viewYaw = viewAng.y;

		rot = rotRef *rot;
		auto ang = EulerAngles(rot);
		ang.y = viewYaw;
		rot = uquat::get_inverse(rotRef) *uquat::create(ang);
		/*local viewRot = pl:GetViewRotation()
		viewRot = pl:GetOrientationAxesRotation() *viewRot
		local viewAng = viewRot:ToEulerAngles()
		local viewYaw = viewAng.y
		print(viewYaw)

		rot = pl:GetOrientationAxesRotation() *rot
		local ang = rot:ToEulerAngles()
		ang.y = viewYaw
		rot = pl:GetOrientationAxesRotation():GetInverse() *ang:ToQuaternion()*/
		/*auto &rotRef = pl->GetOrientationAxesRotation();
		rot = rotRef *rot;

		// Set yaw axis to player's view yaw
		auto ang = EulerAngles(rot);
		auto rotPl = pl->GetViewOrientation();
		rotPl = rotRef *rotPl;
		auto angPl = EulerAngles(rotPl);
		ang.y = angPl.y;
		rot = uquat::create(ang);

		rot = uquat::get_inverse(rotRef) *rot;*/
	}
}
std::optional<physics::Transform> BaseAttachableComponent::GetParentPose() const
{
	if(m_attachment == nullptr || m_attachment->parent.expired())
		return {};
	physics::Transform pose {};
	auto *parent = m_attachment->parent.get();
	auto *pAttachableComponentParent = static_cast<BaseAttachableComponent*>(parent->GetEntity().FindComponent("attachable").get());
	if(pAttachableComponentParent != nullptr)
		pAttachableComponentParent->UpdateAttachmentOffset(); // TODO
	auto &entParent = parent->GetEntity();
	if(((m_attachment->flags &FAttachmentMode::PlayerView) != FAttachmentMode::None) && entParent.IsCharacter())
	{
		auto &charComponent = *entParent.GetCharacterComponent();
		Vector3 pos;
		Quat orientation;
		UpdateViewAttachmentOffset(&entParent,charComponent,pos,orientation);
		pose = {pos,orientation};
	}
	else if(((m_attachment->flags &FAttachmentMode::PlayerViewYaw) != FAttachmentMode::None) && entParent.IsCharacter())
	{
		auto &charComponent = *entParent.GetCharacterComponent();
		Vector3 pos;
		Quat orientation;
		UpdateViewAttachmentOffset(&entParent,charComponent,pos,orientation,true);
		pose = {pos,orientation};
	}
	else
	{
		Vector3 pos {};
		auto rot = uquat::identity();
		if(m_attachment->bone != -1)
		{
			auto animComponentParent = entParent.GetAnimatedComponent();
			if(animComponentParent.valid())
				animComponentParent->GetBonePosition(m_attachment->bone,pos,rot);
		}
		else if(m_attachment->attachment != -1)
		{
			auto pMdlCParent = entParent.GetModelComponent();
			if(pMdlCParent.valid())
				pMdlCParent->GetAttachment(m_attachment->attachment,&pos,&rot);
		}
		auto pTrComponentParent = entParent.GetTransformComponent();
		if(pTrComponentParent.valid())
			pTrComponentParent->LocalToWorld(&pos,&rot);
		pose = {pos,rot};
	}
	return pose;
}
void BaseAttachableComponent::UpdateAttachmentOffset()
{
	auto &entThis = GetEntity();
	NetworkState *state = entThis.GetNetworkState();
	Game *game = state->GetGameState();
	auto t = CFloat(game->CurTime());
	if(t <= m_tLastAttachmentUpdate)
		return;
	m_tLastAttachmentUpdate = t;

	auto pose = GetParentPose();
	if(pose.has_value())
	{
		auto *parent = m_attachment->parent.get();
		auto *pAttachableComponentParent = static_cast<BaseAttachableComponent*>(parent->GetEntity().FindComponent("attachable").get());
		if(pAttachableComponentParent != nullptr)
			pAttachableComponentParent->UpdateAttachmentOffset();

		pose->TranslateLocal(m_attachment->offset);
		auto pTrComponent = entThis.GetTransformComponent();
		if(pTrComponent.valid())
		{
			umath::set_flag(m_stateFlags,StateFlags::UpdatingPosition);
			pTrComponent->SetPosition(pose->GetOrigin());
			umath::set_flag(m_stateFlags,StateFlags::UpdatingPosition,false);
		}
		if((m_attachment->flags &FAttachmentMode::PositionOnly) == FAttachmentMode::None)
		{
			if(pTrComponent.valid())
			{
				pose->RotateLocal(m_attachment->rotation);

				umath::set_flag(m_stateFlags,StateFlags::UpdatingRotation);
				pTrComponent->SetOrientation(pose->GetRotation());
				umath::set_flag(m_stateFlags,StateFlags::UpdatingRotation,false);
			}
			if((m_attachment->flags &FAttachmentMode::BoneMerge) != FAttachmentMode::None && !m_attachment->boneMapping.empty())
			{
				auto mdlComponent = entThis.GetModelComponent();
				auto animComponent = entThis.GetAnimatedComponent();
				auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
				if(hMdl != nullptr && animComponent.valid())
				{
					auto *parent = m_attachment->parent.get();
					auto &entParent = parent->GetEntity();

					Skeleton &skel = hMdl->GetSkeleton();
					auto mdlComponentParent = entParent.GetModelComponent();
					auto animComponentParent = entParent.GetAnimatedComponent();
					auto hMdlParent = mdlComponentParent.valid() ? mdlComponentParent->GetModel() : nullptr;
					if(hMdlParent != nullptr && animComponentParent.valid())
					{
						Skeleton &skelParent = hMdlParent->GetSkeleton();
						auto &bones = skel.GetBones();
						auto &bonesParent = skelParent.GetBones();
						for(unsigned int i=0;i<m_attachment->boneMapping.size();i++)
						{
							int boneIdxParent = m_attachment->boneMapping[i];
							if(boneIdxParent != -1)
							{
								auto &bone = bones[i];
								auto &boneParent = bonesParent[boneIdxParent];
								auto *pos = animComponentParent->GetBonePosition(boneParent->ID);
								auto *orientation = animComponentParent->GetBoneRotation(boneParent->ID);
								if(pos != nullptr && orientation != nullptr)
									animComponent->SetBonePosition(bone->ID,*pos,*orientation);
							}
						}
					}
				}
			}
		}
	}

	InvokeEventCallbacks(EVENT_ON_ATTACHMENT_UPDATE);
}

