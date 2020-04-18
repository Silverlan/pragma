#include "stdafx_client.h"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/model/c_model.h"

extern DLLCLIENT CGame *c_game;

void pragma::CEyeComponent::UpdateEyeballs()
{
	auto mdlC = GetEntity().GetModelComponent();
	if(mdlC.expired())
		return;
	auto &mdl = mdlC->GetModel();
	if(mdl == nullptr)
		return;
	auto &eyeballs = mdl->GetEyeballs();
	for(auto eyeballIndex=decltype(eyeballs.size()){0u};eyeballIndex<eyeballs.size();++eyeballIndex)
		UpdateEyeball(eyeballs.at(eyeballIndex),eyeballIndex);
	InvokeEventCallbacks(EVENT_ON_EYEBALLS_UPDATED);
}

const pragma::CEyeComponent::EyeballConfig *pragma::CEyeComponent::GetEyeballConfig(uint32_t eyeballIndex) const {return const_cast<CEyeComponent*>(this)->GetEyeballConfig(eyeballIndex);}
pragma::CEyeComponent::EyeballConfig *pragma::CEyeComponent::GetEyeballConfig(uint32_t eyeballIndex)
{
	auto *data = GetEyeballData(eyeballIndex);
	return data ? &data->config : nullptr;
}
void pragma::CEyeComponent::SetEyeballConfig(const EyeballConfig &eyeballConfig) {m_eyeballConfig = eyeballConfig;}
const pragma::CEyeComponent::EyeballData *pragma::CEyeComponent::GetEyeballData(uint32_t eyeballIndex) const {return const_cast<CEyeComponent*>(this)->GetEyeballData(eyeballIndex);}
pragma::CEyeComponent::EyeballData *pragma::CEyeComponent::GetEyeballData(uint32_t eyeballIndex)
{
	return (eyeballIndex < m_eyeballData.size()) ? &m_eyeballData.at(eyeballIndex) : nullptr;
}

Vector3 pragma::CEyeComponent::GetViewTarget() const
{
	if(m_viewTarget.has_value())
		return *m_viewTarget;
	constexpr auto dist = 500.f; // Arbitrary distance; just has to be far enough
	auto &ent = GetEntity();
	auto forward = ent.GetForward();
	auto pos = ent.GetPosition() +forward *dist;
	if(m_eyeAttachmentIndex != std::numeric_limits<uint32_t>::max())
	{
		auto mdlC = ent.GetModelComponent();
		if(mdlC.valid())
		{
			Vector3 attPos {};
			auto attRot = uquat::identity();
			if(mdlC->GetAttachment(m_eyeAttachmentIndex,&attPos,&attRot))
			{
				physics::Transform attPose;
				ent.GetPose(attPose);
				attPose *= physics::Transform{attPos,attRot};
				pos = attPose.GetOrigin() +uquat::forward(attPose.GetRotation()) *dist;
			}
		}
	}
	return ClampViewTarget(pos);
}

void pragma::CEyeComponent::ClearViewTarget() {m_viewTarget = {};}

