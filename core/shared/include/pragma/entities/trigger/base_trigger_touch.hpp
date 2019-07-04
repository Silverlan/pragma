#ifndef __BASETRIGGERTOUCH_H__
#define __BASETRIGGERTOUCH_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/physics/physcontact.h"
#include "pragma/physics/phystouch.h"
#include <vector>

class PhysObj;
class PhysContact;
struct PhysTouch;
namespace pragma
{
	struct DLLNETWORK CECanTriggerData
		: public ComponentEvent
	{
		CECanTriggerData(BaseEntity *ent,PhysObj *phys);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		BaseEntity *entity;
		PhysObj *physObj;
		bool canTrigger = true;
	};
	struct DLLNETWORK CETouchData
		: public ComponentEvent
	{
		CETouchData(BaseEntity *ent,PhysObj *phys);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity *entity;
		PhysObj *physObj;
	};
	class DLLNETWORK BaseTouchComponent
		: public BaseEntityComponent
	{
	public:
		enum class TriggerFlags : uint32_t
		{
			None = 0u,
			Players = 1u,
			NPCs = Players<<1u,
			Physics = NPCs<<1u,
			Everything = Players | NPCs | Physics
		};

		static ComponentEventId EVENT_CAN_TRIGGER;
		static ComponentEventId EVENT_ON_START_TOUCH;
		static ComponentEventId EVENT_ON_END_TOUCH;
		static ComponentEventId EVENT_ON_TRIGGER;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		using BaseEntityComponent::BaseEntityComponent;
		// Only called if collision callbacks are enabled (Entity::SetCollisionCallbacksEnabled(true))
		void Touch(BaseEntity *entOther,PhysObj *physOther,physics::ICollisionObject *objThis,physics::ICollisionObject *objOther);
		// Only called if collision callbacks and contact reports are enabled (Entity::SetCollisionCallbacksEnabled(true),Entity::SetCollisionContactReportEnabled(true))
		void Contact(const PhysContact &contact);

		virtual void Initialize() override;
		virtual void OnRemove() override;
		void GetTouchingEntities(std::vector<BaseEntity*> &touching);
		const std::vector<PhysTouch> &GetTouchingInfo() const;
		void Trigger(BaseEntity *ent,PhysObj *phys);
		void StartTouch(BaseEntity *ent,PhysObj *phys);
		void EndTouch(BaseEntity *ent,PhysObj *phys);
		virtual bool CanTrigger(BaseEntity *ent,PhysObj *phys);
		virtual void OnTrigger(BaseEntity *ent,PhysObj *phys);
		virtual void OnStartTouch(BaseEntity *ent,PhysObj *phys);
		virtual void OnEndTouch(BaseEntity *ent,PhysObj *phys);
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		bool IsTouching(BaseEntity *ent);

		void SetTriggerFlags(TriggerFlags flags);
		TriggerFlags GetTriggerFlags() const;

		void EndAllTouch();
		virtual void OnEntitySpawn() override;
	protected:
		std::vector<PhysContact> m_contactReport;
		std::vector<PhysTouch> m_touchReport;
		std::vector<PhysTouch> m_touching;
		std::vector<PhysTouch> m_endTouch;
		TriggerFlags m_triggerFlags = TriggerFlags::None;
		BaseTouchComponent(BaseEntity &ent);
		void OnPhysicsInitialized();
		void UpdateTouch();
		void RemoveTouch(BaseEntity *entRemove);
		std::vector<PhysTouch>::iterator RemoveTouch(std::vector<PhysTouch>::iterator it);
		void StartTouch(PhysTouch &touch);
		void OnContact(PhysContact &contact);
		virtual void OnTouch(PhysTouch &touch);
		virtual bool IsTouchEnabled() const;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseTouchComponent::TriggerFlags)

#endif
