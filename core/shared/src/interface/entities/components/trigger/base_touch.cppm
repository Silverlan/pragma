// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <vector>
#include <queue>


export module pragma.shared:entities.components.triggers.base_touch;

export import :entities.components.base;
export import :physics.contact;
export import :physics.object;
export import :physics.touch;

export {
	namespace pragma {
		struct DLLNETWORK CECanTriggerData : public ComponentEvent {
			CECanTriggerData(BaseEntity *ent);
			virtual void PushArguments(lua_State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua_State *l) override;
			BaseEntity *entity;
			bool canTrigger = true;
		};
		struct DLLNETWORK CETouchData : public ComponentEvent {
			CETouchData(BaseEntity &ent);
			virtual void PushArguments(lua_State *l) override;
			BaseEntity *entity;
		};
		class DLLNETWORK BaseTouchComponent : public BaseEntityComponent {
		public:
			enum class TriggerFlags : uint32_t { None = 0u, Players = 1u, NPCs = Players << 1u, Physics = NPCs << 1u, Everything = Players | NPCs | Physics };
			struct ContactEvent {
				enum class Event : uint8_t { StartTouch = 0, EndTouch };
				EntityHandle contactTarget;
				Event eventType;
			};
			struct TouchInfo {
				PhysTouch touch;
				bool triggered;
			};

			static ComponentEventId EVENT_CAN_TRIGGER;
			static ComponentEventId EVENT_ON_START_TOUCH;
			static ComponentEventId EVENT_ON_END_TOUCH;
			static ComponentEventId EVENT_ON_TRIGGER;
			static ComponentEventId EVENT_ON_TRIGGER_INITIALIZED;
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			using BaseEntityComponent::BaseEntityComponent;
			// Only called if collision callbacks are enabled (Entity::SetCollisionCallbacksEnabled(true))
			void StartTouch(BaseEntity &entOther, PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther);
			void EndTouch(BaseEntity &entOther, PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther);
			// Only called if collision callbacks and contact reports are enabled (Entity::SetCollisionCallbacksEnabled(true),Entity::SetCollisionContactReportEnabled(true))
			void Contact(const pragma::physics::ContactInfo &contactInfo);

			virtual void Initialize() override;
			virtual void OnRemove() override;
			const std::vector<TouchInfo> &GetTouchingInfo() const;
			void Trigger(BaseEntity &ent);
			void StartTouch(BaseEntity &ent);
			void EndTouch(BaseEntity &ent);
			virtual bool CanTrigger(BaseEntity &ent);
			virtual void OnTrigger(BaseEntity &ent);
			virtual void OnStartTouch(BaseEntity &ent);
			virtual void OnEndTouch(BaseEntity &ent);
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

			void SetTriggerFlags(TriggerFlags flags);
			TriggerFlags GetTriggerFlags() const;

			void EndAllTouch();
			virtual void OnEntitySpawn() override;
			void SetNeverDisablePhysicsCallbacks(bool b);
		protected:
			void UpdatePhysics();
			bool IsTouching(BaseEntity &ent) const;

			std::vector<physics::ContactInfo> m_contactReport;

			// Actors that have started touching this actor
			std::vector<TouchInfo> m_touching;
			std::queue<ContactEvent> m_contactEventQueue;

			TriggerFlags m_triggerFlags = TriggerFlags::None;
			bool m_neverDisablePhysicsCallbacks = false;
			BaseTouchComponent(BaseEntity &ent);
			void OnPhysicsInitialized();
			void UpdateTouch();
			void StartTouch(const PhysTouch &touch);
			void FireStartTouchEvents(TouchInfo &touch, bool isFirstTouch);
			void FireEndTouchEvents(TouchInfo &touch, bool isLastTouch);
			void OnContact(physics::ContactInfo &contact);
			virtual void OnTouch(PhysTouch &touch);
			virtual bool IsTouchEnabled() const;
		};
	};
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseTouchComponent::TriggerFlags)
};
