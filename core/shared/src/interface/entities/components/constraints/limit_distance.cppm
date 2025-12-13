// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.limit_distance;

export import :entities.components.constraints.base;

export namespace pragma {
	class DLLNETWORK ConstraintLimitDistanceComponent final : public BaseEntityComponent {
	  public:
		enum class ClampRegion : uint32_t { Inside = 0, Outside, OnSurface };
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintLimitDistanceComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetClampRegion(ClampRegion clampRegion);
		ClampRegion GetClampRegion() const;

		void SetDistance(float dist);
		float GetDistance() const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		ComponentHandle<ConstraintComponent> m_constraintC;
		ClampRegion m_clampRegion = ClampRegion::Inside;
		float m_distSqr = 0.f;
		float m_dist = 0.f;
	};
};
