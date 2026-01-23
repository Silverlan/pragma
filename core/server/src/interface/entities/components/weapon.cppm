// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.weapon;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SWeaponComponent final : public BaseWeaponComponent, public SBaseNetComponent {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		static uint32_t GetWeaponCount();
		static const std::vector<SWeaponComponent *> &GetAll();

		SWeaponComponent(ecs::BaseEntity &ent);
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
		virtual void OnPickedUp(ecs::BaseEntity *ent);
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
		virtual void OnUse(ecs::BaseEntity *pl);
		virtual void OnPhysicsInitialized() override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua::State *l) override;
	  private:
		static std::vector<SWeaponComponent *> s_weapons;
	};
};
