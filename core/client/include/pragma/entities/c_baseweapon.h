#ifndef __C_BASEWEAPON_H__
#define __C_BASEWEAPON_H__
/*
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "baseweapon.h"
#include <pragma/model/animation/play_animation_flags.hpp>

class DLLCLIENT CViewModel;
class DLLCLIENT CBaseWeapon
	: public CBaseEntity,
	public BaseWeapon
{
private:
	static std::vector<CBaseWeapon*> s_weapons;
protected:
	// Either the view-model or the character that owns the weapon
	EntityHandle m_hTarget;
	bool m_bHideWorldModelInFirstPerson = false;
	std::string m_viewModel = "weapons/v_soldier.wmd";
	Vector3 m_viewModelOffset;
	float m_viewFov = std::numeric_limits<float>::quiet_NaN();
	CallbackHandle m_cbOnOwnerObserverModeChanged = {};
	virtual void InitializePhysObj() override;
	virtual Activity TranslateViewActivity(Activity act);
	virtual void OnFireBullets(const BulletInfo &bulletInfo,Vector3 &bulletOrigin,Vector3 &bulletDir,Vector3 *effectsOrigin=nullptr) override;
	virtual void OnPhysicsInitialized() override;
	virtual void OnOwnerChanged(BaseEntity *oldOwner,BaseEntity *newOwner) override;
	void UpdateViewModel();
	void UpdateWorldModel();
	void ClearOwnerCallbacks();
public:
	CBaseWeapon();
	virtual ~CBaseWeapon() override;
	DECLARE_ENTITY_HANDLE;
	virtual void ReceiveData(NetPacket &packet) override;
	virtual bool HandleViewModelAnimationEvent(CViewModel *vm,const AnimationEvent &ev);
	static unsigned int GetWeaponCount();
	static void GetAll(std::vector<CBaseWeapon*> **weapons);
	virtual bool IsWeapon() const override final;
	virtual const BaseWeapon *GetWeapon() const override final;
	virtual BaseWeapon *GetWeapon() override final;
	virtual void Initialize() override;
	virtual void SetOwner(BaseEntity *owner) override;
	virtual void Think(double tDelta) override;
	virtual void OnRemove();
	virtual void Deploy() override;
	virtual void Holster() override;
	virtual bool PlayViewActivity(Activity activity,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
	CViewModel *GetViewModel();
	virtual void PrimaryAttack() override;
	virtual void SecondaryAttack() override;
	virtual void TertiaryAttack() override;
	virtual void Attack4() override;
	virtual void Reload() override;
	virtual bool ShouldDraw(const Vector3 &camOrigin) const override;
	virtual bool ShouldDrawShadow(const Vector3 &camOrigin) const override;
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
};
*/
#endif