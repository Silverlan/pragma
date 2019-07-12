#include "stdafx_shared.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/game/game_limits.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/lua/luacallback.h"
#include "pragma/lua/luafunction_call.h"
#include "pragma/model/model.h"
#include <sharedutils/datastream.h>
#include "pragma/physics/raytraces.h"
#include "pragma/entities/baseentity_trace.hpp"

using namespace pragma;

BaseTransformComponent::BaseTransformComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_pos(util::Vector3Property::Create()),
	m_orientation(util::QuatProperty::Create(uquat::identity())),
	m_scale(util::Vector3Property::Create({1.f,1.f,1.f}))
{
	auto hEnt = ent.GetHandle();
	m_pos->AddCallback([this,hEnt](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
		if(hEnt.IsValid() == false)
			return;
		auto &ent = GetEntity();
		ent.SetStateFlag(BaseEntity::StateFlags::PositionChanged);
		m_tLastMoved = ent.GetNetworkState()->GetGameState()->CurTime();
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto *pPhys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
		if(pPhys != NULL && umath::is_flag_set(pPhysComponent->GetStateFlags(),BasePhysicsComponent::StateFlags::ApplyingPhysicsPosition) == false)
			pPhys->SetPosition(pos);
		ent.MarkForSnapshot();
	});
	m_orientation->AddCallback([this,hEnt](std::reference_wrapper<const Quat> oldRot,std::reference_wrapper<const Quat> rot) {
		if(hEnt.IsValid() == false)
			return;
		auto &ent = GetEntity();
		ent.SetStateFlag(BaseEntity::StateFlags::RotationChanged);
		m_tLastMoved = ent.GetNetworkState()->GetGameState()->CurTime();
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
		if(phys != NULL && umath::is_flag_set(pPhysComponent->GetStateFlags(),BasePhysicsComponent::StateFlags::ApplyingPhysicsRotation) == false)
			phys->SetOrientation(rot);
		ent.MarkForSnapshot();
	});
}
void BaseTransformComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetScale = SetupNetEvent("set_scale");

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &mdl = static_cast<CEOnModelChanged&>(evData.get()).model;
		if(mdl.get() == nullptr)
		{
			SetEyeOffset({});
			return util::EventReply::Unhandled;
		}
		SetEyeOffset(mdl.get()->GetEyeOffset());
		return util::EventReply::Unhandled;
	});
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		/*if(ustring::compare(kvData.key,"origin",false))
			SetPosition(uvec::create(kvData.value));
		else */if(ustring::compare(kvData.key,"angles",false))
		{
			EulerAngles ang;
			ustring::string_to_array<float,double>(kvData.value,&ang.p,atof,3);
			SetAngles(ang);
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
const util::PVector3Property &BaseTransformComponent::GetPosProperty() const {return m_pos;}
const util::PQuatProperty &BaseTransformComponent::GetOrientationProperty() const {return m_orientation;}
const util::PVector3Property &BaseTransformComponent::GetScaleProperty() const {return m_scale;}

Vector3 BaseTransformComponent::GetEyePosition() const
{
	auto eyeOffset = GetEyeOffset();
	Vector3 forward,right,up;
	GetOrientation(&forward,&right,&up);
	auto eyePos = GetPosition();
	eyePos = eyePos +forward *eyeOffset.x +up *eyeOffset.y +right *eyeOffset.z;
	return eyePos;
}
Vector3 BaseTransformComponent::GetEyeOffset() const {return m_eyeOffset *GetScale();}
void BaseTransformComponent::SetEyeOffset(const Vector3 &offset) {m_eyeOffset = offset;}

float BaseTransformComponent::GetMaxAxisScale() const
{
	auto &scale = m_scale->GetValue();
	auto r = scale.x;
	if(umath::abs(scale.y) > umath::abs(r))
		r = scale.y;
	if(umath::abs(scale.z) > umath::abs(r))
		r = scale.z;
	return r;
}
float BaseTransformComponent::GetAbsMaxAxisScale() const {return umath::abs(GetMaxAxisScale());}
const Vector3 &BaseTransformComponent::GetScale() const {return *m_scale;}
void BaseTransformComponent::SetScale(float scale) {SetScale({scale,scale,scale});}
void BaseTransformComponent::SetScale(const Vector3 &scale) {*m_scale = scale;}

float BaseTransformComponent::GetDistance(const Vector3 &p) const {return uvec::distance(GetPosition(),p);}
float BaseTransformComponent::GetDistance(const BaseEntity &ent) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return uvec::distance(GetPosition(),pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{});
}

