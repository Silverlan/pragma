// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.constraints.base;

export import :entities.components.base;
export import :physics.constraint;

export namespace pragma {
	constexpr uint32_t SF_CONSTRAINT_START_INACTIVE = 1024;
	constexpr uint32_t SF_CONSTRAINT_DISABLE_COLLISIONS = 2048;

	class DLLNETWORK BasePointConstraintComponent : public BaseEntityComponent {
	  public:
		BasePointConstraintComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		std::vector<util::TSharedHandle<physics::IConstraint>> &GetConstraints();
		virtual void OnRemove() override;
	  protected:
		std::string m_kvSource;
		std::string m_kvTarget;
		CallbackHandle m_cbGameLoaded = {};
		Vector3 m_posTarget = {0.f, 0.f, 0.f};
		std::vector<util::TSharedHandle<physics::IConstraint>> m_constraints;
		bool SetKeyValue(std::string key, std::string val);
		ecs::BaseEntity *GetSourceEntity();
		void GetTargetEntities(std::vector<ecs::BaseEntity *> &entsTgt);
		virtual void InitializeConstraint();
		virtual void InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt);
		virtual void ClearConstraint();
		void OnTurnOn();
		void OnTurnOff();
	};
};
