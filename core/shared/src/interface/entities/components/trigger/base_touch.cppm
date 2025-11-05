// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"



export module pragma.shared:entities.components.triggers.base_touch;

export import :entities.components.base;
export import :physics.contact;
export import :physics.object;
export import :physics.touch;

export {
	namespace pragma {
		struct DLLNETWORK CECanTriggerData : public ComponentEvent {
			CECanTriggerData(pragma::ecs::BaseEntity *ent);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			pragma::ecs::BaseEntity *entity;
			bool canTrigger = true;
		};
		struct DLLNETWORK CETouchData : public ComponentEvent {
			CETouchData(pragma::ecs::BaseEntity &ent);
			virtual void PushArguments(lua::State *l) override;
			pragma::ecs::BaseEntity *entity;
		};
				namespace baseTouchComponent {
			STATIC_DLL_COMPAT ComponentEventId EVENT_CAN_TRIGGER;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_START_TOUCH;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_END_TOUCH;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_TRIGGER;
			STATIC_DLL_COMPAT ComponentEventId EVENT_ON_TRIGGER_INITIALIZED;
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
				PhysTouch touch;
				bool triggered;
			};

			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			using BaseEntityComponent::BaseEntityComponent;
			// Only called if collision callbacks are enabled (Entity::SetCollisionCallbacksEnabled(true))
			void StartTouch(pragma::ecs::BaseEntity &entOther, pragma::physics::PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther);
			void EndTouch(pragma::ecs::BaseEntity &entOther, pragma::physics::PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther);
			// Only called if collision callbacks and contact reports are enabled (Entity::SetCollisionCallbacksEnabled(true),Entity::SetCollisionContactReportEnabled(true))
			void Contact(const pragma::physics::ContactInfo &contactInfo);

			virtual void Initialize() override;
			virtual void OnRemove() override;
			const std::vector<TouchInfo> &GetTouchingInfo() const;
			void Trigger(pragma::ecs::BaseEntity &ent);
			void StartTouch(pragma::ecs::BaseEntity &ent);
			void EndTouch(pragma::ecs::BaseEntity &ent);
			virtual bool CanTrigger(pragma::ecs::BaseEntity &ent);
			virtual void OnTrigger(pragma::ecs::BaseEntity &ent);
			virtual void OnStartTouch(pragma::ecs::BaseEntity &ent);
			virtual void OnEndTouch(pragma::ecs::BaseEntity &ent);
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

			void SetTriggerFlags(TriggerFlags flags);
			TriggerFlags GetTriggerFlags() const;

			void EndAllTouch();
			virtual void OnEntitySpawn() override;
			void SetNeverDisablePhysicsCallbacks(bool b);
		protected:
			void UpdatePhysics();
			bool IsTouching(pragma::ecs::BaseEntity &ent) const;

			std::vector<physics::ContactInfo> m_contactReport;

			// Actors that have started touching this actor
			std::vector<TouchInfo> m_touching;
			std::queue<ContactEvent> m_contactEventQueue;

			TriggerFlags m_triggerFlags = TriggerFlags::None;
			bool m_neverDisablePhysicsCallbacks = false;
			BaseTouchComponent(pragma::ecs::BaseEntity &ent);
			void OnPhysicsInitialized();
			void UpdateTouch();
			void StartTouch(const PhysTouch &touch);
			void FireStartTouchEvents(TouchInfo &touch, bool isFirstTouch);
			void FireEndTouchEvents(TouchInfo &touch, bool isLastTouch);
			void OnContact(physics::ContactInfo &contact);
			virtual void OnTouch(PhysTouch &touch);
			virtual bool IsTouchEnabled() const;
		};
        using namespace umath::scoped_enum::bitwise;
	};
    namespace umath::scoped_enum::bitwise {
        template<>
        struct enable_bitwise_operators<pragma::BaseTouchComponent::TriggerFlags> : std::true_type {};
    }
};
