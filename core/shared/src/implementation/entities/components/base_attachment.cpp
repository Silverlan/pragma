// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_attachment;

using namespace pragma;

void BaseAttachmentComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseAttachmentComponent::EVENT_ON_ATTACHMENT_UPDATE = registerEvent("ON_ATTACHMENT_UPDATE", ComponentEventInfo::Type::Explicit); }
BaseAttachmentComponent::BaseAttachmentComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseAttachmentComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent("child");
	BindEvent(baseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		if(m_attachment != nullptr && (m_attachment->flags & FAttachmentMode::BoneMerge) != FAttachmentMode::None) {
			static_cast<CEShouldUpdateBones &>(evData.get()).shouldUpdate = true;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
}
util::EventReply BaseAttachmentComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(eventId == baseModelComponent::EVENT_ON_MODEL_CHANGED)
		UpdateAttachmentData(true);
	else if(eventId == baseChildComponent::EVENT_ON_PARENT_CHANGED) {
		if(m_parentModelChanged.IsValid())
			m_parentModelChanged.Remove();
		auto *childC = GetEntity().GetChildComponent();
		auto *parent = childC ? childC->GetParentEntity() : nullptr;
		auto *mdlC = parent ? parent->GetModelComponent() : nullptr;
		if(mdlC) {
			m_parentModelChanged = mdlC->AddEventCallback(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
				UpdateAttachmentData(true);
				return util::EventReply::Unhandled;
			});
		}
	}
	else if(eventId == ecs::baseEntity::EVENT_HANDLE_KEY_VALUE) {
		auto &kvData = static_cast<CEKeyValueData &>(evData);
		if(pragma::string::compare<std::string>(kvData.key, "parent", false) || pragma::string::compare<std::string>(kvData.key, "parentname", false))
			m_kvParent = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	}
	return BaseEntityComponent::HandleEvent(eventId, evData);
}
void BaseAttachmentComponent::OnTick(double dt) { UpdateAttachmentOffset(); }
void BaseAttachmentComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_attachment != nullptr)
		ClearAttachment();

	if(m_parentModelChanged.IsValid())
		m_parentModelChanged.Remove();
}
void BaseAttachmentComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(!m_kvParent.empty()) {
		ecs::EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvParent);
		auto it = entIt.begin();
		if(it != entIt.end()) {
			AttachmentInfo attInfo {};
			attInfo.flags = /*FAttachmentMode::SnapToOrigin | */ FAttachmentMode::UpdateEachFrame;
			AttachToEntity(*it, attInfo);
		}
	}
	UpdateAttachmentData();
}
void BaseAttachmentComponent::ClearAttachment() { AttachToEntity(nullptr); }
std::optional<math::Transform> BaseAttachmentComponent::GetLocalPose() const
{
	auto *attData = GetAttachmentData();
	if(attData == nullptr)
		return {};
	return math::Transform {attData->offset, attData->rotation};
}
void BaseAttachmentComponent::SetLocalPose(const math::Transform &pose)
{
	auto *attData = GetAttachmentData();
	if(attData == nullptr)
		return;
	attData->offset = pose.GetOrigin();
	attData->rotation = pose.GetRotation();
}
void BaseAttachmentComponent::UpdateAttachmentData(bool bForceReload)
{
	if(m_attachment == nullptr)
		return;
	auto *parent = GetEntity().GetParent();
	if(!parent)
		return;
	auto &entParent = *parent;
	if(!((m_attachment->flags & FAttachmentMode::PositionOnly) != FAttachmentMode::None) && (m_attachment->flags & FAttachmentMode::BoneMerge) != FAttachmentMode::None) {
		if(!m_attachment->boneMapping.empty()) {
			if(bForceReload == true)
				m_attachment->boneMapping.clear();
			else
				return;
		}
		auto &hMdl = GetEntity().GetModel();
		if(hMdl != nullptr) {
			auto &skel = hMdl->GetSkeleton();
			auto &hMdlParent = entParent.GetModel();
			if(hMdlParent != nullptr) {
				auto *mdlParent = hMdlParent.get();
				auto &skelParent = mdlParent->GetSkeleton();
				auto &bones = skel.GetBones();
				auto &bonesParent = skelParent.GetBones();
				m_attachment->boneMapping.resize(bones.size());
				std::vector<int> &boneMapping = m_attachment->boneMapping;
				for(unsigned int i = 0; i < bones.size(); i++) {
					auto &bone = bones[i];
					bool bExists = false;
					for(unsigned int j = 0; j < bonesParent.size(); j++) {
						auto &boneParent = bonesParent[j];
						if(bone->name == boneParent->name) {
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
AttachmentData *BaseAttachmentComponent::SetupAttachment(ecs::BaseEntity *ent, const AttachmentInfo &attInfo)
{
	if(m_parentModelChanged.IsValid())
		m_parentModelChanged.Remove();
	if(m_attachment != nullptr) {
		m_attachment = nullptr;
		SetTickPolicy(TickPolicy::Never);
		if(m_poseChangeCallback.IsValid())
			m_poseChangeCallback.Remove();
	}
	GetEntity().SetParent(ent);
	if(ent != nullptr) {
		m_attachment = std::make_unique<AttachmentData>();
		SetTickPolicy(TickPolicy::Always);
		m_attachment->offset = attInfo.offset.has_value() ? *attInfo.offset : Vector3 {};
		m_attachment->rotation = attInfo.rotation.has_value() ? *attInfo.rotation : uquat::identity();
		m_attachment->flags = attInfo.flags;
		Vector3 pos = {};
		auto orientation = uquat::identity();
		if((attInfo.flags & FAttachmentMode::PlayerView) != FAttachmentMode::None && ent->IsCharacter()) {
			auto &charComponent = *ent->GetCharacterComponent();
			pos = charComponent.GetEyePosition();
			orientation = charComponent.GetViewOrientation();
		}
		else {
			auto pTrComponentEnt = ent->GetTransformComponent();
			pos = pTrComponentEnt ? pTrComponentEnt->GetPosition() : Vector3 {};
			orientation = pTrComponentEnt ? pTrComponentEnt->GetRotation() : uquat::identity();
		}
		auto pTrComponent = GetEntity().GetTransformComponent();
		if((attInfo.flags & FAttachmentMode::SnapToOrigin) != FAttachmentMode::None) {
			if(pTrComponent) {
				pTrComponent->SetPosition(pos);
				pTrComponent->SetRotation(orientation);
			}
		}
		else {
			auto invRot = uquat::get_inverse(orientation);
			auto offset = (pTrComponent ? pTrComponent->GetPosition() : Vector3 {}) - pos;
			uvec::rotate(&offset, invRot);
			if(attInfo.offset.has_value() == false)
				m_attachment->offset = offset;
			if(attInfo.rotation.has_value() == false)
				m_attachment->rotation = uquat::get_inverse(orientation) * (pTrComponent ? pTrComponent->GetRotation() : uquat::identity());
		}
		UpdateAttachmentData();

		if(pTrComponent) {
			// Update local pose (relative to parent) if absolute pose
			// has been changed externally
			if((m_attachment->flags & FAttachmentMode::ForceInPlace) == FAttachmentMode::ForceInPlace) {
				if(m_poseChangeCallback.IsValid())
					m_poseChangeCallback.Remove();
			}
			else {
				m_poseChangeCallback = pTrComponent->AddEventCallback(baseTransformComponent::EVENT_ON_POSE_CHANGED, [this, pTrComponent](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
					auto changeFlags = static_cast<CEOnPoseChanged &>(evData.get()).changeFlags;
					if(math::is_flag_set(changeFlags, TransformChangeFlags::PositionChanged) && math::is_flag_set(m_attachment->flags, FAttachmentMode::ForceTranslationInPlace) == false) {
						if(!math::is_flag_set(m_stateFlags, StateFlags::UpdatingPosition)) {
							auto parentPose = GetParentPose();
							if(parentPose.has_value()) {
								auto localOffset = parentPose->GetInverse() * pTrComponent->GetPosition();
								auto *attData = GetAttachmentData();
								if(attData)
									attData->offset = localOffset;
							}
						}
					}
					if(math::is_flag_set(changeFlags, TransformChangeFlags::RotationChanged) && math::is_flag_set(m_attachment->flags, FAttachmentMode::ForceRotationInPlace) == false) {
						if(!math::is_flag_set(m_stateFlags, StateFlags::UpdatingRotation)) {
							auto parentPose = GetParentPose();
							if(parentPose.has_value()) {
								auto localRot = parentPose->GetInverse() * pTrComponent->GetRotation();
								auto *attData = GetAttachmentData();
								if(attData)
									attData->rotation = localRot;
							}
						}
					}
					return util::EventReply::Unhandled;
				});
			}
		}
	}
	OnAttachmentChanged();
	return m_attachment.get();
}
AttachmentData *BaseAttachmentComponent::AttachToEntity(ecs::BaseEntity *ent, const AttachmentInfo &attInfo)
{
	auto *attData = SetupAttachment(ent, attInfo);
	UpdateAttachmentOffset();
	return attData;
}
AttachmentData *BaseAttachmentComponent::AttachToBone(ecs::BaseEntity *ent, uint32_t boneID, const AttachmentInfo &attInfo)
{
	SetupAttachment(ent, attInfo);
	if(m_attachment == nullptr)
		return nullptr;
	m_attachment->bone = boneID;
	if((attInfo.flags & FAttachmentMode::SnapToOrigin) != FAttachmentMode::None) {
		auto *parent = GetEntity().GetParent();
		if(!parent)
			return nullptr;
		auto &entParent = *parent;
		auto animComponentParent = entParent.GetAnimatedComponent();
		if(animComponentParent.expired())
			return nullptr;
		Vector3 pos;
		auto rot = uquat::identity();
		animComponentParent->GetBonePose(m_attachment->bone, &pos, &rot);
		auto pTrComponentParent = entParent.GetTransformComponent();
		if(pTrComponentParent)
			pTrComponentParent->LocalToWorld(&pos, &rot);

		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent) {
			pTrComponent->SetPosition(pos);
			pTrComponent->SetRotation(rot);
		}
	}
	UpdateAttachmentOffset();
	return m_attachment.get();
}
AttachmentData *BaseAttachmentComponent::AttachToBone(ecs::BaseEntity *ent, std::string bone, const AttachmentInfo &attInfo)
{
	if(ent == nullptr) {
		ClearAttachment();
		return nullptr;
	}
	auto &hMdl = ent->GetModel();
	if(hMdl == nullptr) {
		SetupAttachment(ent, attInfo);
		return nullptr;
	}
	int boneID = hMdl->LookupBone(bone);
	if(boneID == -1) {
		ClearAttachment();
		return nullptr;
	}
	return AttachToBone(ent, boneID, attInfo);
}
AttachmentData *BaseAttachmentComponent::AttachToAttachment(ecs::BaseEntity *ent, uint32_t attachmentID, const AttachmentInfo &attInfo)
{
	SetupAttachment(ent, attInfo);
	if(m_attachment == nullptr)
		return nullptr;
	m_attachment->attachment = attachmentID;
	if((attInfo.flags & FAttachmentMode::SnapToOrigin) != FAttachmentMode::None) {
		auto *parent = GetEntity().GetParent();
		if(!parent)
			return nullptr;
		auto &entParent = *parent;
		Vector3 pos {};
		auto rot = uquat::identity();
		auto mdlCParent = entParent.GetModelComponent();
		if(mdlCParent)
			mdlCParent->GetAttachment(m_attachment->attachment, &pos, &rot);
		auto pTrComponentParent = entParent.GetTransformComponent();
		if(pTrComponentParent)
			pTrComponentParent->LocalToWorld(&pos, &rot);

		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent) {
			pTrComponent->SetPosition(pos);
			pTrComponent->SetRotation(rot);
		}
	}
	UpdateAttachmentOffset();
	return m_attachment.get();
}
AttachmentData *BaseAttachmentComponent::AttachToAttachment(ecs::BaseEntity *ent, std::string attachment, const AttachmentInfo &attInfo)
{
	if(ent == nullptr) {
		ClearAttachment();
		return nullptr;
	}
	auto pMdlComponent = ent->GetModelComponent();
	if(!pMdlComponent)
		return nullptr;
	int attachmentID = pMdlComponent->LookupAttachment(attachment);
	if(attachmentID == -1) {
		ClearAttachment();
		return nullptr;
	}
	return AttachToAttachment(ent, attachmentID, attInfo);
}
void BaseAttachmentComponent::SetAttachmentFlags(FAttachmentMode flags)
{
	if(m_attachment == nullptr)
		return;
	m_attachment->flags = flags;
	UpdateAttachmentData();
}
void BaseAttachmentComponent::AddAttachmentFlags(FAttachmentMode flags)
{
	if(m_attachment == nullptr)
		return;
	m_attachment->flags |= flags;
	UpdateAttachmentData();
}
void BaseAttachmentComponent::RemoveAttachmentFlags(FAttachmentMode flags)
{
	if(m_attachment == nullptr)
		return;
	m_attachment->flags &= ~flags;
	UpdateAttachmentData();
}
FAttachmentMode BaseAttachmentComponent::GetAttachmentFlags() const
{
	if(m_attachment == nullptr)
		return FAttachmentMode::None;
	return m_attachment->flags;
}
bool BaseAttachmentComponent::HasAttachmentFlag(FAttachmentMode flag) const
{
	if(m_attachment == nullptr)
		return (flag == FAttachmentMode::None) ? true : false;
	return ((m_attachment->flags & flag) == flag) ? true : false;
}

ecs::BaseEntity *BaseAttachmentComponent::GetParent() const { return GetEntity().GetParent(); }
AttachmentData *BaseAttachmentComponent::GetAttachmentData() const { return m_attachment.get(); }
void BaseAttachmentComponent::UpdateViewAttachmentOffset(ecs::BaseEntity *ent, BaseCharacterComponent &pl, Vector3 &pos, Quat &rot, Bool bYawOnly) const
{
	if(bYawOnly == false) {
		pos = pl.GetEyePosition();
		rot = pl.GetViewOrientation();
	}
	else {
		auto pTrComponentEnt = ent->GetTransformComponent();
		pos = pTrComponentEnt ? pTrComponentEnt->GetPosition() : Vector3 {};
		rot = pTrComponentEnt ? pTrComponentEnt->GetRotation() : uquat::identity();

		auto rotRef = pl.GetOrientationAxesRotation();
		auto viewRot = rotRef * pl.GetViewOrientation();
		auto viewAng = EulerAngles(viewRot);
		auto viewYaw = viewAng.y;

		rot = rotRef * rot;
		auto ang = EulerAngles(rot);
		ang.y = viewYaw;
		rot = uquat::get_inverse(rotRef) * uquat::create(ang);
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
std::optional<math::Transform> BaseAttachmentComponent::GetParentPose() const
{
	if(m_attachment == nullptr)
		return {};
	auto *parent = GetEntity().GetParent();
	if(!parent)
		return {};
	math::Transform pose {};
	auto *pAttachableComponentParent = static_cast<BaseAttachmentComponent *>(parent->FindComponent("attachment").get());
	if(pAttachableComponentParent != nullptr)
		pAttachableComponentParent->UpdateAttachmentOffset(); // TODO
	auto &entParent = *parent;
	if(((m_attachment->flags & FAttachmentMode::PlayerView) != FAttachmentMode::None) && entParent.IsCharacter()) {
		auto &charComponent = *entParent.GetCharacterComponent();
		Vector3 pos;
		Quat orientation;
		UpdateViewAttachmentOffset(&entParent, charComponent, pos, orientation);
		pose = {pos, orientation};
	}
	else if(((m_attachment->flags & FAttachmentMode::PlayerViewYaw) != FAttachmentMode::None) && entParent.IsCharacter()) {
		auto &charComponent = *entParent.GetCharacterComponent();
		Vector3 pos;
		Quat orientation;
		UpdateViewAttachmentOffset(&entParent, charComponent, pos, orientation, true);
		pose = {pos, orientation};
	}
	else {
		Vector3 pos {};
		auto rot = uquat::identity();
		if(m_attachment->bone != -1) {
			auto animComponentParent = entParent.GetAnimatedComponent();
			if(animComponentParent.valid())
				animComponentParent->GetBonePose(m_attachment->bone, &pos, &rot);
		}
		else if(m_attachment->attachment != -1) {
			auto pMdlCParent = entParent.GetModelComponent();
			if(pMdlCParent)
				pMdlCParent->GetAttachment(m_attachment->attachment, &pos, &rot);
		}
		auto pTrComponentParent = entParent.GetTransformComponent();
		if(pTrComponentParent)
			pTrComponentParent->LocalToWorld(&pos, &rot);
		pose = {pos, rot};
	}
	return pose;
}
void BaseAttachmentComponent::UpdateAttachmentOffset(bool invokeUpdateEvents)
{
	auto &entThis = GetEntity();
	auto *state = entThis.GetNetworkState();
	Game *game = state->GetGameState();
	auto t = CFloat(game->CurTime());
	if(t <= m_tLastAttachmentUpdate)
		return;
	m_tLastAttachmentUpdate = t;

	auto pose = GetParentPose();
	auto *parent = GetEntity().GetParent();
	if(pose.has_value() && parent) {
		auto *pAttachableComponentParent = static_cast<BaseAttachmentComponent *>(parent->FindComponent("attachment").get());
		if(pAttachableComponentParent != nullptr)
			pAttachableComponentParent->UpdateAttachmentOffset();

		pose->TranslateLocal(m_attachment->offset);
		auto pTrComponent = entThis.GetTransformComponent();
		if(pTrComponent) {
			math::set_flag(m_stateFlags, StateFlags::UpdatingPosition);
			pTrComponent->SetPosition(pose->GetOrigin());
			math::set_flag(m_stateFlags, StateFlags::UpdatingPosition, false);
		}
		if((m_attachment->flags & FAttachmentMode::PositionOnly) == FAttachmentMode::None) {
			if(pTrComponent) {
				pose->RotateLocal(m_attachment->rotation);

				math::set_flag(m_stateFlags, StateFlags::UpdatingRotation);
				pTrComponent->SetRotation(pose->GetRotation());
				math::set_flag(m_stateFlags, StateFlags::UpdatingRotation, false);
			}
			if((m_attachment->flags & FAttachmentMode::BoneMerge) != FAttachmentMode::None && !m_attachment->boneMapping.empty()) {
				auto mdlComponent = entThis.GetModelComponent();
				auto animComponent = entThis.GetAnimatedComponent();
				auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
				if(hMdl != nullptr && animComponent.valid()) {
					auto &entParent = *parent;

					auto &skel = hMdl->GetSkeleton();
					auto mdlComponentParent = entParent.GetModelComponent();
					auto animComponentParent = entParent.GetAnimatedComponent();
					auto hMdlParent = mdlComponentParent ? mdlComponentParent->GetModel() : nullptr;
					if(hMdlParent != nullptr && animComponentParent.valid()) {
						auto &skelParent = hMdlParent->GetSkeleton();
						auto &bones = skel.GetBones();
						auto &bonesParent = skelParent.GetBones();
						for(unsigned int i = 0; i < m_attachment->boneMapping.size(); i++) {
							int boneIdxParent = m_attachment->boneMapping[i];
							if(boneIdxParent != -1) {
								auto &bone = bones[i];
								auto &boneParent = bonesParent[boneIdxParent];
								Vector3 pos;
								Quat rot;
								if(animComponentParent->GetBonePose(boneParent->ID, &pos, &rot))
									animComponent->SetBonePose(bone->ID, &pos, &rot);
							}
						}
					}
				}
			}
		}
	}
	if(invokeUpdateEvents)
		InvokeEventCallbacks(baseAttachmentComponent::EVENT_ON_ATTACHMENT_UPDATE);
}
