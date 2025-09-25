// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/glmutil.h>
#include "pragma/entities/components/basetoggle.h"
#include "pragma/physics/constraint.hpp"
#include <string>
#include <vector>

#define SF_CONSTRAINT_START_INACTIVE 1024
#define SF_CONSTRAINT_DISABLE_COLLISIONS 2048

export module pragma.shared:entities.components.point.constraints.base;

export namespace pragma {
	class DLLNETWORK BasePointConstraintComponent : public BaseEntityComponent {
	  public:
		BasePointConstraintComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
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
		BaseEntity *GetSourceEntity();
		void GetTargetEntities(std::vector<BaseEntity *> &entsTgt);
		virtual void InitializeConstraint();
		virtual void InitializeConstraint(BaseEntity *src, BaseEntity *tgt);
		virtual void ClearConstraint();
		void OnTurnOn();
		void OnTurnOff();
	};
};
