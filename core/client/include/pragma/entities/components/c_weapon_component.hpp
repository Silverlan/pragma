#ifndef __C_WEAPON_COMPONENT_HPP__
#define __C_WEAPON_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_weapon_component.hpp>
#include <pragma/model/animation/play_animation_flags.hpp>

namespace nwm
{
	class RecipientFilter;
};
namespace pragma
{
	class CViewModelComponent;
	class DLLCLIENT CWeaponComponent final
		: public BaseWeaponComponent,
		public CBaseNetComponent
	{
	public:
		static unsigned int GetWeaponCount();
		static const std::vector<CWeaponComponent*> &GetAll();

		virtual ~CWeaponComponent() override;
		
		CWeaponComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool HandleViewModelAnimationEvent(CViewModelComponent *vm,const AnimationEvent &ev);
		virtual void Deploy() override;
		virtual void Holster() override;
		virtual bool PlayViewActivity(Activity activity,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
		CViewModelComponent *GetViewModel();
		virtual void PrimaryAttack() override;
		virtual void SecondaryAttack() override;
		virtual void TertiaryAttack() override;
		virtual void Attack4() override;
		virtual void Reload() override;
		bool IsInFirstPersonMode() const;
		void UpdateOwnerAttachment();
		void SetViewModel(const std::string &mdl);
		const std::string &GetViewModelName() const;
		void SetHideWorldModelInFirstPerson(bool b);
		bool GetHideWorldModelInFirstPerson() const;
		void SetViewModelOffset(const Vector3 &offset);
		const Vector3 &GetViewModelOffset() const;
		void SetViewFOV(float fov);
		float GetViewFOV() const;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		// Either the view-model or the character that owns the weapon
		EntityHandle m_hTarget;
		bool m_bHideWorldModelInFirstPerson = false;
		std::string m_viewModel = "weapons/v_soldier.wmd";
		Vector3 m_viewModelOffset;
		float m_viewFov = std::numeric_limits<float>::quiet_NaN();
		CallbackHandle m_cbOnOwnerObserverModeChanged = {};
		virtual Activity TranslateViewActivity(Activity act);
		virtual void OnFireBullets(const BulletInfo &bulletInfo,Vector3 &bulletOrigin,Vector3 &bulletDir,Vector3 *effectsOrigin=nullptr) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		void UpdateViewModel();
		void UpdateWorldModel();
		void ClearOwnerCallbacks();
	private:
		static std::vector<CWeaponComponent*> s_weapons;
	};
};

#endif
