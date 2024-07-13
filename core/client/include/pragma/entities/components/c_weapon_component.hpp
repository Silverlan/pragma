/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_WEAPON_COMPONENT_HPP__
#define __C_WEAPON_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_weapon_component.hpp>
#include <pragma/model/animation/play_animation_flags.hpp>
#include <optional>

namespace nwm {
	class RecipientFilter;
};
struct AnimationEvent;
namespace pragma {
	class CViewModelComponent;
	class BaseObserverComponent;
	class DLLCLIENT CWeaponComponent final : public BaseWeaponComponent, public CBaseNetComponent {
	  public:
		static ComponentEventId EVENT_ATTACH_TO_OWNER;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		static unsigned int GetWeaponCount();
		static const std::vector<CWeaponComponent *> &GetAll();

		enum class StateFlags : uint8_t { None = 0u, HideWorldModelInFirstPerson = 1u, UpdatingDeployState = HideWorldModelInFirstPerson << 1u };

		virtual ~CWeaponComponent() override;

		CWeaponComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool HandleViewModelAnimationEvent(CViewModelComponent *vm, const AnimationEvent &ev);
		virtual void Deploy() override;
		virtual void Holster() override;
		virtual bool PlayViewActivity(Activity activity, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
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
		void SetViewFOV(umath::Degree fov);
		umath::Degree GetViewFOV() const;
		virtual bool ShouldTransmitNetData() const override { return true; }

		void SetViewModelComponent(pragma::ComponentId component) { m_viewModelComponent = component; }
		pragma::ComponentId GetViewModelComponent() const { return m_viewModelComponent; }
		void UpdateDeployState();
	  protected:
		void UpdateObserver(BaseObserverComponent *observer);

		// Either the view-model or the character that owns the weapon
		EntityHandle m_hTarget;
		StateFlags m_stateFlags = StateFlags::None;
		std::optional<std::string> m_viewModel {};
		Vector3 m_viewModelOffset;
		pragma::ComponentId m_viewModelComponent = pragma::INVALID_COMPONENT_ID;

		std::optional<umath::Degree> m_viewFov {};
		CallbackHandle m_cbOnOwnerObserverModeChanged = {};
		CallbackHandle m_cbOnObserverChanged {};
		virtual Activity TranslateViewActivity(Activity act);
		virtual void OnFireBullets(const BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin = nullptr) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		void UpdateViewModel();
		void UpdateWorldModel();
		void ClearOwnerCallbacks();
	  private:
		static std::vector<CWeaponComponent *> s_weapons;
	};
	struct DLLCLIENT CEAttachToOwner : public ComponentEvent {
		CEAttachToOwner(BaseEntity &owner, CViewModelComponent *optViewmodel);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity &owner;
		CViewModelComponent *viewModel = nullptr;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CWeaponComponent::StateFlags)

#endif
