// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_character_component.hpp>

export module pragma.client.entities.components:character;

export namespace pragma {
	class DLLCLIENT CCharacterComponent final : public BaseCharacterComponent, public CBaseNetComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		CCharacterComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return false; }
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
	  protected:
		void CreateWaterSplash();
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
