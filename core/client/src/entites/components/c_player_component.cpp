/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/physics/collisiontypes.h"
#include "pragma/math/deltaoffset.h"
#include "pragma/math/deltatransform.h"
#include <mathutil/uquat.h>
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/c_rendermode.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/c_listener.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp.h"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/networking/c_nwm_util.h"
#include <pragma/entities/components/submergible_component.hpp>
#include <pragma/physics/raytraces.h>
#include <pragma/model/model.h>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace pragma;

namespace pragma
{
	using ::operator<<;
};
std::vector<CPlayerComponent*> CPlayerComponent::s_players;
const std::vector<CPlayerComponent*> &CPlayerComponent::GetAll() {return s_players;}
unsigned int CPlayerComponent::GetPlayerCount() {return CUInt32(s_players.size());}

extern DLLCENGINE CEngine *c_engine;
extern ClientState *client;
extern CGame *c_game;

Con::c_cout& CPlayerComponent::print(Con::c_cout &os)
{
	auto &ent = GetEntity();
	auto nameC = ent.GetNameComponent();
	os<<"CPlayer["<<(nameC.valid() ? nameC->GetName() : "")<<"]["<<ent.GetIndex()<<"]"<<"["<<ent.GetClass()<<"]"<<"[";
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		os<<"NULL";
	else
		os<<mdl->GetName();
	os<<"]";
	return os;
}

std::ostream& CPlayerComponent::print(std::ostream &os)
{
	auto &ent = GetEntity();
	auto nameC = ent.GetNameComponent();
	os<<"CPlayer["<<(nameC.valid() ? nameC->GetName() : "")<<"]["<<ent.GetIndex()<<"]"<<"["<<ent.GetClass()<<"]"<<"[";
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		os<<"NULL";
	else
		os<<mdl->GetName();
	os<<"]";
	return os;
}

CPlayerComponent::CPlayerComponent(BaseEntity &ent)
	: BasePlayerComponent(ent),m_viewOffset(0,m_standEyeLevel,0),
	m_crouchViewOffset(nullptr),m_upDirOffset(nullptr)
{
	s_players.push_back(this);

	BindEventUnhandled(SubmergibleComponent::EVENT_ON_WATER_SUBMERGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto pSoundEmitterComponent = ent.GetComponent<CSoundEmitterComponent>();
		if(pSoundEmitterComponent.valid())
		{
			if(m_sndUnderwater == nullptr)
			{
				m_sndUnderwater = client->CreateSound("fx.underwater",ALSoundType::Effect,ALCreateFlags::Mono);
				m_sndUnderwater->SetRelative(true);
			}
			if(m_sndUnderwater != nullptr)
			{
				m_sndUnderwater->SetGain(1.f);
				m_sndUnderwater->FadeIn(0.1f);
			}
		}
	});
	BindEventUnhandled(SubmergibleComponent::EVENT_ON_WATER_EMERGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(m_sndUnderwater != nullptr)
			m_sndUnderwater->FadeOut(0.1f);
	});
}

CPlayerComponent::~CPlayerComponent()
{
	auto it = std::find(s_players.begin(),s_players.end(),this);
	if(it != s_players.end())
		s_players.erase(it);
	if(m_cbCalcOrientationView.IsValid())
		m_cbCalcOrientationView.Remove();
	m_crouchViewOffset = nullptr;
	m_upDirOffset = nullptr;
	if(m_cbObserver.IsValid())
		m_cbObserver.Remove();
	if(m_cbUnderwaterDsp.IsValid() == true)
		m_cbUnderwaterDsp->Remove();
}

luabind::object CPlayerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPlayerHandle>(l);}

void CPlayerComponent::OnDeployWeapon(BaseEntity &ent)
{
	if(IsLocalPlayer() == false)
		return;
	auto *vm = c_game->GetViewModel();
	if(vm == nullptr)
		return;
	auto pRenderComponent = static_cast<CBaseEntity&>(vm->GetEntity()).GetRenderComponent();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderMode(RenderMode::View); // TODO: Set render mode to none when weapon is holstered
}

void CPlayerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePlayerComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CCharacterComponent))
	{
		auto &pCharComponent = static_cast<CCharacterComponent&>(component);
		auto &pUpDirProp = pCharComponent.GetUpDirectionProperty();
		FlagCallbackForRemoval(pUpDirProp->AddCallback([this](std::reference_wrapper<const Vector3> oldVal,std::reference_wrapper<const Vector3> newVal) {
			OnSetUpDirection(newVal);
		}),CallbackType::Component,&pCharComponent);
	}
}

util::EventReply CPlayerComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BasePlayerComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseCharacterComponent::EVENT_ON_DEPLOY_WEAPON)
		OnDeployWeapon(static_cast<const CEOnDeployWeapon&>(evData).weapon);
	else if(eventId == BaseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON)
		OnSetActiveWeapon(static_cast<const CEOnSetActiveWeapon&>(evData).weapon);
	else if(eventId == BaseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED)
		OnSetCharacterOrientation(static_cast<const CEOnSetCharacterOrientation&>(evData).up);
	return util::EventReply::Unhandled;
}

void CPlayerComponent::OnSetUpDirection(const Vector3 &direction)
{
	m_upDirOffset = nullptr;
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	//auto &rot = charComponent->NormalizeViewOrientation();
	//m_upDirOffset = std::make_unique<DeltaTransform>(Vector3(0,0,0),rot,4);
}

void CPlayerComponent::OnSetActiveWeapon(BaseEntity *ent)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	auto *prevWeapon = charComponent.valid() ? charComponent->GetActiveWeapon() : nullptr;
	if(prevWeapon != nullptr && prevWeapon->IsWeapon())
		static_cast<pragma::CWeaponComponent&>(*prevWeapon->GetWeaponComponent()).UpdateOwnerAttachment();
}

void CPlayerComponent::OnWaterSubmerged()
{
	if(IsLocalPlayer() == false || m_cbUnderwaterDsp.IsValid() == true)
		return;
	auto *entDsp = c_game->CreateEntity<CEnvSoundDsp>();
	if(entDsp == nullptr)
		return;
	auto *pDspComponent = static_cast<pragma::BaseEnvSoundDspComponent*>(entDsp->FindComponent("sound_dsp").get());
	entDsp->SetKeyValue("spawnflags",std::to_string(umath::to_integral(pragma::BaseEnvSoundDspComponent::SpawnFlags::All | pragma::BaseEnvSoundDspComponent::SpawnFlags::AffectRelative)));
	if(pDspComponent != nullptr)
		pDspComponent->SetDSPEffect("underwater");
	entDsp->Spawn();
	auto *pToggleComponent = static_cast<pragma::BaseToggleComponent*>(entDsp->FindComponent("toggle").get());
	if(pToggleComponent != nullptr)
		pToggleComponent->TurnOn();
	m_cbUnderwaterDsp = entDsp->GetHandle();
}

void CPlayerComponent::OnWaterEmerged()
{
	if(m_cbUnderwaterDsp.IsValid() == false)
		return;
	m_cbUnderwaterDsp->Remove();
}

void CPlayerComponent::UpdateObserverOffset()
{
	// Obsolete
	// TODO: Remove this function
}

bool CPlayerComponent::IsInFirstPersonMode() const
{
	auto *pTgt = GetObserverTarget();
	if(pTgt == nullptr)
		return false;
	return (GetObserverMode() == OBSERVERMODE::FIRSTPERSON && &pTgt->GetEntity() == &GetEntity()) ? true : false;
}

void CPlayerComponent::SetObserverTarget(BaseObservableComponent *ent)
{
	BasePlayerComponent::SetObserverTarget(ent);
	UpdateObserverOffset();
}

