// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"





export module pragma.server.entities.components.character;

import pragma.server.ai;

export namespace pragma {
	class DLLSERVER SCharacterComponent final : public BaseCharacterComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts);

		SCharacterComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void SetActiveWeapon(pragma::ecs::BaseEntity *ent) override;
		void DropActiveWeapon();
		void DropWeapon(std::string className);
		void DropWeapon(pragma::ecs::BaseEntity *ent);
		void SelectNextWeapon();
		void SelectPreviousWeapon();
		virtual void OnTick(double tDelta) override;
		virtual void SetAmmoCount(UInt32 ammoType, UInt16 count) override;
		using BaseCharacterComponent::SetAmmoCount;

		Faction *GetFaction();
		virtual void SetFaction(Faction &faction);
		void SetNoTarget(bool b);
		bool GetNoTarget() const;
		void SetGodMode(bool b);
		bool GetGodMode() const;
	  protected:
		void OnFrozen(bool bFrozen);
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
		virtual void InitializeLuaObject(lua_State *l) override;
		Faction *m_faction;
		bool m_bNoTarget;
		bool m_bGodMode;
	};
};
