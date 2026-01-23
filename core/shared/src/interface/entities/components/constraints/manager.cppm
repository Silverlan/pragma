// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.manager;

export import :entities.components.constraints.base;

export namespace pragma {
	namespace constraintManagerComponent {
		REGISTER_COMPONENT_EVENT(EVENT_APPLY_CONSTRAINT)
	}
	class DLLNETWORK ConstraintManagerComponent final : public BaseEntityComponent {
	  public:
		struct DLLNETWORK ConstraintInfo {
			ConstraintComponent *constraint = nullptr;

			ConstraintComponent *operator*() { return constraint; }
			const ConstraintComponent *operator*() const { return constraint; }
			ConstraintComponent *operator->() { return constraint; }
			const ConstraintComponent *operator->() const { return constraint; }
		};
		enum class CoordinateSpace : uint8_t { World = pragma::math::to_integral(math::CoordinateSpace::World), Local = pragma::math::to_integral(math::CoordinateSpace::Local), Object = pragma::math::to_integral(math::CoordinateSpace::Object) };

		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ConstraintManagerComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;

		static void ApplyConstraints(const NetworkState &nw);

		virtual void InitializeLuaObject(lua::State *lua) override;
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
