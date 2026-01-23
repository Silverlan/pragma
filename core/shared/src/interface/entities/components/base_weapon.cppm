// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_weapon;

export import :entities.components.base_ownable;
export import :game.bullet_info;

export namespace pragma {
	struct DLLNETWORK CEOnClipSizeChanged : public ComponentEvent {
		CEOnClipSizeChanged(UInt16 oldClipSize, UInt16 newClipSize);
		virtual void PushArguments(lua::State *l) override;
		UInt16 oldClipSize;
		UInt16 newClipSize;
	};
	namespace baseWeaponComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_DEPLOY)
		REGISTER_COMPONENT_EVENT(EVENT_ON_HOLSTER)
		REGISTER_COMPONENT_EVENT(EVENT_ON_PRIMARY_ATTACK)
		REGISTER_COMPONENT_EVENT(EVENT_ON_SECONDARY_ATTACK)
		REGISTER_COMPONENT_EVENT(EVENT_ON_TERTIARY_ATTACK)
		REGISTER_COMPONENT_EVENT(EVENT_ON_ATTACK4)
		REGISTER_COMPONENT_EVENT(EVENT_ON_END_PRIMARY_ATTACK)
		REGISTER_COMPONENT_EVENT(EVENT_ON_END_SECONDARY_ATTACK)
		REGISTER_COMPONENT_EVENT(EVENT_ON_RELOAD)
		REGISTER_COMPONENT_EVENT(EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED)
	}
	class DLLNETWORK BaseWeaponComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		virtual void Deploy();
		virtual void Holster();
		virtual void PrimaryAttack();
		virtual void SecondaryAttack();
		virtual void TertiaryAttack();
		virtual void Attack4();
		virtual void EndPrimaryAttack();
		virtual void EndSecondaryAttack();
		void EndAttack();
		virtual void SetNextPrimaryAttack(float t);
		virtual void SetNextSecondaryAttack(float t);
		void SetNextAttack(float t);
		virtual void SetAutomaticPrimary(bool b);
		virtual void SetAutomaticSecondary(bool b);
		bool IsAutomaticPrimary() const;
		bool IsAutomaticSecondary() const;
		bool IsDeployed() const;
		virtual void Reload();
		virtual void OnTick(double tDelta) override;
		virtual void OnRemove() override;
		// Ammo
		UInt32 GetPrimaryAmmoType() const;
		const util::PUInt32Property &GetPrimaryAmmoTypeProperty() const;
		UInt32 GetSecondaryAmmoType() const;
		const util::PUInt32Property &GetSecondaryAmmoTypeProperty() const;
		Bool HasPrimaryAmmo() const;
		Bool HasSecondaryAmmo() const;
		Bool IsPrimaryClipEmpty() const;
		Bool IsSecondaryClipEmpty() const;
		Bool HasAmmo() const;
		virtual void SetPrimaryClipSize(UInt16 size);
		virtual void SetSecondaryClipSize(UInt16 size);
		UInt16 GetPrimaryClipSize() const;
		const util::PUInt16Property &GetPrimaryClipSizeProperty() const;
		UInt16 GetSecondaryClipSize() const;
		const util::PUInt16Property &GetSecondaryClipSizeProperty() const;
		UInt16 GetMaxPrimaryClipSize() const;
		const util::PUInt16Property &GetMaxPrimaryClipSizeProperty() const;
		UInt16 GetMaxSecondaryClipSize() const;
		const util::PUInt16Property &GetMaxSecondaryClipSizeProperty() const;
		virtual void SetMaxPrimaryClipSize(UInt16 size);
		virtual void SetMaxSecondaryClipSize(UInt16 size);
		virtual void SetPrimaryAmmoType(UInt32 type);
		virtual void SetSecondaryAmmoType(UInt32 type);

		BaseOwnableComponent *GetOwnerComponent();
		virtual void OnEntitySpawn() override;
	  protected:
		BaseWeaponComponent(ecs::BaseEntity &ent);
		void UpdateTickPolicy();
		bool CanPrimaryAttack() const;
		bool CanSecondaryAttack() const;
		virtual void OnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin = nullptr);
		virtual void OnPhysicsInitialized();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

		ComponentHandle<BaseOwnableComponent> m_whOwnerComponent = {};
		float m_tDeploy = 0.f;
		float m_tNextPrimaryAttack = 0.f;
		float m_tNextSecondaryAttack = 0.f;
		bool m_bAutomaticPrimary = false;
		bool m_bAutomaticSecondary = false;
		bool m_bInAttack1 = false;
		bool m_bInAttack2 = false;
		bool m_bDeployed = false;
		int32_t m_attMuzzle = -1;
		// Ammo
		util::PUInt32Property m_ammoPrimary = nullptr;
		util::PUInt32Property m_ammoSecondary = nullptr;
		util::PUInt16Property m_clipPrimary = nullptr;
		util::PUInt16Property m_clipSecondary = nullptr;
		util::PUInt16Property m_maxPrimaryClipSize = nullptr;
		util::PUInt16Property m_maxSecondaryClipSize = nullptr;
		//
	};
};
