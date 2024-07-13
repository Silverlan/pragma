/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/entities/components/c_observer_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_game_component.hpp"
#include "pragma/entities/components/c_input_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/c_model.h"
#include "pragma/console/c_cvar.h"
#include <pragma/physics/raytraces.h>
#include <pragma/entities/components/orientation_component.hpp>

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

ComponentEventId CObserverComponent::EVENT_CALC_VIEW = pragma::INVALID_COMPONENT_ID;
ComponentEventId CObserverComponent::EVENT_CALC_VIEW_OFFSET = pragma::INVALID_COMPONENT_ID;
void CObserverComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_CALC_VIEW = registerEvent("CALC_VIEW", ComponentEventInfo::Type::Explicit);
	EVENT_CALC_VIEW = registerEvent("CALC_VIEW_OFFSET", ComponentEventInfo::Type::Explicit);
}
CObserverComponent::CObserverComponent(BaseEntity &ent) : BaseObserverComponent(ent) {}

CObserverComponent::~CObserverComponent() {}

void CObserverComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

bool CObserverComponent::IsInFirstPersonMode() const { return (GetObserverMode() == ObserverMode::FirstPerson) ? true : false; }

Bool CObserverComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetObserverMode)
		SetObserverMode(packet->Read<ObserverMode>());
	else if(eventId == m_netEvSetObserverTarget) {
		auto *ent = nwm::read_entity(packet);
		auto pObsComponent = ent->GetComponent<pragma::CObservableComponent>();
		SetObserverTarget(pObsComponent.get());
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

void CObserverComponent::ReceiveData(NetPacket &packet)
{
	auto *ent = nwm::read_entity(packet);
	if(!ent)
		SetObserverTarget(nullptr);
	else {
		auto observableC = ent->GetComponent<CObservableComponent>();
		if(observableC.valid())
			SetObserverTarget(observableC.get());
	}

	auto mode = packet->Read<ObserverMode>();
	SetObserverMode(mode);
}

void CObserverComponent::SetObserverTarget(BaseObservableComponent *ent) { BaseObserverComponent::SetObserverTarget(ent); }

void CObserverComponent::DoSetObserverMode(ObserverMode mode) { BaseObserverComponent::DoSetObserverMode(mode); }
void CObserverComponent::SetObserverMode(ObserverMode mode)
{
	BaseObserverComponent::SetObserverMode(mode);
	auto *renderC = static_cast<CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(renderC)
		renderC->UpdateShouldDrawState();
}

static CVar cvSpeed = GetClientConVar("cl_mouse_sensitivity");
static CVar cvYaw = GetClientConVar("cl_mouse_yaw");
static CVar cvPitch = GetClientConVar("cl_mouse_pitch");
void CObserverComponent::UpdateCharacterViewOrientationFromMouseMovement()
{
	auto *target = GetObserverTarget();
	if(IsActive() == false || !target)
		return;
	auto &ent = target->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	//Vector3 pos = pl->GetPosition();
	auto orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetRotation();
	uquat::inverse(m_curFrameRotationModifier);
	orientation = m_curFrameRotationModifier * orientation;
	m_curFrameRotationModifier = uquat::identity();
	//Con::cerr<<"Actual ("<<pl->GetIndex()<<"): "<<&(*pl->GetViewOrientation())<<Con::endl;

	float speed = -cvSpeed->GetFloat();
	float speedYaw = cvYaw->GetFloat();
	float speedPitch = cvPitch->GetFloat();

	//Vector3 up = pl->GetUpDirection();
	auto rot = uquat::identity();
	auto observerC = GetEntity().GetComponent<CObserverComponent>();
	auto *pObserverTarget = observerC.valid() ? observerC->GetObserverTarget() : nullptr;
	if(pObserverTarget != nullptr) {
		auto *pObserverCharComponent = static_cast<pragma::CCharacterComponent *>(pObserverTarget->GetEntity().GetCharacterComponent().get());
		if(pObserverCharComponent != nullptr)
			rot = pObserverCharComponent->GetOrientationAxesRotation();
		else
			pObserverTarget = nullptr;
	}
	if(pObserverTarget == nullptr && charComponent.valid())
		rot = charComponent->GetOrientationAxesRotation();

	const Vector3 forward(0, 0, 1);
	const Vector3 right(-1, 0, 0);
	const Vector3 up(0, 1, 0);

	auto *gameC = static_cast<CGame &>(GetGame()).GetGameComponent();
	float xDelta = 0.f;
	float yDelta = 0.f;
	if(gameC) {
		auto inputC = gameC->GetEntity().GetComponent<CInputComponent>();
		if(inputC.valid()) {
			xDelta = inputC->GetMouseDeltaX();
			yDelta = inputC->GetMouseDeltaY();
		}
	}

	const auto tDelta = 0.016f; // 60 FPS as reference
	auto rotPitch = uquat::create(-right, speed * speedPitch * CFloat(tDelta) * yDelta);
	auto rotYaw = uquat::create(up, speed * speedYaw * CFloat(tDelta) * xDelta);
	orientation = rot * orientation;
	auto oldAng = EulerAngles(orientation);
	orientation = rotYaw * orientation * rotPitch;
	// TODO: Does this work with custom player up directions?

	EulerAngles ang(orientation);
	if(ang.p < -90.f || ang.p > 90.f) {
		if(ang.p < -90.f) {
			if(oldAng.p >= -90.f)
				ang.p = -90.f;
			else
				ang.p = umath::max(ang.p, oldAng.p);
		}
		else if(ang.p > 90.f) {
			if(oldAng.p <= 90.f)
				ang.p = 90.f;
			else
				ang.p = umath::min(ang.p, oldAng.p);
		}
		orientation = uquat::create(ang);
	}
	//
	orientation = uquat::get_inverse(rot) * orientation;

	if(charComponent.valid())
		charComponent->SetViewOrientation(orientation);
	else
		pTrComponent->SetRotation(orientation);
}

void CObserverComponent::ApplyCameraObservationMode(Vector3 &pos, Quat &rot, Quat &rotModifier)
{
	auto *obsC = GetObserverTarget();
	if(obsC == nullptr)
		return;
	pragma::ObserverCameraData *obsCamData = nullptr;
	switch(GetObserverMode()) {
	case ObserverMode::FirstPerson:
		obsCamData = &obsC->GetCameraData(BaseObservableComponent::CameraType::FirstPerson);
		break;
	case ObserverMode::ThirdPerson:
		obsCamData = &obsC->GetCameraData(BaseObservableComponent::CameraType::ThirdPerson);
		break;
	}
	//auto physType = ent->GetPhysicsType();
	auto pTrComponentObs = obsC->GetEntity().GetTransformComponent();
	auto camRot = (obsCamData == nullptr || obsCamData->angleLimits.has_value() == false) ? rot : pTrComponentObs ? pTrComponentObs->GetRotation() : uquat::identity();

	auto pose = obsC->GetEntity().GetPose();
	if(obsCamData && obsCamData->localOrigin.has_value())
		pose.TranslateLocal(*obsCamData->localOrigin);
	else {
		// Note: GetEyePosition is not always reliable, since it's derived from the Source Engine $eyeposition parameter,
		// which is only valid for NPC models.
		// Instead, we'll use the "eyes" attachment as reference if available.
		auto &mdl = obsC->GetEntity().GetModel();
		auto eyeAtt = mdl ? mdl->LookupAttachment("eyes") : -1; // TODO: Cache the lookup
		auto eyePose = (eyeAtt != -1) ? obsC->GetEntity().GetAttachmentPose(eyeAtt) : std::optional<umath::Transform> {};
		if(eyePose.has_value()) {
			eyePose->GetOrigin().x = 0;
			eyePose->GetOrigin().z = 0;
			pose.TranslateLocal(eyePose->GetOrigin());
		}
		else if(pTrComponentObs)
			pose.SetOrigin(pTrComponentObs->GetEyePosition());
	}

	auto &entObs = obsC->GetEntity();
	auto charComponent = entObs.GetCharacterComponent();
	auto pTrComponent = entObs.GetTransformComponent();
	if(obsCamData == nullptr || obsCamData->angleLimits.has_value() == false) {
		if(charComponent.valid())
			rot = charComponent->GetViewOrientation();
		else
			rot = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	}
	else
		rot = pTrComponentObs ? pTrComponentObs->GetRotation() : uquat::identity();

	auto rotateWithObservee = (obsCamData && obsCamData->rotateWithObservee) ? true : false;
	auto rotPos = camRot;
	if(rotateWithObservee) {
		// Apply entity rotation for the current frame
		auto entRot = entObs.GetRotation() * rotModifier;
		rotModifier = entRot;
		rotPos = entRot * rotPos;
	}

	auto camLookAtPos = pose.GetOrigin();
	auto camPos = camLookAtPos;
	if(obsCamData)
		camPos += uquat::forward(rotPos) * (*obsCamData->offset)->z + uquat::up(rotPos) * (*obsCamData->offset)->y - uquat::right(rotPos) * (*obsCamData->offset)->x;

	if(obsCamData != nullptr && uvec::length_sqr(*obsCamData->offset) > 0.f) {
		TraceData data {};
		data.SetSource(camLookAtPos);
		data.SetTarget(camPos);
		data.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
		data.SetFilter(obsC->GetEntity());
		auto r = GetGame().RayCast(data);
		pos = (r.hitType == RayCastHitType::Block) ? r.position : camPos;
	}
	else
		pos = camPos;
}

void CObserverComponent::UpdateCameraPose()
{
	auto *target = GetObserverTarget();
	if(IsActive() == false || !target)
		return;
	auto &ent = target->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	Vector3 pos;
	Quat orientation = uquat::identity();
	pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	Vector3 offset = static_cast<CObservableComponent *>(target)->GetViewOffset();
	auto upDir = uvec::UP;
	if(charComponent.valid()) {
		auto *orientC = charComponent->GetOrientationComponent();
		if(orientC)
			upDir = orientC->GetUpDirection();
	}
	offset = Vector3(offset.x, 0, offset.z) + upDir * offset.y;
	pos += offset;

	orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetRotation();

	auto rotModifier = uquat::identity();
	InvokeEventCallbacks(EVENT_CALC_VIEW, CECalcView {pos, orientation, rotModifier});

	ApplyCameraObservationMode(pos, orientation, rotModifier);

	orientation = rotModifier * orientation;
	m_curFrameRotationModifier = rotModifier;

	if(charComponent.valid())
		charComponent->SetViewOrientation(orientation);
	else
		pTrComponent->SetRotation(orientation);

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

	InvokeEventCallbacks(EVENT_CALC_VIEW_OFFSET, CECalcViewOffset {pos, orientation});

	auto &entCam = GetEntity();
	entCam.SetPosition(pos);
	entCam.SetRotation(orientation);
	// cam->UpdateViewMatrix();
}

/////////////////

CECalcView::CECalcView(Vector3 &pos, Quat &rot, Quat &rotModifier) : pos {pos}, rot {rot}, rotModifier {rotModifier} {}
void CECalcView::PushArguments(lua_State *l)
{
	Lua::Push<Vector3 *>(l, &pos);
	Lua::Push<Quat *>(l, &rot);
	Lua::Push<Quat *>(l, &rotModifier);
}

/////////////////

CECalcViewOffset::CECalcViewOffset(Vector3 &pos, Quat &rot) : pos {pos}, rot {rot} {}
void CECalcViewOffset::PushArguments(lua_State *l)
{
	Lua::Push<Vector3 *>(l, &pos);
	Lua::Push<Quat *>(l, &rot);
}
