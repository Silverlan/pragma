#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_attachable_component.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/lua/luafunction_call.h>
#include <wgui/wgui.h>
#include <pragma/entities/components/base_transform_component.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

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
	if(charComponent.expired() && pTrComponent.expired())
		return;
	//Vector3 pos = pl->GetPosition();
	auto orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetOrientation();
	uquat::inverse(m_curFrameRotationModifier);
	orientation = m_curFrameRotationModifier *orientation;
	m_curFrameRotationModifier = uquat::identity();
	//Con::cerr<<"Actual ("<<pl->GetIndex()<<"): "<<&(*pl->GetViewOrientation())<<Con::endl;

	auto w = c_engine->GetWindowWidth();
	auto h = c_engine->GetWindowHeight();
	float wDelta,hDelta;
	auto &window = c_engine->GetWindow();
	if(window.IsFocused() && WGUI::GetInstance().GetFocusedElement() == nullptr)
	{
		auto pos = window.GetCursorPos();
		window.SetCursorPos(Vector2i(w /2,h /2));
		wDelta = pos.x -w /2.f;
		hDelta = pos.y -h /2.f;
	}
	else
	{
		wDelta = 0.f;
		hDelta = 0.f;
	}
	float speed = -cvSpeed->GetFloat();
	float speedYaw = cvYaw->GetFloat();
	float speedPitch = cvPitch->GetFloat();
	/*EulerAngles a = orientation->ToEulerAngles();
	a = a +Angle(
		speed *speedPitch *tDelta *hDelta,
		speed *speedYaw *tDelta *wDelta,
		0
	);
	*orientation = a.ToQuaternion();*/

	//Quat rot(1,0,0,0);
	//rot.RotateX(speed *speedPitch *tDelta *hDelta);
	//rot.RotateY(speed *speedYaw *tDelta *wDelta);
	//orientation->RotateX(speed *speedPitch *tDelta *hDelta);
	//orientation->RotateY(speed *speedYaw *tDelta *wDelta);

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

	//rot = uquat::create(EulerAngles(rot));
	//uquat::normalize(rot);
	const Vector3 forward(0,0,1);
	const Vector3 right(-1,0,0);
	const Vector3 up(0,1,0);

	//Vector3 up = Vector3(0,1,0);//pl->GetUpDirection();
	//Vector3 forward = up;
	//EulerAngles angUp = Vector3::toAngle(&up);
	//Vector3::rotate(&forward,&Angle(0,0,-90));

	/*CALL_ENGINE_LUA_HOOK("CalcTest",11,1,{
		luabind::object(m_lua,orientation).push(m_lua);
		luabind::object(m_lua,Quat(rot)).push(m_lua);
		luabind::object(m_lua,forward).push(m_lua);
		luabind::object(m_lua,right).push(m_lua);
		luabind::object(m_lua,up).push(m_lua);
		Lua::PushNumber(m_lua,speed);
		Lua::PushNumber(m_lua,speedPitch);
		Lua::PushNumber(m_lua,speedYaw);
		Lua::PushNumber(m_lua,tDelta);
		Lua::PushNumber(m_lua,hDzelta);
		Lua::PushNumber(m_lua,wDelta);
	},{
		if(Lua::IsSet(m_lua,-1) && _lua_isQuaternion(m_lua,-1))
		{
			Quat *r = _lua_Quaternion_check(m_lua,-1);
			orientation = *r;
		}
	});*/
	
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
			pTrComponent->SetOrientation(orientation);
		return;
	//}
	//*orientation = *orientation *rot;
	
	//Vector3 forward = up;
	//EulerAngles angUp = Vector3::toAngle(&up);
	//Vector3::rotate(&forward,&Angle(0,0,-90));
	//orientation->Rotate(forward,speed *speedPitch *tDelta *hDelta);
	//orientation->Rotate(up,speed *speedYaw *tDelta *wDelta);
	
	//EulerAngles ang(orientation);
	/*
	CALL_ENGINE_LUA_HOOK("CalcOrientation",3,1,{
		luabind::object(m_lua,up).push(m_lua);
		luabind::object(m_lua,forward).push(m_lua);
		luabind::object(m_lua,ang).push(m_lua);
	},{
		if(Lua::IsSet(m_lua,-1) && _lua_isAngle(m_lua,-1))
		{
			EulerAngles *r = _lua_Angle_check(m_lua,-1);
			ang = *r;
		}
	});
	*/
	/*EulerAngles angForward(forward);
	if(ang.p > (angForward.p +90.f))
		ang.p = angForward.p +90.f;
	else if(ang.p < (angForward.p -90.f))
		ang.p = angForward.p -90.f;
	pl->SetViewOrientation(Quat(ang));*/
}

void CGame::CalcView()
{
	auto *pl = GetLocalPlayer();
	if(pl == NULL)
		return;
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent.expired())
		return;
	Vector3 pos;
	Quat orientation = uquat::identity();
	if(m_camPosOverride != NULL)
		pos = *m_camPosOverride;
	else
	{
		pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
		Vector3 offset = pl->GetViewOffset();
		Vector3 upDir = charComponent.valid() ? charComponent->GetUpDirection() : uvec::UP;
		offset = Vector3(offset.x,0,offset.z) +upDir *offset.y;
		pos += offset;
	}
	if(m_camRotOverride != NULL)
		orientation = Quat(*m_camRotOverride);
	else
		orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetOrientation();

	auto rotModifier = uquat::identity();
	CallCallbacks<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>,std::reference_wrapper<Quat>>("CalcView",std::ref(pos),std::ref(orientation),std::ref(rotModifier));
	CallLuaCallbacks<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>,std::reference_wrapper<Quat>>("CalcView",std::ref(pos),std::ref(orientation),std::ref(rotModifier));

	orientation = rotModifier *orientation;
	m_curFrameRotationModifier = rotModifier;

	if(charComponent.valid())
		charComponent->SetViewOrientation(orientation);
	else
		pTrComponent->SetOrientation(orientation);
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
