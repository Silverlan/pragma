// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_static_bvh_user;

export import :entities.components.base;

export namespace pragma {
	class BaseBvhComponent;
	class BaseStaticBvhCacheComponent;
	class PanimaComponent;
	namespace baseStaticBvhUserComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_ACTIVATION_STATE_CHANGED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_STATIC_BVH_COMPONENT_CHANGED)
	}
	class DLLNETWORK BaseStaticBvhUserComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		virtual ~BaseStaticBvhUserComponent() override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		void SetStaticBvhCacheComponent(BaseStaticBvhCacheComponent *component);
		void InitializeDynamicBvhSubstitute(size_t staticBvhCacheVersion);
		size_t GetStaticBvhCacheVersion() const { return m_staticBvhCacheVersion; }
		bool HasDynamicBvhSubstitute() const;
		void DestroyDynamicBvhSubstitute();
		bool IsActive() const;
	  protected:
		BaseStaticBvhUserComponent(ecs::BaseEntity &ent);
		friend BaseStaticBvhCacheComponent;
		void UpdateBvhStatus();
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		CallbackHandle m_cbOnPoseChanged;
		ComponentHandle<BaseStaticBvhCacheComponent> m_staticBvhComponent {};
		BaseBvhComponent *m_bvhComponent = nullptr;
		PanimaComponent *m_panimaComponent = nullptr;
		bool m_isActive = false;
		size_t m_staticBvhCacheVersion = 0;
	};
};
