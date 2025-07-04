// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_OBSERVER_COMPONENT_HPP__
#define __S_OBSERVER_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_observer_component.hpp>

namespace pragma {
	namespace networking {
		class IServerClient;
	};
	class DLLSERVER SObserverComponent final : public BaseObserverComponent, public SBaseNetComponent {
	  public:
		SObserverComponent(BaseEntity &ent);
		virtual ~SObserverComponent() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual void DoSetObserverMode(ObserverMode mode) override;
		virtual void SetObserverTarget(BaseObservableComponent *ent) override;

		virtual bool ShouldTransmitNetData() const override { return true; };
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
