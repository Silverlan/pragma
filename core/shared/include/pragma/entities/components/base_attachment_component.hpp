/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_ATTACHMENT_COMPONENT_HPP__
#define __BASE_ATTACHMENT_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/parentmode.h"
#include "pragma/entities/parentinfo.h"
#include "pragma/entities/entity_uuid_ref.hpp"
#include <sharedutils/property/util_property.hpp>

struct AttachmentData;
struct AttachmentInfo;
namespace pragma {
	class ParentComponent;
	class BaseChildComponent;
	class DLLNETWORK BaseAttachmentComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_ATTACHMENT_UPDATE;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		enum class StateFlags : uint32_t { None = 0u, UpdatingPosition = 1u, UpdatingRotation = UpdatingPosition << 1u };

		virtual void Initialize() override;
		virtual void OnRemove() override;
		void UpdateAttachmentOffset(bool invokeUpdateEvents = true);

		AttachmentData *AttachToEntity(BaseEntity *ent, const AttachmentInfo &attInfo = {});
		AttachmentData *AttachToBone(BaseEntity *ent, uint32_t boneID, const AttachmentInfo &attInfo = {});
		AttachmentData *AttachToBone(BaseEntity *ent, std::string bone, const AttachmentInfo &attInfo = {});
		AttachmentData *AttachToAttachment(BaseEntity *ent, uint32_t attachmentID, const AttachmentInfo &attInfo = {});
		AttachmentData *AttachToAttachment(BaseEntity *ent, std::string attachment, const AttachmentInfo &attInfo = {});
		virtual void SetAttachmentFlags(FAttachmentMode flags);
		void AddAttachmentFlags(FAttachmentMode flags);
		void RemoveAttachmentFlags(FAttachmentMode flags);
		FAttachmentMode GetAttachmentFlags() const;
		bool HasAttachmentFlag(FAttachmentMode flag) const;
		BaseEntity *GetParent() const;
		AttachmentData *GetAttachmentData() const;
		void UpdateAttachmentData(bool bForceReload = false);
		void ClearAttachment();

		std::optional<umath::Transform> GetLocalPose() const;
		void SetLocalPose(const umath::Transform &pose);

		virtual void OnTick(double dt) override;
	  protected:
		BaseAttachmentComponent(BaseEntity &ent);
		virtual void OnAttachmentChanged() {}
		virtual AttachmentData *SetupAttachment(BaseEntity *ent, const AttachmentInfo &attInfo);
		virtual void UpdateViewAttachmentOffset(BaseEntity *ent, pragma::BaseCharacterComponent &pl, Vector3 &pos, Quat &rot, Bool bYawOnly = false) const;
		virtual void OnEntitySpawn() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		std::optional<umath::Transform> GetParentPose() const;

		StateFlags m_stateFlags = StateFlags::None;
		float m_tLastAttachmentUpdate = 0.f;
		std::unique_ptr<AttachmentData> m_attachment = nullptr;
		CallbackHandle m_poseChangeCallback = {};
		CallbackHandle m_parentModelChanged {};
		std::string m_kvParent;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseAttachmentComponent::StateFlags)

#endif
