/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_attachable_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/lua/luafunction_call.h>
#include <wgui/wgui.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <prosper_window.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

static CVar cvSpeed = GetClientConVar("cl_mouse_sensitivity");
static CVar cvAcceleration = GetClientConVar("cl_mouse_acceleration");
static CVar cvYaw = GetClientConVar("cl_mouse_yaw");
static CVar cvPitch = GetClientConVar("cl_mouse_pitch");

void CGame::CalcLocalPlayerOrientation()
{
	auto *pl = GetLocalPlayer();
	if(pl == NULL)
		return;
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	//Vector3 pos = pl->GetPosition();
	auto orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetRotation();
	uquat::inverse(m_curFrameRotationModifier);
	orientation = m_curFrameRotationModifier *orientation;
	m_curFrameRotationModifier = uquat::identity();
	//Con::cerr<<"Actual ("<<pl->GetIndex()<<"): "<<&(*pl->GetViewOrientation())<<Con::endl;

	auto w = c_engine->GetRenderContext().GetWindowWidth();
	auto h = c_engine->GetRenderContext().GetWindowHeight();
	float wDelta,hDelta;
	auto *window = WGUI::GetInstance().FindFocusedWindow();
	if(window && window->IsValid() && WGUI::GetInstance().GetFocusedElement(window) == nullptr)
	{
		auto pos = (*window)->GetCursorPos();
		(*window)->SetCursorPos(Vector2i(umath::round(w /2.f),umath::round(h /2.f)));
		wDelta = pos.x -w /2.f;
		hDelta = pos.y -h /2.f;
		if((h %2) != 0)
			hDelta -= 0.5f;
	}
	else
	{
		wDelta = 0.f;
		hDelta = 0.f;
	}
	float speed = -cvSpeed->GetFloat();
	float speedYaw = cvYaw->GetFloat();
	float speedPitch = cvPitch->GetFloat();

	//Vector3 up = pl->GetUpDirection();
	auto rot = uquat::identity();
	auto *pObserverTarget = pl->GetObserverTarget();
	if(pObserverTarget != nullptr)
	{
		auto *pObserverCharComponent = static_cast<pragma::CCharacterComponent*>(pObserverTarget->GetEntity().GetCharacterComponent().get());
		if(pObserverCharComponent != nullptr)
			rot = pObserverCharComponent->GetOrientationAxesRotation();
		else
			pObserverTarget = nullptr;
	}
	if(pObserverTarget == nullptr && charComponent.valid())
		rot = charComponent->GetOrientationAxesRotation();

	const Vector3 forward(0,0,1);
	const Vector3 right(-1,0,0);
	const Vector3 up(0,1,0);
	
	auto acc = cvAcceleration->GetFloat() +1.f;
	if(hDelta != 0.f)
		hDelta = umath::pow(CFloat(abs(hDelta)),acc) *((hDelta > 0.f) ? 1 : -1);
	if(wDelta != 0.f)
		wDelta = umath::pow(CFloat(abs(wDelta)),acc) *((wDelta > 0.f) ? 1 : -1);

	const auto tDelta = 0.016f; // 60 FPS as reference
	auto rotPitch = uquat::create(-right,speed *speedPitch *CFloat(tDelta) *hDelta);
	auto rotYaw = uquat::create(up,speed *speedYaw *CFloat(tDelta) *wDelta);
	orientation = rot *orientation;
	auto oldAng = EulerAngles(orientation);
	orientation = rotYaw *orientation *rotPitch;
	// TODO: Does this work with custom player up directions? Is there a direct way, without converting to euler angles?

	EulerAngles ang(orientation);
	if(ang.p < -90.f || ang.p > 90.f)
	{
		//ang.r = 0.f;
		if(ang.p < -90.f)
		{
			if(oldAng.p >= -90.f)
				ang.p = -90.f;
			else
				ang.p = umath::max(ang.p,oldAng.p);
			//ang.p = -90.f;
		}
		else if(ang.p > 90.f)
		{
			if(oldAng.p <= 90.f)
				ang.p = 90.f;
			else
				ang.p = umath::min(ang.p,oldAng.p);
			//ang.p = 90.f;
		}
		orientation = uquat::create(ang);
	}
	//
	orientation = uquat::get_inverse(rot) *orientation;

	//if(true)
	//{
		if(charComponent.valid())
			charComponent->SetViewOrientation(orientation);
		else
			pTrComponent->SetRotation(orientation);
		return;
	//}
}

void CGame::CalcView()
{
	auto *pl = GetLocalPlayer();
	if(pl == NULL)
		return;
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	Vector3 pos;
	Quat orientation = uquat::identity();
	pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
	Vector3 offset = pl->GetViewOffset();
	Vector3 upDir = charComponent.valid() ? charComponent->GetUpDirection() : uvec::UP;
	offset = Vector3(offset.x,0,offset.z) +upDir *offset.y;
	pos += offset;

	orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetRotation();

	auto rotModifier = uquat::identity();
	CallCallbacks<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>,std::reference_wrapper<Quat>>("CalcView",std::ref(pos),std::ref(orientation),std::ref(rotModifier));
	CallLuaCallbacks<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>,std::reference_wrapper<Quat>>("CalcView",std::ref(pos),std::ref(orientation),std::ref(rotModifier));

	orientation = rotModifier *orientation;
	m_curFrameRotationModifier = rotModifier;

	if(charComponent.valid())
		charComponent->SetViewOrientation(orientation);
	else
		pTrComponent->SetRotation(orientation);
	pl->SetViewPos(pos);

	// Update entities attached to player camera
	// TODO: Do this properly (e.g. by callback)
	/*EntityIterator entIt {*this};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CAttachableComponent>>();
	for(auto *ent : entIt)
	{
		auto pAttComponent = ent->GetComponent<pragma::CAttachableComponent>();
		if((pAttComponent->GetAttachmentFlags() &(FAttachmentMode::PlayerView | FAttachmentMode::PlayerViewYaw)) != FAttachmentMode::None)
			pAttComponent->UpdateAttachmentOffset();
	}*/

	CallCallbacks<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>("CalcViewOffset",std::ref(pos),std::ref(orientation));
	CallLuaCallbacks<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>("CalcViewOffset",std::ref(pos),std::ref(orientation));

	auto *cam = c_game->GetRenderCamera();
	if(cam == nullptr)
		return;
	cam->GetEntity().SetPosition(pos);
	cam->GetEntity().SetRotation(orientation);
	cam->UpdateViewMatrix();
}
