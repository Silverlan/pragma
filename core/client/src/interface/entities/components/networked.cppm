// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:entities.components.networked;

import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CNetworkedComponent final : public BaseNetworkedComponent, public CBaseNetComponent {
	  public:
		CNetworkedComponent(BaseEntity &ent) : BaseNetworkedComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
