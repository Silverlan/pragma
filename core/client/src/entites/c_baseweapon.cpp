#include "stdafx_client.h"
#include "pragma/entities/c_baseweapon.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include <pragma/model/animation/activities.h>
#include "pragma/entities/c_viewmodel.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/console/c_cvar.h"
#include <pragma/util/bulletinfo.h>
/*
DEFINE_ENTITY_HANDLE(CBaseWeapon,WeaponHandle);

std::vector<CBaseWeapon*> CBaseWeapon::s_weapons;
void CBaseWeapon::GetAll(std::vector<CBaseWeapon*> **weapons) {*weapons = &s_weapons;}
unsigned int CBaseWeapon::GetWeaponCount() {return CUInt32(s_weapons.size());}

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

CBaseWeapon::CBaseWeapon()
	: CBaseEntity(),BaseWeapon()
{
	s_weapons.push_back(this);
}

CBaseWeapon::~CBaseWeapon()
{
	auto it = std::find(s_weapons.begin(),s_weapons.end(),this);
	if(it != s_weapons.end())
		s_weapons.erase(it);
	ClearOwnerCallbacks();
}

void CBaseWeapon::ReceiveData(NetPacket &packet)
{
	CBaseEntity::ReceiveData(packet);
	auto primAmmoType = packet->Read<UInt32>();
	auto secAmmoType = packet->Read<UInt32>();
	auto primClipSize = packet->Read<UInt16>();
	auto secClipSize = packet->Read<UInt16>();
	auto primMaxClipSize = packet->Read<UInt16>();
	auto secMaxClipSize = packet->Read<UInt16>();
	SetPrimaryAmmoType(primAmmoType);
	SetSecondaryAmmoType(secAmmoType);
	SetPrimaryClipSize(primClipSize);
	SetSecondaryClipSize(secClipSize);
	SetMaxPrimaryClipSize(primMaxClipSize);
	SetMaxSecondaryClipSize(secMaxClipSize);
}

bool CBaseWeapon::ShouldDraw(const Vector3 &camOrigin) const
{
	auto renderMode = GetRenderMode();
	if(renderMode != RenderMode::None)
	{
		if(renderMode == RenderMode::View)
		{
			auto *pl = c_game->GetLocalPlayer();
			if(pl->IsInFirstPersonMode() == false)
				return false;
			return CBaseEntity::ShouldDraw(camOrigin);
		}
		auto *owner = GetOwner();
		if(owner != nullptr)
		{
			auto *c = dynamic_cast<BaseCharacter*>(owner);
			if(c != nullptr)
				return (c->GetActiveWeapon() == this) ? true : false;
		}
	}
	return CBaseEntity::ShouldDraw(camOrigin);
}

bool CBaseWeapon::ShouldDrawShadow(const Vector3 &camOrigin) const
{
	return (m_bDeployed == false && CBaseEntity::ShouldDrawShadow(camOrigin)) ? true : false;
}

bool CBaseWeapon::HandleViewModelAnimationEvent(CViewModel*,const AnimationEvent&) {return false;}

void CBaseWeapon::InitializePhysObj()
{
	CBaseEntity::InitializePhysObj();
	BaseWeapon::InitializePhysObj(m_physObject);
}

bool CBaseWeapon::IsWeapon() const {return true;}
const BaseWeapon *CBaseWeapon::GetWeapon() const {return this;}
BaseWeapon *CBaseWeapon::GetWeapon() {return this;}

void CBaseWeapon::Initialize()
{
	CBaseEntity::Initialize();
	BaseWeapon::Initialize();
}

void CBaseWeapon::Think(double tDelta)
{
	CBaseEntity::Think(tDelta);
	BaseWeapon::Think(tDelta);
}

void CBaseWeapon::OnRemove()
{
	CBaseEntity::OnRemove();
	BaseWeapon::OnRemove();
}

void CBaseWeapon::UpdateViewModel()
{
	if(IsDeployed() == false)
		return;
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	if(m_viewModel.empty() == true)
	{
		vm->SetRenderMode(RenderMode::None);
		vm->SetViewModelOffset({});
		return;
	}
	vm->GetModelComponent().SetModel(m_viewModel);
	vm->SetRenderMode(RenderMode::View);
	vm->SetViewModelOffset(GetViewModelOffset());
	vm->SetViewFOV(GetViewFOV());
}

static auto cvViewFov = GetClientConVar("cl_fov_viewmodel");
void CBaseWeapon::SetViewModelOffset(const Vector3 &offset)
{
	m_viewModelOffset = offset;
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	vm->SetViewModelOffset(offset);
}
const Vector3 &CBaseWeapon::GetViewModelOffset() const {return m_viewModelOffset;}
void CBaseWeapon::SetViewFOV(float fov)
{
	m_viewFov = fov;
	auto *vm = GetViewModel();
	if(vm == nullptr)
		return;
	vm->SetViewFOV(fov);
}
float CBaseWeapon::GetViewFOV() const
{
	if(std::isnan(m_viewFov) == true)
		return cvViewFov->GetFloat();
	return m_viewFov;
}

bool CBaseWeapon::IsInFirstPersonMode() const
{
	if(IsDeployed() == false)
		return false;
	auto *owner = GetOwner();
	if(owner == nullptr || owner->IsPlayer() == false)
		return false;
	auto *pl = static_cast<CPlayer*>(owner);
	return pl->IsLocalPlayer() && pl->IsInFirstPersonMode();
}

void CBaseWeapon::UpdateWorldModel()
{
	SetRenderMode(IsInFirstPersonMode() ? ((m_bHideWorldModelInFirstPerson == true) ? RenderMode::None : RenderMode::View) : RenderMode::World);
}

void CBaseWeapon::OnPhysicsInitialized()
{
	CBaseEntity::OnPhysicsInitialized();
	BaseWeapon::OnPhysicsInitialized();
}

void CBaseWeapon::SetViewModel(const std::string &mdl)
{
	m_viewModel = mdl;
	UpdateViewModel();
}
const std::string &CBaseWeapon::GetViewModelName() const {return m_viewModel;}

void CBaseWeapon::OnFireBullets(const BulletInfo &bulletInfo,Vector3 &bulletOrigin,Vector3 &bulletDir,Vector3 *effectsOrigin)
{
	CBaseEntity::OnFireBullets(bulletInfo,bulletOrigin,bulletDir,effectsOrigin);
	BaseWeapon::OnFireBullets(bulletInfo,bulletOrigin,bulletDir,effectsOrigin);
	auto *owner = GetOwner();
	if(owner != nullptr && owner->IsPlayer())
	{
		auto *pl = static_cast<CPlayer*>(owner);
		if(pl->IsLocalPlayer())
		{
			bulletDir = pl->GetViewForward();
			bulletOrigin = pl->GetViewPos();
		}
	}
	if(effectsOrigin == nullptr)
		return;
	if(std::isnan(bulletInfo.effectOrigin.x) == false)
	{
		*effectsOrigin = bulletInfo.effectOrigin;
		return;
	}
	if(GetAttachment(m_attMuzzle,effectsOrigin,static_cast<Quat*>(nullptr)) == true)
		LocalToWorld(effectsOrigin);
}

void CBaseWeapon::ClearOwnerCallbacks()
{
	if(m_cbOnOwnerObserverModeChanged.IsValid() == false)
		return;
	m_cbOnOwnerObserverModeChanged.Remove();
}

void CBaseWeapon::OnOwnerChanged(BaseEntity *oldOwner,BaseEntity *newOwner)
{
	ClearOwnerCallbacks();
	if(newOwner != nullptr && newOwner->IsPlayer() == true && static_cast<CPlayer*>(newOwner)->IsLocalPlayer() == true)
	{
		m_cbOnOwnerObserverModeChanged = newOwner->AddCallback("OnObserverModeChanged",FunctionCallback<void,OBSERVERMODE,OBSERVERMODE>::Create([this](OBSERVERMODE oldMode,OBSERVERMODE newMode) {
			UpdateOwnerAttachment();
		}));
	}
}

void CBaseWeapon::UpdateOwnerAttachment()
{
	m_hTarget = {};
	UpdateWorldModel();
	auto *owner = GetOwner();
	if(owner == nullptr)
	{
		SetParent(nullptr);
		return;
	}
	CGame *game = client->GetGameState();
	BaseEntity *parent = owner;
	if(owner->IsPlayer())
	{
		auto *pl = static_cast<CPlayer*>(owner);
		if(pl->IsLocalPlayer() && pl->GetActiveWeapon() == this && IsInFirstPersonMode() == true)
		{
			parent = game->GetViewModel();
			if(parent == nullptr)
				return;
		}
	}
	m_hTarget = parent->GetHandle();
	SetPosition(parent->GetPosition());
	SetOrientation(parent->GetOrientation());
	auto attId = parent->LookupAttachment("weapon");
	if(attId != -1)
		SetParentAttachment(parent,"weapon",FPARENT_SNAP_TO_ORIGIN | FPARENT_UPDATE_EACH_FRAME);
	else
		SetParent(parent,FPARENT_SNAP_TO_ORIGIN | FPARENT_UPDATE_EACH_FRAME);
	//SetParent(parent,FPARENT_BONEMERGE | FPARENT_UPDATE_EACH_FRAME);
	//SetAnimated(true);
}

void CBaseWeapon::SetHideWorldModelInFirstPerson(bool b)
{
	m_bHideWorldModelInFirstPerson = b;
	UpdateWorldModel();
}
bool CBaseWeapon::GetHideWorldModelInFirstPerson() const {return m_bHideWorldModelInFirstPerson;}

void CBaseWeapon::SetOwner(BaseEntity *owner)
{
	BaseWeapon::SetOwner(owner);
	UpdateOwnerAttachment();
}

void CBaseWeapon::Deploy()
{
	BaseWeapon::Deploy();
	UpdateOwnerAttachment();
	UpdateViewModel();
	BaseEntity *vm = GetViewModel();
	if(vm == nullptr)
		return;
	CGame *game = client->GetGameState();
	if(PlayViewActivity(Activity::VmDeploy) == false)
		PlayViewActivity(Activity::VmIdle);
}

void CBaseWeapon::Holster()
{
	BaseWeapon::Holster();
	BaseEntity *vm = GetViewModel();
	if(vm == NULL)
		return;
	CGame *game = client->GetGameState();
	PlayViewActivity(Activity::VmHolster);
}

Activity CBaseWeapon::TranslateViewActivity(Activity act) {return act;}

CViewModel *CBaseWeapon::GetViewModel()
{
	BaseEntity *parent = m_hTarget.get();
	if(parent == NULL)
		return NULL;
	CGame *game = client->GetGameState();
	CViewModel *vm = game->GetViewModel();
	if(vm != parent)
		return NULL;
	return vm;
}

bool CBaseWeapon::PlayViewActivity(Activity activity,pragma::FPlayAnim flags)
{
	BaseEntity *vm = GetViewModel();
	if(vm == nullptr)
		return false;
	activity = TranslateViewActivity(activity);
	vm->GetModelComponent().PlayActivity(activity,flags);
	return true;
}

void CBaseWeapon::PrimaryAttack()
{
	if(!CanPrimaryAttack())
		return;
	BaseWeapon::PrimaryAttack();
}
void CBaseWeapon::SecondaryAttack()
{
	if(!CanSecondaryAttack())
		return;
	BaseWeapon::PrimaryAttack();
}
void CBaseWeapon::TertiaryAttack()
{
	BaseWeapon::TertiaryAttack();
}
void CBaseWeapon::Attack4()
{
	BaseWeapon::Attack4();
}
void CBaseWeapon::Reload()
{
	BaseWeapon::Reload();
}
*/
