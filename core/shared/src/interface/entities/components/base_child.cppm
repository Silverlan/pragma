// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.base_child;

export import :entities.components.base;
export import :entities.universal_reference;

export namespace pragma {
	class DLLNETWORK BaseChildComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_PARENT_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		void ClearParent();
		void SetParent(const pragma::EntityURef &parent);
		const pragma::EntityURef &GetParent() const;

		pragma::ecs::BaseEntity *GetParentEntity();
		const pragma::ecs::BaseEntity *GetParentEntity() const { return const_cast<BaseChildComponent *>(this)->GetParentEntity(); }
		bool HasParent() const;
	  protected:
		BaseChildComponent(pragma::ecs::BaseEntity &ent);
		virtual void OnEntitySpawn() override;
		virtual void OnParentChanged(pragma::ecs::BaseEntity *parent) {};

		EntityURef m_parent;
		bool m_parentValid = false;
		pragma::NetEventId m_netEvSetParent = pragma::INVALID_NET_EVENT;
	};
};
