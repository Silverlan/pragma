// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_attachment;

export import :entities.components.base;
export import :entities.components.base_character;
export import :entities.parent_info;

export {
	namespace pragma {
		namespace baseAttachmentComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_ATTACHMENT_UPDATE);
		}
		class DLLNETWORK BaseAttachmentComponent : public BaseEntityComponent {
		  public:
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

			enum class StateFlags : uint32_t { None = 0u, UpdatingPosition = 1u, UpdatingRotation = UpdatingPosition << 1u };

			virtual void Initialize() override;
			virtual void OnRemove() override;
			void UpdateAttachmentOffset(bool invokeUpdateEvents = true);

			AttachmentData *AttachToEntity(ecs::BaseEntity *ent, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToBone(ecs::BaseEntity *ent, uint32_t boneID, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToBone(ecs::BaseEntity *ent, std::string bone, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToAttachment(ecs::BaseEntity *ent, uint32_t attachmentID, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToAttachment(ecs::BaseEntity *ent, std::string attachment, const AttachmentInfo &attInfo = {});
			virtual void SetAttachmentFlags(FAttachmentMode flags);
			void AddAttachmentFlags(FAttachmentMode flags);
			void RemoveAttachmentFlags(FAttachmentMode flags);
			FAttachmentMode GetAttachmentFlags() const;
			bool HasAttachmentFlag(FAttachmentMode flag) const;
			ecs::BaseEntity *GetParent() const;
			AttachmentData *GetAttachmentData() const;
			void UpdateAttachmentData(bool bForceReload = false);
			void ClearAttachment();

			std::optional<math::Transform> GetLocalPose() const;
			void SetLocalPose(const math::Transform &pose);

			virtual void OnTick(double dt) override;
		  protected:
			BaseAttachmentComponent(ecs::BaseEntity &ent);
			virtual void OnAttachmentChanged() {}
			virtual AttachmentData *SetupAttachment(ecs::BaseEntity *ent, const AttachmentInfo &attInfo);
			virtual void UpdateViewAttachmentOffset(ecs::BaseEntity *ent, BaseCharacterComponent &pl, Vector3 &pos, Quat &rot, Bool bYawOnly = false) const;
			virtual void OnEntitySpawn() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
			std::optional<math::Transform> GetParentPose() const;

			StateFlags m_stateFlags = StateFlags::None;
			float m_tLastAttachmentUpdate = 0.f;
			std::unique_ptr<AttachmentData> m_attachment = nullptr;
			CallbackHandle m_poseChangeCallback = {};
			CallbackHandle m_parentModelChanged {};
			std::string m_kvParent;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseAttachmentComponent::StateFlags)
};
