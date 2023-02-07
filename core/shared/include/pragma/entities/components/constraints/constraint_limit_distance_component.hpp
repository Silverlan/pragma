/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONSTRAINT_LIMIT_DISTANCE_COMPONENT_HPP__
#define __CONSTRAINT_LIMIT_DISTANCE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class ConstraintComponent;
	class DLLNETWORK ConstraintLimitDistanceComponent final : public BaseEntityComponent {
	  public:
		enum class ClampRegion : uint32_t { Inside = 0, Outside, OnSurface };
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLimitDistanceComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetClampRegion(ClampRegion clampRegion);
		ClampRegion GetClampRegion() const;

		void SetDistance(float dist);
		float GetDistance() const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		ClampRegion m_clampRegion = ClampRegion::Inside;
		float m_distSqr = 0.f;
		float m_dist = 0.f;
	};
};

#endif
