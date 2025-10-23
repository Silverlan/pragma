// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.constraints.base;

export import :entities.components.base;
export import :entities.universal_reference;
export import :game.coordinate_system;
export import :game.enums;

export namespace pragma {
	class ConstraintManagerComponent;
	class DLLNETWORK ConstraintComponent final : public BaseEntityComponent {
	  public:
		struct DLLNETWORK ConstraintParticipants {
			pragma::ComponentHandle<pragma::BaseEntityComponent> driverC;
			pragma::ComponentHandle<pragma::BaseEntityComponent> drivenObjectC;
			pragma::ComponentMemberIndex driverPropIdx;
			pragma::ComponentMemberIndex drivenObjectPropIdx;
		};

		static ComponentEventId EVENT_APPLY_CONSTRAINT;
		static ComponentEventId EVENT_ON_DRIVER_CHANGED;
		static ComponentEventId EVENT_ON_DRIVEN_OBJECT_CHANGED;
		static ComponentEventId EVENT_ON_ORDER_INDEX_CHANGED;
		static ComponentEventId EVENT_ON_PARTICIPANTS_FLAGGED_DIRTY;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		virtual void OnTick(double tDelta) override;

		void SetInfluence(float influence);
		float GetInfluence() const;

		void SetDriver(const pragma::EntityUComponentMemberRef &driver);
		const pragma::EntityUComponentMemberRef &GetDriver() const;

		void SetDrivenObject(const pragma::EntityUComponentMemberRef &drivenObject);
		const pragma::EntityUComponentMemberRef &GetDrivenObject() const;

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

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void SetConstraintParticipantsDirty();
		const std::optional<ConstraintParticipants> &UpdateConstraintParticipants();
		friend ConstraintManagerComponent;
		pragma::EntityUComponentMemberRef m_driver;
		pragma::EntityUComponentMemberRef m_drivenObject;
		CoordinateSpace m_driverSpace = CoordinateSpace::World;
		CoordinateSpace m_drivenObjectSpace = CoordinateSpace::World;
		pragma::ComponentHandle<ConstraintManagerComponent> m_curDrivenConstraintManager;
		std::optional<ConstraintComponent::ConstraintParticipants> m_constraintParticipants {};
		bool m_hasDriver = true;
		bool m_registeredWithConstraintManager = false;
		int32_t m_orderIndex = 0;
		float m_influence = 1.f;
	};
	struct DLLNETWORK CEOnConstraintOrderIndexChanged : public ComponentEvent {
		CEOnConstraintOrderIndexChanged(int32_t oldOrderIndex, int32_t newOrderIndex);
		virtual void PushArguments(lua_State *l) override;
		int32_t oldOrderIndex = 0;
		int32_t newOrderIndex = 0;
	};
};