void CPlayerComponent::UpdateObserverCallback()
{
	if(IsLocalPlayer() == false)
		return;
	auto obsMode = GetObserverMode();
	if(obsMode == OBSERVERMODE::NONE)
	{
		if(m_cbObserver.IsValid())
			m_cbObserver.Remove();
		return;
	}
	if(m_cbObserver.IsValid())
		return;
	// TODO
	//m_lastObserveeRotation = GetEntity().GetRotation();
	m_cbObserver = c_game->AddCallback("CalcView",FunctionCallback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>,std::reference_wrapper<Quat>>::Create(
		[this](std::reference_wrapper<Vector3> refPos,std::reference_wrapper<Quat> refRot,
			std::reference_wrapper<Quat> rotMod
			) {
		auto *obsC = GetObserverTarget();
		if(obsC == nullptr)
			return;
		pragma::ObserverCameraData *obsCamData = nullptr;
		switch(GetObserverMode())
		{
		case OBSERVERMODE::FIRSTPERSON:
			obsCamData = &obsC->GetCameraData(BaseObservableComponent::CameraType::FirstPerson);
			break;
		case OBSERVERMODE::THIRDPERSON:
			obsCamData = &obsC->GetCameraData(BaseObservableComponent::CameraType::ThirdPerson);
			break;
		}
		auto &pos = refPos.get();
		auto &rot = refRot.get();
		//auto physType = ent->GetPhysicsType();
		auto pTrComponentObs = obsC->GetEntity().GetTransformComponent();
		auto camRot = (obsCamData == nullptr || obsCamData->angleLimits.has_value() == false) ? rot : pTrComponentObs.valid() ? pTrComponentObs->GetOrientation() : uquat::identity();

		umath::Transform pose;
		obsC->GetEntity().GetPose(pose);
		if(obsCamData && obsCamData->localOrigin.has_value())
			pose.TranslateLocal(*obsCamData->localOrigin);
		else if(pTrComponentObs.valid())
			pose.SetOrigin(pTrComponentObs->GetEyePosition());

		auto &entThis = GetEntity();
		auto charComponent = entThis.GetCharacterComponent();
		auto pTrComponent = entThis.GetTransformComponent();
		if(obsCamData == nullptr || obsCamData->angleLimits.has_value() == false)
		{
			if(charComponent.valid())
				rot = charComponent->GetViewOrientation();
			else
				rot = pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();
		}
		else
			rot = pTrComponentObs.valid() ? pTrComponentObs->GetOrientation() : uquat::identity();

		auto rotateWithObservee = (obsCamData && obsCamData->rotateWithObservee) ? true : false;
		auto rotPos = camRot;
		if(rotateWithObservee)
		{
			// Apply entity rotation for the current frame
			auto &entRot = obsC->GetEntity().GetRotation() *rotMod.get();
			rotMod.get() = entRot;
			rotPos = entRot *rotPos;
		}

		auto camLookAtPos = pose.GetOrigin();
		auto camPos = camLookAtPos;
		if(obsCamData)
			camPos += uquat::forward(rotPos) *(*obsCamData->offset)->z +uquat::up(rotPos) *(*obsCamData->offset)->y -uquat::right(rotPos) *(*obsCamData->offset)->x;

		if(obsCamData != nullptr && uvec::length_sqr(*obsCamData->offset) > 0.f)
		{
			TraceData data {};
			data.SetSource(camLookAtPos);
			data.SetTarget(camPos);
			data.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
			data.SetFilter(obsC->GetEntity());
			auto r = c_game->RayCast(data);
			pos = (r.hitType == RayCastHitType::Block) ? r.position : camPos;
		}
		else
			pos = camPos;
	}));
}

void CPlayerComponent::DoSetObserverMode(OBSERVERMODE mode)
{
	BasePlayerComponent::DoSetObserverMode(mode);
	UpdateObserverOffset();
	UpdateObserverCallback();
}

