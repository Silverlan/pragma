// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.attachment;
import :game;
import :networking.util;

using namespace pragma;

void CAttachmentComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CAttachmentComponent::ReceiveData(NetPacket &packet)
{
	auto bParent = packet->Read<Bool>();
	if(bParent == true) {
		auto att = packet->Read<int>();
		auto bone = packet->Read<int>();
		auto flags = packet->Read<FAttachmentMode>();
		auto offset = packet->Read<Vector3>();
		auto rot = packet->Read<Quat>();
		auto bBoneMapping = packet->Read<Bool>();
		std::vector<int> boneMapping;
		if(bBoneMapping == true) {
			auto numBoneMapping = packet->Read<UInt32>();
			boneMapping.reserve(numBoneMapping);
			for(UInt32 i = 0; i < numBoneMapping; i++) {
				auto boneId = packet->Read<int>();
				boneMapping.push_back(boneId);
			}
		}

		auto hThis = GetHandle();
		networking::read_unique_entity(packet, [this, hThis, att, bone, flags, offset, rot, bBoneMapping, boneMapping](ecs::BaseEntity *ent) {
			if(hThis.expired())
				return;
			if(m_attachment == nullptr)
				m_attachment = std::make_unique<AttachmentData>();
			m_attachment->attachment = att;
			m_attachment->bone = bone;
			m_attachment->flags = flags;
			m_attachment->offset = offset;
			m_attachment->rotation = rot;
			m_attachment->boneMapping = boneMapping;
		});
	}
}

void CAttachmentComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseAttachmentComponent)); }

void CAttachmentComponent::UpdateViewAttachmentOffset(ecs::BaseEntity *ent, BaseCharacterComponent &pl, Vector3 &pos, Quat &rot, Bool bYawOnly) const
{
	auto *scene = get_cgame()->GetRenderScene<CSceneComponent>();
	auto cam = scene ? scene->GetActiveCamera() : pragma::ComponentHandle<CCameraComponent> {};
	if(cam.expired())
		return;
	auto trComponent = cam->GetEntity().GetTransformComponent();
	auto forward = trComponent ? trComponent->GetForward() : uvec::PRM_FORWARD;
	auto up = trComponent ? trComponent->GetUp() : uvec::PRM_UP;
	if(bYawOnly == false) {
		pos = cam->GetEntity().GetPosition();
		auto right = uvec::cross(forward, up);
		rot = uquat::create(forward, right, up);
	}
	else {
		/*BaseEntity::UpdateViewParentOffset(ent,pl,pos,rot,bYawOnly);
		auto &posCam = cam->GetPos();
		pos.x = posCam.x;
		pos.z = posCam.z;*/

		auto pTrComponentEnt = ent->GetTransformComponent();
		if(pTrComponentEnt) {
			pos = pTrComponentEnt->GetPosition(); //cam->GetPos();
			rot = pTrComponentEnt->GetRotation(); //cam->GetRotation();
		}

		auto rotRef = pl.GetOrientationAxesRotation();
		auto viewRot = rotRef * cam->GetEntity().GetRotation(); //pl->GetViewOrientation();
		auto viewAng = EulerAngles(viewRot);
		auto viewYaw = viewAng.y;

		rot = rotRef * rot;
		auto ang = EulerAngles(rot);
		ang.y = viewYaw;
		rot = uquat::get_inverse(rotRef) * uquat::create(ang);

		/*auto ang = ent->GetAngles();
		ang.y = pl->GetViewAngles().y;
		rot = uquat::create(ang);*/
	}
}