void BaseTransformComponent::SetPosition(const Vector3 &pos,Bool bForceUpdate)
{
	if(
		bForceUpdate == false &&
		fabsf(pos.x -(*m_pos)->x) <= ENT_EPSILON &&
		fabsf(pos.y -(*m_pos)->y) <= ENT_EPSILON &&
		fabsf(pos.z -(*m_pos)->z) <= ENT_EPSILON
	)
		return;
	auto &ent = GetEntity();
	if(std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z))
	{
		Con::cwar<<"WARNING: NaN position ("<<pos.x<<","<<pos.y<<","<<pos.z<<") for entity ";
		ent.print(Con::cout);
		Con::cwar<<"! Ignoring..."<<Con::endl;
		return;
	}
	if(std::isinf(pos.x) || std::isinf(pos.y) || std::isinf(pos.z))
	{
		Con::cwar<<"WARNING: inf position ("<<pos.x<<","<<pos.y<<","<<pos.z<<") for entity ";
		ent.print(Con::cout);
		Con::cwar<<"! Ignoring..."<<Con::endl;
		return;
	}
	*m_pos = pos;
}

void BaseTransformComponent::SetPosition(const Vector3 &pos) {SetPosition(pos,false);}

const Vector3 &BaseTransformComponent::GetPosition() const {return *m_pos;}

void BaseTransformComponent::SetOrientation(const Quat &q)
{
	if(
		fabsf(q.w -(*m_orientation)->w) <= ENT_EPSILON &&
		fabsf(q.x -(*m_orientation)->x) <= ENT_EPSILON &&
		fabsf(q.y -(*m_orientation)->y) <= ENT_EPSILON &&
		fabsf(q.z -(*m_orientation)->z) <= ENT_EPSILON
	)
		return;
	auto &ent = GetEntity();
	if(std::isnan(q.w) || std::isnan(q.x) || std::isnan(q.y) || std::isnan(q.z))
	{
		Con::cwar<<"WARNING: NaN rotation ("<<q.w<<","<<q.x<<","<<q.y<<","<<q.z<<") for entity ";
		ent.print(Con::cout);
		Con::cwar<<"! Ignoring..."<<Con::endl;
		return;
	}
	*m_orientation = q;
}

void BaseTransformComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	ds->Write<Vector3>(*m_pos);
	ds->Write<Quat>(*m_orientation);
	ds->Write<Vector3>(*m_scale);
	ds->Write<Vector3>(m_eyeOffset);
}

void BaseTransformComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	auto pos = ds->Read<Vector3>();
	SetPosition(pos);

	auto rot = ds->Read<Quat>();
	SetOrientation(rot);

	auto scale = ds->Read<Vector3>();
	SetScale(scale);

	auto eyeOffset = ds->Read<Vector3>();
	SetEyeOffset(eyeOffset);
}

const Quat &BaseTransformComponent::GetOrientation() const {return *m_orientation;}
void BaseTransformComponent::SetAngles(const EulerAngles &ang) {SetOrientation(uquat::create(ang));}

void BaseTransformComponent::SetPitch(float pitch)
{
	EulerAngles angles = GetAngles();
	angles.p = pitch;
	SetAngles(angles);
}
void BaseTransformComponent::SetYaw(float yaw)
{
	EulerAngles angles = GetAngles();
	angles.y = yaw;
	SetAngles(angles);
}
void BaseTransformComponent::SetRoll(float roll)
{
	EulerAngles angles = GetAngles();
	angles.r = roll;
	SetAngles(angles);
}

void BaseTransformComponent::LocalToWorld(Vector3 *origin) const
{
	uvec::local_to_world(GetPosition(),GetOrientation(),*origin);
}
void BaseTransformComponent::LocalToWorld(Quat *rot) const
{
	uvec::local_to_world(GetOrientation(),*rot);
}
void BaseTransformComponent::LocalToWorld(Vector3 *origin,Quat *rot) const
{
	LocalToWorld(origin);
	LocalToWorld(rot);
}

