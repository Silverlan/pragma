// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_observer;

export import :entities.components.base;
export import pragma.util;

export namespace pragma {
	class BaseObservableComponent;
	namespace baseObserverComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_OBSERVATION_MODE_CHANGED)
	}
	class DLLNETWORK BaseObserverComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		virtual ~BaseObserverComponent() override;

		virtual void SetObserverMode(ObserverMode mode);
		ObserverMode GetObserverMode() const;
		const util::PEnumProperty<ObserverMode> &GetObserverModeProperty() const;

		virtual void SetObserverTarget(BaseObservableComponent *ent);
		BaseObservableComponent *GetObserverTarget() const;
	  protected:
		BaseObserverComponent(ecs::BaseEntity &ent);
		void ClearObserverTarget();
		virtual void OnRemove() override;
		virtual void DoSetObserverMode(ObserverMode mode) {};

		ComponentHandle<BaseObservableComponent> m_observerTarget = {};

		NetEventId m_netEvSetObserverMode = INVALID_NET_EVENT;
		NetEventId m_netEvSetObserverTarget = INVALID_NET_EVENT;
		util::PEnumProperty<ObserverMode> m_obsMode = nullptr;

		virtual void Initialize() override;
	};
};