void CPlayerComponent::ApplyViewRotationOffset(const EulerAngles &ang,float dur)
{
	auto tStart = c_game->CurTime();
	auto cb = FunctionCallback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>::Create(nullptr);
	static_cast<Callback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>*>(cb.get())->SetFunction([cb,tStart,ang,dur](std::reference_wrapper<Vector3>,std::reference_wrapper<Quat> rot) mutable {
		auto &t = c_game->CurTime();
		auto tDelta = umath::min(static_cast<float>(t -tStart),dur);
		auto sc = static_cast<float>(umath::sin(tDelta /(dur /2.f) *M_PI_2));
		EulerAngles rotOffset {
			static_cast<float>(umath::approach_angle(0.f,ang.p,umath::abs(ang.p) *sc)),
			static_cast<float>(umath::approach_angle(0.f,ang.y,umath::abs(ang.y) *sc)),
			static_cast<float>(umath::approach_angle(0.f,ang.r,umath::abs(ang.r) *sc))
		};
		rot.get() = rot.get() *uquat::create(rotOffset);
		if(tDelta >= dur)
		{
			cb.Remove();
			/*auto cb = FunctionCallback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>::Create(nullptr);
			static_cast<Callback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>*>(cb.get())->SetFunction([cb,ang](std::reference_wrapper<Vector3> pos,std::reference_wrapper<Quat> rot) mutable {
				rot.get() = rot.get() *uquat::create(ang);
				cb.Remove();
			});
			c_game->AddCallback("CalcView",cb);*/ // Makes sure the camera rotation STAYS at the designated rotation (Works, but only reasonable for pitch-axis, and at half the duration -> Useless?)
		}
	});
	c_game->AddCallback("CalcViewOffset",cb);
}

bool CPlayerComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetObserverTarget)
	{
		auto *ent = nwm::read_entity(packet);
		auto pObsComponent = ent->GetComponent<pragma::CObservableComponent>();
		SetObserverTarget(pObsComponent.get());
	}
	else if(eventId == m_netEvApplyViewRotationOffset)
	{
		auto ang = nwm::read_angles(packet);
		auto dur = packet->Read<float>();
		ApplyViewRotationOffset(ang,dur);
	}
	else if(eventId == m_netEvPrintMessage)
	{
		auto msg = packet->ReadString();
		auto type = static_cast<MESSAGE>(packet->Read<std::underlying_type_t<MESSAGE>>());
		PrintMessage(msg,type);
	}
	else if(eventId == m_netEvRespawn)
	{
		auto charComponent = GetEntity().GetCharacterComponent();
		if(charComponent.valid())
			charComponent->Respawn();
	}
	else if(eventId == m_netEvSetViewOrientation)
	{
		auto charComponent = GetEntity().GetCharacterComponent();
		if(charComponent.valid())
		{
			auto rot = packet->Read<Quat>();
			charComponent->SetViewOrientation(rot);
		}
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}

void CPlayerComponent::OnUpdateMatrices(Mat4 &transformMatrix)
{
	if(IsLocalPlayer() && IsInFirstPersonMode())
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		auto t = (pTrComponent.valid() ? pTrComponent->GetForward() : uvec::FORWARD) *VIEW_BODY_OFFSET;
		transformMatrix = glm::translate(umat::identity(),t) *transformMatrix; // Translate to align shadow with view body
	}
}

void CPlayerComponent::Initialize()
{
	BasePlayerComponent::Initialize();

	BindEventUnhandled(SubmergibleComponent::EVENT_ON_WATER_SUBMERGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnWaterSubmerged();
	});
	BindEventUnhandled(SubmergibleComponent::EVENT_ON_WATER_EMERGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnWaterEmerged();
	});
	BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw&>(evData.get());
		if(ShouldDraw(shouldDrawData.camOrigin) == false)
		{
			shouldDrawData.shouldDraw = CEShouldDraw::ShouldDraw::No;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEvent(CRenderComponent::EVENT_SHOULD_DRAW_SHADOW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw&>(evData.get());
		if(ShouldDrawShadow(shouldDrawData.camOrigin) == false)
		{
			shouldDrawData.shouldDraw = CEShouldDraw::ShouldDraw::No;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnUpdateMatrices(static_cast<CEOnUpdateRenderMatrices&>(evData.get()).transformation);
	});

	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderMode(RenderMode::World);
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->SetCollisionType(COLLISIONTYPE::AABB);
}

