// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.base;

export import :entities.components.base;
export import :entities.universal_reference;
export import :game.coordinate_system;
export import :game.enums;

export namespace pragma {
	class ConstraintManagerComponent;
	namespace constraintComponent {
		REGISTER_COMPONENT_EVENT(EVENT_APPLY_CONSTRAINT)
		REGISTER_COMPONENT_EVENT(EVENT_ON_DRIVER_CHANGED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_DRIVEN_OBJECT_CHANGED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_ORDER_INDEX_CHANGED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_PARTICIPANTS_FLAGGED_DIRTY)
	}
	class DLLNETWORK ConstraintComponent final : public BaseEntityComponent {
	  public:
		struct DLLNETWORK ConstraintParticipants {
			ComponentHandle<BaseEntityComponent> driverC;
			ComponentHandle<BaseEntityComponent> drivenObjectC;
			ComponentMemberIndex driverPropIdx;
			ComponentMemberIndex drivenObjectPropIdx;
		};

		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		virtual void OnTick(double tDelta) override;

		void SetInfluence(float influence);
		float GetInfluence() const;

		void SetDriver(const EntityUComponentMemberRef &driver);
		const EntityUComponentMemberRef &GetDriver() const;

		void SetDrivenObject(const EntityUComponentMemberRef &drivenObject);
		const EntityUComponentMemberRef &GetDrivenObject() const;

		void SetDriverSpace(CoordinateSpace space);
		CoordinateSpace GetDriverSpace() const;

		void SetDrivenObjectSpace(CoordinateSpace space);
		CoordinateSpace GetDrivenObjectSpace() const;

		void SetOrderIndex(int32_t idx);
		int32_t GetOrderIndex() const;

		void ApplyConstraint();
		void SetDriverEnabled(bool enabled);
		bool HasDriver() const;

		const std::optional<ConstraintParticipants> &GetConstraintParticipants() const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void SetConstraintParticipantsDirty();
		const std::optional<ConstraintParticipants> &UpdateConstraintParticipants();
		friend ConstraintManagerComponent;
		EntityUComponentMemberRef m_driver;
		EntityUComponentMemberRef m_drivenObject;
		CoordinateSpace m_driverSpace = CoordinateSpace::World;
		CoordinateSpace m_drivenObjectSpace = CoordinateSpace::World;
		ComponentHandle<ConstraintManagerComponent> m_curDrivenConstraintManager;
		std::optional<ConstraintParticipants> m_constraintParticipants {};
		bool m_hasDriver = true;
		bool m_registeredWithConstraintManager = false;
		int32_t m_orderIndex = 0;
		float m_influence = 1.f;
	};
	struct DLLNETWORK CEOnConstraintOrderIndexChanged : public ComponentEvent {
		CEOnConstraintOrderIndexChanged(int32_t oldOrderIndex, int32_t newOrderIndex);
		virtual void PushArguments(lua::State *l) override;
		int32_t oldOrderIndex = 0;
		int32_t newOrderIndex = 0;
	};
};
