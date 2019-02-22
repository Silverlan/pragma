#include "stdafx_client.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/c_baseweapon.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/model/model.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(viewmodel,CViewModel);

extern DLLCLIENT CGame *c_game;

void CViewModelComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw&>(evData.get());
		auto *pl = c_game->GetLocalPlayer();
		if(pl == nullptr || pl->GetObserverMode() != OBSERVERMODE::FIRSTPERSON)
		{
			shouldDrawData.shouldDraw = CEShouldDraw::ShouldDraw::No;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEvent(CAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &ent = GetEntity();
		auto pAttachableComponent = ent.GetComponent<CAttachableComponent>();
		auto *parent = pAttachableComponent.valid() ? pAttachableComponent->GetParent() : nullptr;
		if(parent != nullptr && parent->GetEntity().IsCharacter())
		{
			auto charComponent = parent->GetEntity().GetCharacterComponent();
			auto *wep = charComponent->GetActiveWeapon();
			if(wep != nullptr && wep->IsWeapon())
			{
				if(static_cast<pragma::CWeaponComponent&>(*wep->GetWeaponComponent()).HandleViewModelAnimationEvent(this,static_cast<CEHandleAnimationEvent&>(evData.get()).animationEvent))
					return util::EventReply::Handled;
			}
		}
		return util::EventReply::Handled; // Always overwrite
	});
	BindEventUnhandled(CAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(hMdl != nullptr)
		{
			auto anim = hMdl->GetAnimation(static_cast<CEOnAnimationComplete&>(evData.get()).animation);
			if(anim != nullptr && anim->HasFlag(FAnim::Loop) == true)
				return;
		}
		auto animComponent = ent.GetAnimatedComponent();
		if(animComponent.valid())
			animComponent->PlayActivity(Activity::VmIdle);
	});

	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	ent.AddComponent<pragma::CTransformComponent>();
	ent.AddComponent<pragma::LogicComponent>(); // Logic component is needed for animations
	auto pRenderComponent = ent.AddComponent<pragma::CRenderComponent>();
	if(pRenderComponent.valid())
	{
		pRenderComponent->SetRenderMode(RenderMode::None);
		pRenderComponent->SetCastShadows(false);
	}
	auto pMdlComponent = ent.AddComponent<CModelComponent>();
	if(pMdlComponent.valid())
		pMdlComponent->SetModel("weapons/v_soldier.wmd");
	ent.AddComponent<CAnimatedComponent>();
}

static auto cvViewFov = GetClientConVar("cl_fov_viewmodel");
void CViewModelComponent::SetViewFOV(float fov)
{
	m_viewFov = fov;
	auto &ent = GetEntity();
	auto pAttComponent = ent.GetComponent<CAttachableComponent>();
	auto *parent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
	if(parent == nullptr || parent->GetEntity().IsPlayer() == false)
		return;
	static_cast<pragma::CPlayerComponent*>(parent->GetEntity().GetPlayerComponent().get())->UpdateViewFOV();
}
float CViewModelComponent::GetViewFOV() const
{
	if(std::isnan(m_viewFov) == true)
		return cvViewFov->GetFloat();
	return m_viewFov;
}

void CViewModelComponent::SetViewModelOffset(const Vector3 &offset)
{
	m_viewModelOffset = offset;
	auto &ent = GetEntity();
	auto pAttComponent = ent.GetComponent<CAttachableComponent>();
	auto *parent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
	if(parent == nullptr || parent->GetEntity().IsPlayer() == false)
		return;
	static_cast<pragma::CPlayerComponent*>(parent->GetEntity().GetPlayerComponent().get())->UpdateViewModelTransform();
}
const Vector3 &CViewModelComponent::GetViewModelOffset() const {return m_viewModelOffset;}
luabind::object CViewModelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CViewModelComponentHandleWrapper>(l);}

/////////////////

void CViewModel::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CViewModelComponent>();
}
