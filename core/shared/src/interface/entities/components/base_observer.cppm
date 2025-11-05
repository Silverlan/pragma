// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_observer;

export import :entities.components.base;
export import pragma.util;

export namespace pragma {
	namespace baseObserverComponent {
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_OBSERVATION_MODE_CHANGED;
	}
	class BaseObservableComponent;
	namespace baseObserverComponent {
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_OBSERVATION_MODE_CHANGED;
	}
	class DLLNETWORK BaseObserverComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		virtual ~BaseObserverComponent() override;

		virtual void SetObserverMode(ObserverMode mode);
		ObserverMode GetObserverMode() const;
		const util::PEnumProperty<ObserverMode> &GetObserverModeProperty() const;

		virtual void SetObserverTarget(BaseObservableComponent *ent);
		BaseObservableComponent *GetObserverTarget() const;
	  protected:
		BaseObserverComponent(pragma::ecs::BaseEntity &ent);
		void ClearObserverTarget();
		virtual void OnRemove() override;
		virtual void DoSetObserverMode(ObserverMode mode) {};

		ComponentHandle<BaseObservableComponent> m_observerTarget = {};

		pragma::NetEventId m_netEvSetObserverMode = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetObserverTarget = pragma::INVALID_NET_EVENT;
		util::PEnumProperty<ObserverMode> m_obsMode = nullptr;

		virtual void Initialize() override;
	};
};
