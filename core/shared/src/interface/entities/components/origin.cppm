// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property_vector.h>

export module pragma.shared:entities.components.origin;

export namespace pragma {
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