void CPlayerComponent::UpdateViewModelTransform()
{
	auto *vm = c_game->GetViewModel();
	if(vm == nullptr)
		return;
	auto &vmEnt = vm->GetEntity();
	auto &ent = GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentVm = vmEnt.GetTransformComponent();
	if(pTrComponentVm.valid() && (charComponent.valid() || pTrComponent.valid()))
	{
		auto pos = charComponent.valid() ? charComponent->GetEyePosition() : pTrComponent->GetPosition();
		auto &rot = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetOrientation();
		auto offset = vm->GetViewModelOffset();
		uvec::local_to_world(pos,rot,offset);
		pTrComponentVm->SetPosition(offset);
		pTrComponentVm->SetOrientation(rot);
	}
	auto pAttComponent = vmEnt.AddComponent<CAttachableComponent>();
	if(pAttComponent.valid())
	{
		AttachmentInfo attInfo {};
		attInfo.flags |= FAttachmentMode::PlayerView | FAttachmentMode::UpdateEachFrame;
		pAttComponent->AttachToEntity(&ent,attInfo);
	}
}

void CPlayerComponent::UpdateViewFOV()
{
	auto *vm = c_game->GetViewModel();
	if(vm == nullptr)
		return;
	c_game->SetViewModelFOV(vm->GetViewFOV());
}

void CPlayerComponent::SetLocalPlayer(bool b)
{
	BasePlayerComponent::SetLocalPlayer(b);
	if(b == false)
		return;
	auto *vm = c_game->GetViewModel();
	if(vm != nullptr)
	{
		auto &vmEnt = vm->GetEntity();
		UpdateViewModelTransform();
		UpdateViewFOV();
		if(!vmEnt.IsSpawned())
			vmEnt.Spawn();
	}
	auto &ent = GetEntity();
	auto *body = c_game->GetViewBody();
	if(body != nullptr)
	{
		auto &entBody = body->GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto pTrComponentBody = body->GetEntity().GetTransformComponent();
		if(pTrComponent.valid() && pTrComponentBody.valid())
		{
			Vector3 pos = pTrComponent->GetPosition() +pTrComponent->GetForward() *VIEW_BODY_OFFSET;
			auto rot = pTrComponent->GetOrientation();
			pTrComponentBody->SetPosition(pos);
			pTrComponentBody->SetOrientation(Quat(rot));
		}
		auto pAttComponent = entBody.AddComponent<CAttachableComponent>();
		if(pAttComponent.valid())
		{
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::PlayerViewYaw | FAttachmentMode::BoneMerge | FAttachmentMode::UpdateEachFrame;
			pAttComponent->AttachToEntity(&ent,attInfo);
		}
		//body->SetRenderMode(RenderMode::None);
		if(!entBody.IsSpawned())
			entBody.Spawn();
	}
	auto *listener = c_game->GetListener();
	if(listener != nullptr)
	{
		auto &entListener = listener->GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto pTrComponentListener = entListener.GetTransformComponent();
		if(pTrComponent.valid() && pTrComponentListener.valid())
		{
			pTrComponentListener->SetPosition(pTrComponent->GetPosition());
			pTrComponentListener->SetOrientation(pTrComponent->GetOrientation());
		}
		auto pAttComponent = entListener.AddComponent<CAttachableComponent>();
		if(pAttComponent.valid())
		{
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::PlayerView;
			pAttComponent->AttachToEntity(&ent,attInfo);
		}
		if(!entListener.IsSpawned())
			entListener.Spawn();
	}
}

