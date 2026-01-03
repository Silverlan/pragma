// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.origin;

export import :entities.components.base;

export namespace pragma {
	namespace originComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_ORIGIN_CHANGED)
	}
	class DLLNETWORK OriginComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		OriginComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetOriginPose(const math::Transform &pose);
		void SetOriginPos(const Vector3 &pos);
		void SetOriginRot(const Quat &rot);

		const math::Transform &GetOriginPose() const;
		const Vector3 &GetOriginPos() const;
		const Quat &GetOriginRot() const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		math::Transform m_origin;
	};
};
