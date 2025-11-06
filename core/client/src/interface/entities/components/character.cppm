// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:entities.components.character;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CCharacterComponent final : public BaseCharacterComponent, public CBaseNetComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);
		CCharacterComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return false; }
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
	  protected:
		void CreateWaterSplash();
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
