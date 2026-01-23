// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.observer;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SObserverComponent final : public BaseObserverComponent, public SBaseNetComponent {
	  public:
		SObserverComponent(ecs::BaseEntity &ent);
		virtual ~SObserverComponent() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual void DoSetObserverMode(ObserverMode mode) override;
		virtual void SetObserverTarget(BaseObservableComponent *ent) override;

		virtual bool ShouldTransmitNetData() const override { return true; };
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
