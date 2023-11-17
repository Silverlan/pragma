/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONSTRAINT_MANAGER_COMPONENT_HPP__
#define __CONSTRAINT_MANAGER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class ConstraintComponent;
	class DLLNETWORK ConstraintManagerComponent final : public BaseEntityComponent {
	  public:
		struct DLLNETWORK ConstraintInfo {
			ConstraintComponent *constraint = nullptr;

			ConstraintComponent *operator*() { return constraint; }
			const ConstraintComponent *operator*() const { return constraint; }
			ConstraintComponent *operator->() { return constraint; }
			const ConstraintComponent *operator->() const { return constraint; }
		};
		enum class CoordinateSpace : uint8_t { World = umath::to_integral(umath::CoordinateSpace::World), Local = umath::to_integral(umath::CoordinateSpace::Local), Object = umath::to_integral(umath::CoordinateSpace::Object) };
		static ComponentEventId EVENT_APPLY_CONSTRAINT;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ConstraintManagerComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;

		static void ApplyConstraints(const NetworkState &nw);

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		friend ConstraintComponent;
		std::vector<ConstraintInfo> &GetConstraints();
		const std::vector<ConstraintInfo> &GetConstraints() const { return const_cast<ConstraintManagerComponent *>(this)->GetConstraints(); }
		void Clear();
		void AddConstraint(ConstraintComponent &constraint);
		void RemoveConstraint(ConstraintComponent &constraint);
		void ReleaseConstraint(ConstraintComponent &constraint);
		void ChangeOrder(ConstraintComponent &constraint, int32_t newOrderIndex);
		std::vector<ConstraintInfo>::iterator FindConstraint(ConstraintComponent &constraint);
		std::vector<ConstraintComponent *> m_ownConstraints;
	};
};

#endif