bool CPlayerComponent::ShouldDraw(const Vector3 &camOrigin) const
{
	if(!IsLocalPlayer())
		return true;
#pragma message ("TODO: Find a better way to enable rendering, if being rendered through anything but the main camera (e.g. reflections)!")
	auto &scene = c_game->GetScene();
	if(c_game->GetRenderScene() != scene)
		return true;
	return (GetObserverMode() != OBSERVERMODE::FIRSTPERSON) ? true : false;
}

bool CPlayerComponent::ShouldDrawShadow(const Vector3 &camOrigin) const
{
	auto pRenderComponent = static_cast<const CBaseEntity&>(GetEntity()).GetRenderComponent();
	return pRenderComponent.valid() ? pRenderComponent->GetCastShadows() : false;;
}

Vector3 &CPlayerComponent::GetViewOffset() {return m_viewOffset;}
void CPlayerComponent::SetViewOffset(Vector3 offset) {m_viewOffset = offset;}

void CPlayerComponent::Think(double tDelta)
{
	BasePlayerComponent::Think(tDelta);

	if(m_crouchViewOffset != NULL)
	{
		DeltaOffset &doffset = *m_crouchViewOffset;
		if(doffset.time <= 0)
			m_crouchViewOffset = nullptr;
		else
		{
			doffset.delta = umath::min(doffset.delta +tDelta /0.2,1.0); // 0.2 seconds to reach full speed
			float scale = CFloat((doffset.delta *tDelta) /doffset.time);
			scale = umath::min(scale,1.0f);
			Vector3 mv = doffset.offset *scale;
			doffset.offset -= mv;
			SetViewOffset(GetViewOffset() +mv);
			doffset.time -= tDelta;
			if(doffset.time <= 0)
				m_crouchViewOffset = nullptr;
		}
	}
	if(m_upDirOffset != NULL)
	{
		DeltaTransform &dtrans = *m_upDirOffset;
		if(dtrans.time <= 0)
			m_upDirOffset = nullptr;
		else
		{
			dtrans.delta = umath::min(dtrans.delta +tDelta /0.2,1.0); // 0.2 seconds to reach full speed
			float scale = CFloat((dtrans.delta *tDelta) /dtrans.time);
			scale = umath::min(scale,1.0f);

			Vector3 mv = dtrans.offset *scale;
			dtrans.offset -= mv;
			SetViewOffset(GetViewOffset() +mv);

			dtrans.time -= tDelta;
			if(dtrans.time <= 0)
				m_upDirOffset = nullptr;
		}
	}
}
void CPlayerComponent::OnCrouch()
{
	m_crouchViewOffset = std::make_unique<DeltaOffset>(Vector3(0,m_crouchEyeLevel -GetViewOffset().y,0),0.2f);
}
void CPlayerComponent::OnUnCrouch()
{
	m_crouchViewOffset = std::make_unique<DeltaOffset>(Vector3(0,m_standEyeLevel -GetViewOffset().y,0),0.4f);
}
void CPlayerComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const {outTypeIndex = std::type_index(typeid(BasePlayerComponent));}
void CPlayerComponent::ReceiveData(NetPacket &packet)
{
	m_timeConnected = packet->Read<double>();
	auto hThis = GetHandle();
	nwm::read_unique_entity(packet,[hThis,this](BaseEntity *ent) {
		if(ent == nullptr || hThis.expired())
			return;
		m_entFlashlight = ent->GetHandle();
		auto &entThis = GetEntity();
		auto charComponent = entThis.GetCharacterComponent();
		auto pTrComponentEnt = ent->GetTransformComponent();
		auto pTrComponent = entThis.GetTransformComponent();
		if(pTrComponentEnt.valid() && (charComponent.valid() || pTrComponent.valid()))
		{
			pTrComponentEnt->SetPosition(charComponent.valid() ? (charComponent->GetEyePosition() +charComponent->GetViewRight() *12.f +charComponent->GetViewForward() *5.f) : pTrComponent->GetPosition());
			pTrComponentEnt->SetOrientation(charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetOrientation());
		}
		auto pAttComponent = ent->AddComponent<CAttachableComponent>();
		if(pAttComponent.valid())
		{
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::PlayerView | FAttachmentMode::UpdateEachFrame;
			pAttComponent->AttachToEntity(&entThis,attInfo);
		}
	});
}

