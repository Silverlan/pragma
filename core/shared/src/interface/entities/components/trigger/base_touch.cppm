// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.triggers.base_touch;

export import :entities.components.base;
export import :physics.contact;
export import :physics.object;
export import :physics.touch;

export {
	namespace pragma {
		struct DLLNETWORK CECanTriggerData : public ComponentEvent {
			CECanTriggerData(ecs::BaseEntity *ent);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			ecs::BaseEntity *entity;
			bool canTrigger = true;
		};
		struct DLLNETWORK CETouchData : public ComponentEvent {
			CETouchData(ecs::BaseEntity &ent);
			virtual void PushArguments(lua::State *l) override;
			ecs::BaseEntity *entity;
		};
		namespace baseTouchComponent {
			REGISTER_COMPONENT_EVENT(EVENT_CAN_TRIGGER)
			REGISTER_COMPONENT_EVENT(EVENT_ON_START_TOUCH)
			REGISTER_COMPONENT_EVENT(EVENT_ON_END_TOUCH)
			REGISTER_COMPONENT_EVENT(EVENT_ON_TRIGGER)
			REGISTER_COMPONENT_EVENT(EVENT_ON_TRIGGER_INITIALIZED)
		}
		class DLLNETWORK BaseTouchComponent : public BaseEntityComponent {
		  public:
			enum class TriggerFlags : uint32_t { None = 0u, Players = 1u, NPCs = Players << 1u, Physics = NPCs << 1u, Everything = Players | NPCs | Physics };
			struct ContactEvent {
				enum class Event : uint8_t { StartTouch = 0, EndTouch };
				EntityHandle contactTarget;
				Event eventType;
			};
			struct TouchInfo {
				physics::PhysTouch touch;
				bool triggered;
			};

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			using BaseEntityComponent::BaseEntityComponent;
			// Only called if collision callbacks are enabled (Entity::SetCollisionCallbacksEnabled(true))
			void StartTouch(ecs::BaseEntity &entOther, physics::PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther);
			void EndTouch(ecs::BaseEntity &entOther, physics::PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther);
			// Only called if collision callbacks and contact reports are enabled (Entity::SetCollisionCallbacksEnabled(true),Entity::SetCollisionContactReportEnabled(true))
			void Contact(const physics::ContactInfo &contactInfo);

			virtual void Initialize() override;
			virtual void OnRemove() override;
			const std::vector<TouchInfo> &GetTouchingInfo() const;
			void Trigger(ecs::BaseEntity &ent);
			void StartTouch(ecs::BaseEntity &ent);
			void EndTouch(ecs::BaseEntity &ent);
			virtual bool CanTrigger(ecs::BaseEntity &ent);
			virtual void OnTrigger(ecs::BaseEntity &ent);
			virtual void OnStartTouch(ecs::BaseEntity &ent);
			virtual void OnEndTouch(ecs::BaseEntity &ent);
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

			void SetTriggerFlags(TriggerFlags flags);
			TriggerFlags GetTriggerFlags() const;

			void EndAllTouch();
			virtual void OnEntitySpawn() override;
			void SetNeverDisablePhysicsCallbacks(bool b);
		  protected:
			void UpdatePhysics();
			bool IsTouching(ecs::BaseEntity &ent) const;

			std::vector<physics::ContactInfo> m_contactReport;

			// Actors that have started touching this actor
			std::vector<TouchInfo> m_touching;
			std::queue<ContactEvent> m_contactEventQueue;

			TriggerFlags m_triggerFlags = TriggerFlags::None;
			bool m_neverDisablePhysicsCallbacks = false;
			BaseTouchComponent(ecs::BaseEntity &ent);
			void OnPhysicsInitialized();
			void UpdateTouch();
			void StartTouch(const physics::PhysTouch &touch);
			void FireStartTouchEvents(TouchInfo &touch, bool isFirstTouch);
			void FireEndTouchEvents(TouchInfo &touch, bool isLastTouch);
			void OnContact(physics::ContactInfo &contact);
			virtual void OnTouch(physics::PhysTouch &touch);
			virtual bool IsTouchEnabled() const;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseTouchComponent::TriggerFlags)
};
