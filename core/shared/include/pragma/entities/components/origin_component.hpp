/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ORIGIN_COMPONENT_HPP__
#define __ORIGIN_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property_vector.h>

namespace pragma {
	class DLLNETWORK OriginComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_ORIGIN_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		OriginComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetOriginPose(const umath::Transform &pose);
		void SetOriginPos(const Vector3 &pos);
		void SetOriginRot(const Quat &rot);

		const umath::Transform &GetOriginPose() const;
		const Vector3 &GetOriginPos() const;
		const Quat &GetOriginRot() const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		umath::Transform m_origin;
	};
};

#endif
