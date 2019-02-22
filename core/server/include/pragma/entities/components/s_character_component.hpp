#ifndef __S_CHARACTER_COMPONENT_HPP__
#define __S_CHARACTER_COMPONENT_HPP__

#include <pragma/entities/components/base_character_component.hpp>
#include "pragma/serverdefinitions.h"

class Faction;
namespace pragma
{
	class DLLSERVER SCharacterComponent final
		: public BaseCharacterComponent
	{
	public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		SCharacterComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void SetActiveWeapon(BaseEntity *ent) override;
		void DropActiveWeapon();
		void DropWeapon(std::string className);
		void DropWeapon(BaseEntity *ent);
		void SelectNextWeapon();
		void SelectPreviousWeapon();
		virtual void Think(double tDelta) override;
		virtual void SetAmmoCount(UInt32 ammoType,UInt16 count) override;
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
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		Faction *m_faction;
		bool m_bNoTarget;
		bool m_bGodMode;
	};
};

#endif
