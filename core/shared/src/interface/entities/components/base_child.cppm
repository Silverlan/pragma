// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_child;

export import :entities.components.base;
export import :entities.universal_reference;

export namespace pragma {
	namespace baseChildComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_PARENT_CHANGED)
	}
	class DLLNETWORK BaseChildComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		void ClearParent();
		void SetParent(const EntityURef &parent);
		const EntityURef &GetParent() const;

		ecs::BaseEntity *GetParentEntity();
		const ecs::BaseEntity *GetParentEntity() const { return const_cast<BaseChildComponent *>(this)->GetParentEntity(); }
		bool HasParent() const;
	  protected:
		BaseChildComponent(ecs::BaseEntity &ent);
		virtual void OnEntitySpawn() override;
		virtual void OnParentChanged(ecs::BaseEntity *parent) {};

		EntityURef m_parent;
		bool m_parentValid = false;
		NetEventId m_netEvSetParent = INVALID_NET_EVENT;
	};
};
