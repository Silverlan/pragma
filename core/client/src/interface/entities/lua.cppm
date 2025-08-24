// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/lua/sh_lua_component.hpp>
#include <pragma/lua/handle_holder.hpp>

export module pragma.client.entities.components:lua;

export namespace pragma {
	class DLLCLIENT CLuaBaseEntityComponent final : public BaseLuaBaseEntityComponent, public CBaseSnapshotComponent {
	  public:
		CLuaBaseEntityComponent(BaseEntity &ent);

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual bool ShouldTransmitSnapshotData() const override;
	  protected:
		virtual void InvokeNetEventHandle(const std::string &methodName, NetPacket &packet, pragma::BasePlayerComponent *pl) override;
	};
};

export namespace pragma::lua {
	using CLuaBaseEntityComponentHolder = HandleHolder<CLuaBaseEntityComponent>;
};
