/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_WEAPON_COMPONENT_HPP__
#define __S_WEAPON_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <cinttypes>
#include <vector>
#include <pragma/entities/components/base_weapon_component.hpp>

namespace nwm {
	class RecipientFilter;
};
namespace pragma {
	class DLLSERVER SWeaponComponent final : public BaseWeaponComponent, public SBaseNetComponent {
	  public:
		static uint32_t GetWeaponCount();
		static const std::vector<SWeaponComponent *> &GetAll();

		SWeaponComponent(BaseEntity &ent);
		virtual ~SWeaponComponent() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void Initialize() override;
		virtual void Deploy() override;
		virtual void Holster() override;
		virtual void PrimaryAttack() override;
		virtual void SecondaryAttack() override;
		virtual void TertiaryAttack() override;
		virtual void Attack4() override;
		virtual void Reload() override;
		virtual void OnTick(double tDelta) override;
		virtual void OnRemove() override;
		void Drop();
		virtual void OnPickedUp(BaseEntity *ent);
		// Ammo
		virtual void SetPrimaryClipSize(UInt16 size) override;
		virtual void SetSecondaryClipSize(UInt16 size) override;
		virtual void SetMaxPrimaryClipSize(UInt16 size) override;
		virtual void SetMaxSecondaryClipSize(UInt16 size) override;
		virtual void SetPrimaryAmmoType(UInt32 type) override;
		virtual void SetSecondaryAmmoType(UInt32 type) override;
		void AddPrimaryClip(UInt16 num);
		void AddSecondaryClip(UInt16 num);
		void RefillPrimaryClip(UInt16 num);
		void RefillSecondaryClip(UInt16 num);
		void RefillPrimaryClip();
		void RefillSecondaryClip();
		void RemovePrimaryClip(UInt16 num = 1);
		void RemoveSecondaryClip(UInt16 num = 1);
		void SetPrimaryAmmoType(const std::string &type);
		void SetSecondaryAmmoType(const std::string &type);
		//

		virtual bool ShouldTransmitNetData() const override { return true; };
	  protected:
		void GetTargetRecipients(networking::ClientRecipientFilter &rf);
		virtual void OnUse(BaseEntity *pl);
		virtual void OnPhysicsInitialized() override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	  private:
		static std::vector<SWeaponComponent *> s_weapons;
	};
};

#endif
