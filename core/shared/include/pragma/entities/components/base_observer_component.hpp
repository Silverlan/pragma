// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_OBSERVER_COMPONENT_HPP__
#define __BASE_OBSERVER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/observermode.h"

namespace pragma {
	class BaseObservableComponent;
	class DLLNETWORK BaseObserverComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_OBSERVATION_MODE_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		virtual ~BaseObserverComponent() override;

		virtual void SetObserverMode(ObserverMode mode);
		ObserverMode GetObserverMode() const;
		const util::PEnumProperty<ObserverMode> &GetObserverModeProperty() const;

		virtual void SetObserverTarget(BaseObservableComponent *ent);
		BaseObservableComponent *GetObserverTarget() const;
	  protected:
		BaseObserverComponent(BaseEntity &ent);
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

#endif