void CPlayerComponent::PrintMessage(std::string message,MESSAGE type)
{
	switch(type)
	{
		case MESSAGE::PRINTCONSOLE:
			Con::cout<<message<<Con::endl;
			break;
		case MESSAGE::PRINTCHAT:
		{
			// TODO
			//auto *l = client->GetLuaState();
			//DLLLUA void GetGlobal(lua_State *lua,const std::string &name);
			break;
		}
	}
}

void CPlayerComponent::OnSetCharacterOrientation(const Vector3 &up)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	if(IsLocalPlayer() == false)
		return;
	if(m_cbCalcOrientationView.IsValid())
		m_cbCalcOrientationView.Remove();

	// Update camera rotation
	auto rotCur = charComponent->GetViewOrientation();
	auto &rotRel = charComponent->GetOrientationAxesRotation();

	auto rotDst = rotRel *rotCur;
	//auto ang = EulerAngles{rotDst};
	// to euler angles
	auto m = glm::mat4_cast(rotDst);
	EulerAngles ang;
	glm::extractEulerAngleYXZ(m,ang.y,ang.p,ang.r);
	ang.p = umath::rad_to_deg(ang.p);
	ang.y = umath::rad_to_deg(ang.y);
	ang.r = umath::rad_to_deg(ang.r);
	//

	auto fToQuat = [](const EulerAngles &ang) {
		auto m = umat::identity();
		m = glm::eulerAngleYXZ(umath::deg_to_rad(ang.y),umath::deg_to_rad(ang.p),umath::deg_to_rad(ang.r));
		auto q = glm::quat_cast(m);
		return q;
	};

	ang.r = 0.f;
	Quat rotCurNoRoll;
	if(ang.p < -135.f || ang.p > 135.f)
		rotCurNoRoll = fToQuat(ang) *fToQuat(EulerAngles{0.f,0.f,180.f});
	else
	{
		if(ang.p < -90.f)
			ang.p = -90.f;
		else if(ang.p > 90.f)
			ang.p = 90.f;
		rotCurNoRoll = fToQuat(ang);
	}
	rotDst = uquat::get_inverse(rotRel) *rotCurNoRoll;
	charComponent->SetViewOrientation(rotDst);
	//

	/*m_cbCalcOrientationView = c_game->AddCallback("CalcView",FunctionCallback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>::Create([this,up](std::reference_wrapper<Vector3> refPos,std::reference_wrapper<Quat> refRot) {
		//SetUpDirection(up);
		auto &ent = GetEntity();
		auto charComponent = ent.GetCharacterComponent();
		auto pTrComponent = ent.GetTransformComponent();
		auto rotCur = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();
		//auto &up = GetUpDirection();

		auto rotRel = charComponent.valid() ? charComponent->GetOrientationAxesRotation() : uquat::identity();
		auto newRotCur = rotRel *rotCur;
		auto ang = EulerAngles(newRotCur);
		ang.r = 0.f;
		auto rotDst = uquat::identity(); // rotCurNoRoll
		if(ang.p < -135.f || ang.p > 135.f)
		{
			rotDst = uquat::create(ang) *uquat::create(EulerAngles(0.f,0.f,180.f)); // This will effectively flip pitch around
			ang.p = umath::clamp(ang.p,-90.f,90.f);
		}
		else
		{
			ang.p = umath::clamp(ang.p,-90.f,90.f);
			rotDst = uquat::create(ang);
		}

		//refRot.get() = uquat::get_inverse(rotRel) *uquat::slerp(newRotCur,rotDst,c_engine->GetDeltaFrameTime() *4.f);
		// TODO: Remove callback on complete!
	}));*/
}
