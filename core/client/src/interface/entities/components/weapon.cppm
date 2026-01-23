// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.weapon;

export import :entities.components.entity;
export import :entities.components.view_model;

export namespace pragma {
	namespace cWeaponComponent {
		using namespace baseWeaponComponent;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ATTACH_TO_OWNER;
	}
	class DLLCLIENT CWeaponComponent final : public BaseWeaponComponent, public CBaseNetComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		static unsigned int GetWeaponCount();
		static const std::vector<CWeaponComponent *> &GetAll();

		enum class StateFlags : uint8_t { None = 0u, HideWorldModelInFirstPerson = 1u, UpdatingDeployState = HideWorldModelInFirstPerson << 1u };

		virtual ~CWeaponComponent() override;

		CWeaponComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool HandleViewModelAnimationEvent(CViewModelComponent *vm, const AnimationEvent &ev);
		virtual void Deploy() override;
		virtual void Holster() override;
		virtual bool PlayViewActivity(Activity activity, FPlayAnim flags = FPlayAnim::Default);
		CViewModelComponent *GetViewModel();
		virtual void PrimaryAttack() override;
		virtual void SecondaryAttack() override;
		virtual void TertiaryAttack() override;
		virtual void Attack4() override;
		virtual void Reload() override;
		bool IsInFirstPersonMode() const;
		void UpdateOwnerAttachment();
		void SetViewModel(const std::string &mdl);
		const std::optional<std::string> &GetViewModelName() const;
		void SetHideWorldModelInFirstPerson(bool b);
		bool GetHideWorldModelInFirstPerson() const;
		void SetViewModelOffset(const Vector3 &offset);
		const Vector3 &GetViewModelOffset() const;
		void SetViewFOV(math::Degree fov);
		math::Degree GetViewFOV() const;
		virtual bool ShouldTransmitNetData() const override { return true; }

		void SetViewModelComponent(ComponentId component) { m_viewModelComponent = component; }
		ComponentId GetViewModelComponent() const { return m_viewModelComponent; }
		void UpdateDeployState();
	  protected:
		void UpdateObserver(BaseObserverComponent *observer);

		// Either the view-model or the character that owns the weapon
		EntityHandle m_hTarget;
		StateFlags m_stateFlags = StateFlags::None;
		std::optional<std::string> m_viewModel {};
		Vector3 m_viewModelOffset;
		ComponentId m_viewModelComponent = INVALID_COMPONENT_ID;

		std::optional<math::Degree> m_viewFov {};
		CallbackHandle m_cbOnOwnerObserverModeChanged = {};
		CallbackHandle m_cbOnObserverChanged {};
		virtual Activity TranslateViewActivity(Activity act);
		virtual void OnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin = nullptr) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		void UpdateViewModel();
		void UpdateWorldModel();
		void ClearOwnerCallbacks();
	  private:
		static std::vector<CWeaponComponent *> s_weapons;
	};
	struct DLLCLIENT CEAttachToOwner : public ComponentEvent {
		CEAttachToOwner(ecs::BaseEntity &owner, CViewModelComponent *optViewmodel);
		virtual void PushArguments(lua::State *l) override;
		ecs::BaseEntity &owner;
		CViewModelComponent *viewModel = nullptr;
	};

	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::CWeaponComponent::StateFlags)
}
