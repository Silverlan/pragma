// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:entities.components.surface;

import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CSurfaceComponent final : public BaseSurfaceComponent, public CBaseNetComponent {
	  public:
		CSurfaceComponent(BaseEntity &ent) : BaseSurfaceComponent(ent) {}

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
