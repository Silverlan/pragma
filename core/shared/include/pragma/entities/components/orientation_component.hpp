/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __ORIENTATION_COMPONENT_HPP__
#define __ORIENTATION_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class DLLNETWORK OrientationComponent final : public BaseEntityComponent {
	  public:
		OrientationComponent(BaseEntity &ent);
		virtual void Initialize() override;

		const util::PVector3Property &GetUpDirectionProperty() const;
		const Vector3 &GetUpDirection() const;
		virtual void SetUpDirection(const Vector3 &direction);
		// Returns the forward,right and up vectors, depending on the player's up direction
		void GetOrientationAxes(Vector3 *outForward, Vector3 *outRight, Vector3 *outUp) const;
		// Returns the rotation between the world's up vector and the player's up direction
		const Quat &GetOrientationAxesRotation() const;
	  protected:
		virtual void InitializeLuaObject(lua_State *lua) override;
		util::PVector3Property m_upDirection = nullptr;
		Vector3 m_axForward = {0.f, 0.f, 1.f};
		Vector3 m_axRight = {-1.f, 0.f, 0.f};
		Quat m_axRot = uquat::identity();
	};
};

#endif
