// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.character;

import :ai;

export namespace pragma {
	namespace sCharacterComponent {
		using namespace baseActorComponent;
		using namespace baseCharacterComponent;
	}
	class DLLSERVER SCharacterComponent final : public BaseCharacterComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		SCharacterComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void SetActiveWeapon(ecs::BaseEntity *ent) override;
		void DropActiveWeapon();
		void DropWeapon(std::string className);
		void DropWeapon(ecs::BaseEntity *ent);
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
		virtual void InitializeLuaObject(lua::State *l) override;
		Faction *m_faction;
		bool m_bNoTarget;
		bool m_bGodMode;
	};
};