static void angle_to_vector(const EulerAngles &angles,Vector3 &outForward)
{
	auto y = -umath::deg_to_rad(angles.y); // Needs to be inverted for some reason
	auto sy = umath::sin(y);
	auto cy = umath::cos(y);

	auto sp = umath::sin(umath::deg_to_rad(angles.p));
	auto cp = umath::cos(umath::deg_to_rad(angles.p));

	outForward.x = cp *cy;
	outForward.y = cp *sy;
	outForward.z = -sp;
}
Vector3 pragma::CEyeComponent::ClampViewTarget(const Vector3 &viewTarget) const
{
	auto flexC = GetEntity().GetComponent<CFlexComponent>();
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto mdl = GetEntity().GetModel();
	if(flexC.expired() || mdlC.expired() || mdl == nullptr)
		return Vector3{};
	// Test
	/*auto t = std::fmodf(c_game->RealTime(),6.0) /6.0;
	auto t2 = std::fmodf(c_game->RealTime(),3.0) /3.0;
	auto s = umath::sin(t *umath::pi *2);
	auto s2 = umath::sin(t2 *umath::pi *2);
	auto f = 1.0;
	flexC->SetFlexController("eyes_updown",f *(s *90 -45));
	flexC->SetFlexController("eyes_rightleft",f *(s2 *90 -45));*/

	auto tmp = viewTarget;
	if(m_eyeAttachmentIndex != std::numeric_limits<uint32_t>::max())
	{
		Vector3 pos {};
		auto rot = uquat::identity();
		mdlC->GetAttachment(m_eyeAttachmentIndex,&pos,&rot);

		physics::Transform attPose;
		GetEntity().GetPose(attPose);
		attPose *= physics::Transform{pos,rot};
		auto localPos = attPose.GetInverse() *tmp;
		// FIXME: clamp distance to something based on eyeball distance
		if(localPos.z < 6)
			localPos.z = 6;

		auto flDist = uvec::length(localPos);
		uvec::normalize(&localPos);

		// calculate animated eye deflection

		Vector3 eyeDeflect {};

		auto eyeAng = EulerAngles{};
		if(m_eyeUpDownFlexController != std::numeric_limits<uint32_t>::max())
			flexC->GetFlexController(m_eyeUpDownFlexController,eyeAng.p);
		if(m_eyeLeftRightFlexController != std::numeric_limits<uint32_t>::max())
			flexC->GetFlexController(m_eyeLeftRightFlexController,eyeAng.y);

		angle_to_vector(eyeAng,eyeDeflect);
		eyeDeflect.x = 0;

		eyeDeflect = eyeDeflect *(localPos.z *localPos.z);
		localPos = localPos +Vector3(eyeDeflect.y,-eyeDeflect.z,eyeDeflect.x);
		uvec::normalize(&localPos);

		// check to see if the eye is aiming outside the max eye deflection
		auto flMaxEyeDeflection = umath::cos(umath::deg_to_rad(mdl->GetMaxEyeDeflection()));
		if(localPos.z < flMaxEyeDeflection)
		{
			// TODO: Unsure if this is correct, further testing required
			// if so, clamp it to 30 degrees offset
			// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 1, 0, "%5.3f %5.3f %5.3f", lcl.x, lcl.y, lcl.z );
			localPos.z = 0;
			auto d = uvec::length_sqr(localPos);
			if(d > 0.0)
			{
				d = umath::sqrt((1.0 -flMaxEyeDeflection *flMaxEyeDeflection) /(localPos.y *localPos.y +localPos.x *localPos.x));
				localPos.z = flMaxEyeDeflection;
				localPos.y = localPos.y * d;
				localPos.x = localPos.x * d;
			}
			else
				localPos.z = 1.0;
		}

		localPos = localPos *flDist;
		localPos = attPose *localPos;
		tmp = localPos;
	}
	return tmp;
}
void pragma::CEyeComponent::SetViewTarget(const Vector3 &viewTarget)
{
	m_viewTarget = ClampViewTarget(viewTarget);
}
pragma::physics::Transform pragma::CEyeComponent::CalcEyeballPose(uint32_t eyeballIndex,physics::Transform *optOutBonePose) const
{
	auto *eyeballData = GetEyeballData(eyeballIndex);
	auto mdl = GetEntity().GetModel();
	auto *eyeball = mdl ? mdl->GetEyeball(eyeballIndex) : nullptr;
	if(eyeballData == nullptr || eyeball == nullptr)
	{
		if(optOutBonePose)
			*optOutBonePose = {};
		return physics::Transform{};
	}
	auto &config = eyeballData->config;
	auto tmp = eyeball->origin;
	tmp.x = tmp.x +config.eyeShift.x *umath::sign(tmp.x);
	tmp.y = tmp.y +config.eyeShift.y *umath::sign(tmp.y);
	tmp.z = tmp.z +config.eyeShift.z *umath::sign(tmp.z);

	Vector3 bonePos;
	Quat boneRot;
	m_animC->GetGlobalBonePosition(eyeball->boneIndex,bonePos,boneRot);
	physics::Transform bonePose {bonePos,boneRot};
	if(optOutBonePose)
		*optOutBonePose = bonePose;
	return bonePose *physics::Transform{tmp,uquat::identity()};
}
bool pragma::CEyeComponent::GetEyeballProjectionVectors(uint32_t eyeballIndex,Vector4 &outProjU,Vector4 &outProjV) const
{
	auto *eyeballData = GetEyeballData(eyeballIndex);
	if(eyeballData == nullptr)
		return false;
	outProjU = eyeballData->state.irisProjectionU;
	outProjV = eyeballData->state.irisProjectionV;
	return true;
}
void pragma::CEyeComponent::UpdateEyeball(const Eyeball &eyeball,uint32_t eyeballIndex)
{
	if(eyeballIndex >= m_eyeballData.size() || m_animC.expired())
		return;
	auto &eyeballData = m_eyeballData.at(eyeballIndex);
	auto &state = eyeballData.state;
	auto &config = eyeballData.config;

	auto &viewTarget = GetViewTarget();

	// move eyeball into worldspace
	physics::Transform bonePose {};
	state.origin = CalcEyeballPose(eyeballIndex,&bonePose).GetOrigin();
	state.up = eyeball.up;
	uvec::rotate(&state.up,bonePose.GetRotation());

	// look directly at target
	state.forward = viewTarget -state.origin;
	uvec::normalize(&state.forward);

	if(config.eyeMove == false)
	{
		// TODO: What's this for?
		//VectorRotate( eyeball.forward, boneToWorld[eyeball.bone], pstate.forward )
		//pstate.forward = -1 *pstate.forward -- ???
	}

	state.right = uvec::cross(state.forward,state.up);
	uvec::normalize(&state.right);

	// shift N degrees off of the target
	auto dz = eyeball.zOffset;
	state.forward = state.forward +(eyeball.zOffset +dz) *state.right;

	// Jitter
	state.forward += umath::random(-config.jitter.x,config.jitter.x) *state.right;
	state.forward += umath::random(-config.jitter.y,config.jitter.y) *state.up;

	uvec::normalize(&state.forward);

	// re-aim eyes 
	state.right = uvec::cross(state.forward,state.up);
	uvec::normalize(&state.right);
		
	state.up = uvec::cross(state.right,state.forward);
	uvec::normalize(&state.up);

	auto scale = (1.0 /eyeball.irisScale) +config.eyeSize;
	if(scale > 0.0)
		scale = 1.0 /scale;

	auto org = state.origin;

	auto u = state.right *-static_cast<float>(scale);
	state.irisProjectionU.x = u.x;
	state.irisProjectionU.y = u.y;
	state.irisProjectionU.z = u.z;
	state.irisProjectionU.w = -uvec::dot(org,u);

	auto v = state.up *-static_cast<float>(scale);
	state.irisProjectionV.x = v.x;
	state.irisProjectionV.y = v.y;
	state.irisProjectionV.z = v.z;
	state.irisProjectionV.w = -uvec::dot(org,v);
}
