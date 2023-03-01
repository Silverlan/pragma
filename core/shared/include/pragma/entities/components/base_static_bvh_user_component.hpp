/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_STATIC_BVH_USER_COMPONENT_HPP__
#define __BASE_STATIC_BVH_USER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <unordered_set>

namespace pragma {
	class BaseStaticBvhCacheComponent;
	class DLLNETWORK BaseStaticBvhUserComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_ACTIVATION_STATE_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		virtual ~BaseStaticBvhUserComponent() override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		void SetStaticBvhCacheComponent(BaseStaticBvhCacheComponent *component);
		bool IsActive() const;
	  protected:
		BaseStaticBvhUserComponent(BaseEntity &ent);
		friend BaseStaticBvhCacheComponent;
		void UpdateBvhStatus();
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		CallbackHandle m_cbOnPoseChanged;
		pragma::ComponentHandle<BaseStaticBvhCacheComponent> m_staticBvhComponent {};
		bool m_isActive = false;
	};
};

#endif
