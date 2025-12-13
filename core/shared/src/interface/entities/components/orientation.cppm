// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.orientation;

export import :entities.components.base;
export import pragma.util;

export namespace pragma {
	class DLLNETWORK OrientationComponent final : public BaseEntityComponent {
	  public:
		OrientationComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		const util::PVector3Property &GetUpDirectionProperty() const;
		const Vector3 &GetUpDirection() const;
		virtual void SetUpDirection(const Vector3 &direction);
		// Returns the forward,right and up vectors, depending on the player's up direction
		void GetOrientationAxes(Vector3 *outForward, Vector3 *outRight, Vector3 *outUp) const;
		// Returns the rotation between the world's up vector and the player's up direction
		const Quat &GetOrientationAxesRotation() const;
	  protected:
		virtual void InitializeLuaObject(lua::State *lua) override;
		util::PVector3Property m_upDirection = nullptr;
		Vector3 m_axForward = {0.f, 0.f, 1.f};
		Vector3 m_axRight = {-1.f, 0.f, 0.f};
		Quat m_axRot = uquat::identity();
	};
};
