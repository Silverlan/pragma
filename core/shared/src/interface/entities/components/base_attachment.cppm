// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_attachment;

export import :entities.components.base;
export import :entities.components.base_character;
export import :entities.parent_info;

export {
	namespace pragma {
		namespace baseAttachmentComponent {
			CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_ATTACHMENT_UPDATE;
		}
		class DLLNETWORK BaseAttachmentComponent : public BaseEntityComponent {
		  public:
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

			enum class StateFlags : uint32_t { None = 0u, UpdatingPosition = 1u, UpdatingRotation = UpdatingPosition << 1u };

			virtual void Initialize() override;
			virtual void OnRemove() override;
			void UpdateAttachmentOffset(bool invokeUpdateEvents = true);

			AttachmentData *AttachToEntity(pragma::ecs::BaseEntity *ent, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToBone(pragma::ecs::BaseEntity *ent, uint32_t boneID, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToBone(pragma::ecs::BaseEntity *ent, std::string bone, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToAttachment(pragma::ecs::BaseEntity *ent, uint32_t attachmentID, const AttachmentInfo &attInfo = {});
			AttachmentData *AttachToAttachment(pragma::ecs::BaseEntity *ent, std::string attachment, const AttachmentInfo &attInfo = {});
			virtual void SetAttachmentFlags(pragma::FAttachmentMode flags);
			void AddAttachmentFlags(pragma::FAttachmentMode flags);
			void RemoveAttachmentFlags(pragma::FAttachmentMode flags);
			pragma::FAttachmentMode GetAttachmentFlags() const;
			bool HasAttachmentFlag(pragma::FAttachmentMode flag) const;
			pragma::ecs::BaseEntity *GetParent() const;
			AttachmentData *GetAttachmentData() const;
			void UpdateAttachmentData(bool bForceReload = false);
			void ClearAttachment();

			std::optional<umath::Transform> GetLocalPose() const;
			void SetLocalPose(const umath::Transform &pose);

			virtual void OnTick(double dt) override;
		  protected:
			BaseAttachmentComponent(pragma::ecs::BaseEntity &ent);
			virtual void OnAttachmentChanged() {}
			virtual AttachmentData *SetupAttachment(pragma::ecs::BaseEntity *ent, const AttachmentInfo &attInfo);
			virtual void UpdateViewAttachmentOffset(pragma::ecs::BaseEntity *ent, pragma::BaseCharacterComponent &pl, Vector3 &pos, Quat &rot, Bool bYawOnly = false) const;
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
		using namespace umath::scoped_enum::bitwise;
	};
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::BaseAttachmentComponent::StateFlags> : std::true_type {};
	}
};