void BaseTransformComponent::WorldToLocal(Vector3 *origin) const
{
	uvec::world_to_local(GetPosition(),GetOrientation(),*origin);
}
void BaseTransformComponent::WorldToLocal(Quat *rot) const
{
	uvec::world_to_local(GetOrientation(),*rot);
}
void BaseTransformComponent::WorldToLocal(Vector3 *origin,Quat *rot) const
{
	WorldToLocal(origin);
	WorldToLocal(rot);
}

EulerAngles BaseTransformComponent::GetAngles() const {return EulerAngles(*m_orientation);}
float BaseTransformComponent::GetPitch() const {return GetAngles().p;}
float BaseTransformComponent::GetYaw() const {return GetAngles().y;}
float BaseTransformComponent::GetRoll() const {return GetAngles().r;}
Vector3 BaseTransformComponent::GetForward() const {return uquat::forward(*m_orientation);}
Vector3 BaseTransformComponent::GetUp() const {return uquat::up(*m_orientation);}
Vector3 BaseTransformComponent::GetRight() const {return uquat::right(*m_orientation);}
void BaseTransformComponent::GetOrientation(Vector3 *forward,Vector3 *right,Vector3 *up) const {uquat::get_orientation(*m_orientation,forward,right,up);}
Mat4 BaseTransformComponent::GetRotationMatrix() const {return umat::create(*m_orientation);}

void BaseTransformComponent::UpdateLastMovedTime()
{
	m_tLastMoved = GetEntity().GetNetworkState()->GetGameState()->CurTime();
}

Vector3 BaseTransformComponent::GetDirection(const BaseEntity &ent,bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetDirection(pTrComponent.valid() ? ent.GetCenter() : Vector3{},bIgnoreYAxis);
}
EulerAngles BaseTransformComponent::GetAngles(const BaseEntity &ent,bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetAngles(pTrComponent.valid() ? ent.GetCenter() : Vector3{},bIgnoreYAxis);
}
float BaseTransformComponent::GetDotProduct(const BaseEntity &ent,bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetDotProduct(pTrComponent.valid() ? ent.GetCenter() : Vector3{},bIgnoreYAxis);
}
Vector3 BaseTransformComponent::GetDirection(const Vector3 &pos,bool bIgnoreYAxis) const
{
	auto &origin = GetPosition();
	auto dir = pos -origin;
	if(bIgnoreYAxis == true)
		dir.y = 0.f;
	uvec::normalize(&dir);
	return dir;
}
EulerAngles BaseTransformComponent::GetAngles(const Vector3 &pos,bool bIgnoreYAxis) const {return uvec::to_angle(GetDirection(pos,bIgnoreYAxis),GetUp());}
float BaseTransformComponent::GetDotProduct(const Vector3 &pos,bool bIgnoreYAxis) const {return uvec::dot(GetForward(),GetDirection(pos,bIgnoreYAxis));}

Vector3 BaseTransformComponent::GetOrigin() const
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	return pPhysComponent.valid() ? pPhysComponent->GetOrigin() : GetPosition();
}

double BaseTransformComponent::GetLastMoveTime() const {return m_tLastMoved;}

TraceData util::get_entity_trace_data(BaseTransformComponent &component)
{
	auto &origin = component.GetPosition();
	auto dir = component.GetForward();
	TraceData trData;
	trData.SetSource(origin);
	trData.SetTarget(origin +dir *static_cast<float>(GameLimits::MaxRayCastRange));
	trData.SetFilter(component.GetEntity());
	trData.SetFlags(RayCastFlags::InvertFilter);
	auto pPhysComponent = component.GetEntity().GetPhysicsComponent();
	if(pPhysComponent.valid())
	{
		trData.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
		trData.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	}
	return trData;
}
void BaseTransformComponent::SetRawPosition(const Vector3 &pos)
{
	// TODO: Obsolete function; Remove this and use SetPosition instead!
	*m_pos = pos;
}
void BaseTransformComponent::SetRawOrientation(const Quat &rot)
{
	// TODO: Obsolete function; Remove this and use SetOrientation instead!
	*m_orientation = rot;
}
void BaseTransformComponent::SetRawScale(const Vector3 &scale)
{
	// TODO: Obsolete function; Remove this and use SetScale instead!
	*m_scale = scale;
}
