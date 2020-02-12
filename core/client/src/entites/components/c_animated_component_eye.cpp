#include "stdafx_client.h"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/model/c_model.h"

#pragma optimize("",off)
void pragma::CAnimatedComponent::UpdateEyeballs()
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

const pragma::CAnimatedComponent::EyeballConfig *pragma::CAnimatedComponent::GetEyeballConfig(uint32_t eyeballIndex) const {return const_cast<CAnimatedComponent*>(this)->GetEyeballConfig(eyeballIndex);}
pragma::CAnimatedComponent::EyeballConfig *pragma::CAnimatedComponent::GetEyeballConfig(uint32_t eyeballIndex)
{
	auto *data = GetEyeballData(eyeballIndex);
	return data ? &data->config : nullptr;
}
void pragma::CAnimatedComponent::SetEyeballConfig(const EyeballConfig &eyeballConfig) {m_eyeballConfig = eyeballConfig;}
const pragma::CAnimatedComponent::EyeballData *pragma::CAnimatedComponent::GetEyeballData(uint32_t eyeballIndex) const {return const_cast<CAnimatedComponent*>(this)->GetEyeballData(eyeballIndex);}
pragma::CAnimatedComponent::EyeballData *pragma::CAnimatedComponent::GetEyeballData(uint32_t eyeballIndex)
{
	return (eyeballIndex < m_eyeballData.size()) ? &m_eyeballData.at(eyeballIndex) : nullptr;
}

const Vector3 &pragma::CAnimatedComponent::GetViewTarget() const {return m_viewTarget;}

// TODO
static Vector3 convert_vertex(const Vector3 &v)
{
	return Vector3{v.x,-v.z,v.y};
}
static Vector3 convert_vertex_back(const Vector3 &v)
{
	return Vector3{v.x,v.z,-v.y};
}
static void AngleVectors (const EulerAngles &angles, Vector3 *forward)
{
	auto sy = umath::sin(umath::deg_to_rad(angles.y));
	auto cy = umath::cos(umath::deg_to_rad(angles.y));

	auto sp = umath::sin(umath::deg_to_rad(angles.p));
	auto cp = umath::cos(umath::deg_to_rad(angles.p));

	forward->x = cp*cy;
	forward->y = cp*sy;
	forward->z = -sp;
	*forward = convert_vertex(*forward);
}
// TOOD
static float		MaxEyeDeflection() { return 0.866f;}//flMaxEyeDeflection != 0.0f ? flMaxEyeDeflection : 0.866f; } // default to cos(30) if not set
Vector3 pragma::CAnimatedComponent::GetClampedViewTarget() const
{
	auto flexC = GetEntity().GetComponent<CFlexComponent>();
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto mdl = GetEntity().GetModel();
	if(flexC.expired() || mdlC.expired() || mdl == nullptr)
		return Vector3{};

	if (false)// !pStudioHdr )
		return Vector3( 0, 0, 0);

	// aim the eyes
	auto tmp = m_viewTarget;

	auto m_bSearchedForEyeFlexes = false; // TODO
	uint32_t m_iEyeUpdown = -1;
	uint32_t m_iEyeRightleft = -1;

	if ( !m_bSearchedForEyeFlexes )
	{
		m_bSearchedForEyeFlexes = true;
		mdl->GetFlexControllerId( "eyes_updown",m_iEyeUpdown );
		mdl->GetFlexControllerId( "eyes_rightleft",m_iEyeRightleft );

		if ( m_iEyeUpdown != -1 )
		{
			//pStudioHdr->pFlexcontroller( m_iEyeUpdown )->localToGlobal = AddGlobalFlexController( "eyes_updown" );
		}
		if ( m_iEyeRightleft != -1 )
		{
			//pStudioHdr->pFlexcontroller( m_iEyeRightleft )->localToGlobal = AddGlobalFlexController( "eyes_rightleft" );
		}
	}

	int32_t m_iEyeAttachment =mdlC->LookupAttachment("eyes"); // TODO
	if (m_iEyeAttachment != -1)
	{
		Vector3 pos {};
		Quat rot = uquat::identity();
		if (!mdlC->GetAttachment(m_iEyeAttachment,&pos,&rot))
		{
			return Vector3( 0, 0, 0);
		}
		tmp = CalcEyeballPose(1) *tmp; // TODO
		auto attPose = physics::Transform{pos,uquat::identity()};//rot}; // TODO
		auto local = tmp *attPose.GetInverse();
		local = convert_vertex_back(local);
		//Vector3 local;
		//VectorITransform( tmp, attToWorld, local );

		// FIXME: clamp distance to something based on eyeball distance
		if (local.x < 6)
		{
			local.x = 6;
		}
		float flDist = uvec::length(local);
		uvec::normalize(&local);

		// calculate animated eye deflection
		Vector3 eyeDeflect;
		EulerAngles eyeAng( 0, 0, 0 );
		if ( m_iEyeUpdown != -1 )
			eyeAng.p = flexC->GetFlexWeight(m_iEyeUpdown);

		if ( m_iEyeRightleft != -1 )
			eyeAng.y = flexC->GetFlexWeight(m_iEyeRightleft);

		// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 0, 0, "%5.3f %5.3f", eyeAng.x, eyeAng.y );

		AngleVectors( eyeAng, &eyeDeflect );
		eyeDeflect.x = 0;

		// reduce deflection the more the eye is off center
		// FIXME: this angles make no damn sense
		eyeDeflect = eyeDeflect * (local.x * local.x);
		local = local + eyeDeflect;
		uvec::normalize(&local);

		// check to see if the eye is aiming outside the max eye deflection
		float flMaxEyeDeflection = MaxEyeDeflection();
		if ( local.x < flMaxEyeDeflection )
		{
			// if so, clamp it to 30 degrees offset
			// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 1, 0, "%5.3f %5.3f %5.3f", local.x, local.y, local.z );
			local.x = 0;
			float d = uvec::length_sqr(local);
			if ( d > 0.0f )
			{
				d = sqrtf( ( 1.0f - flMaxEyeDeflection * flMaxEyeDeflection ) / ( local.y*local.y + local.z*local.z ) );
				local.x = flMaxEyeDeflection;
				local.y = local.y * d;
				local.z = local.z * d;
			}
			else
			{
				local.x = 1.0;
			}
		}
		local = local * flDist;
		local = convert_vertex(local);
		tmp = local *attPose;
		tmp = CalcEyeballPose(1).GetInverse() *tmp; // TODO
	}

	//modelrender->SetViewTarget( GetModelPtr(), GetBody(), tmp );

	/*
	debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 0, 0, "%.2f %.2f %.2f  : %.2f %.2f %.2f", 
	m_viewtarget.x, m_viewtarget.y, m_viewtarget.z, 
	m_prevviewtarget.x, m_prevviewtarget.y, m_prevviewtarget.z );
	*/

	return tmp;
}
void pragma::CAnimatedComponent::SetViewTarget(const Vector3 &viewTarget)
{
	m_viewTarget = viewTarget;
	//m_viewTarget = GetClampedViewTarget();

}

// TODO
static Vector4 GetMatAsVec4(const Mat3x4 m,int32_t i)
{
	//return Vector4(m[0][i],m[1][i],m[2][i],m[3][i]); // TODO: Might be the other way around (x,i)
	return Vector4(m[i][0],m[i][1],m[i][2],m[i][3]); // TODO: Might be the other way around (x,i)
}
static Vector3 GetMatAsVec(const Mat3x4 m,int32_t i)
{
	auto v = GetMatAsVec4(m,i);
	return Vector3(v.x,v.y,v.z);
}
static void SetMatAsVec(Mat3x4 &m,int32_t i,const Vector3 &v)
{
	//m[0][i] = v.x;
	//m[1][i] = v.y;
	//m[2][i] = v.z;
	m[i][0] = v.x;
	m[i][1] = v.y;
	m[i][2] = v.z;
}
pragma::physics::Transform pragma::CAnimatedComponent::CalcEyeballPose(uint32_t eyeballIndex) const
{
	auto mdl = GetEntity().GetModel();
	auto *eyeball = mdl ? mdl->GetEyeball(eyeballIndex) : nullptr;
	if(eyeball == nullptr)
		return physics::Transform{};
	Vector3 bonePos {};
	auto boneRot = uquat::identity();
	GetGlobalBonePosition(eyeball->boneIndex,bonePos,boneRot);
	physics::Transform bonePose {bonePos,boneRot};

	// TODO
	auto fConvertVertex = [](const Vector3 &v) {
		return Vector3{v.x,-v.z,v.y};
	};
	auto forward = fConvertVertex(eyeball->forward);
	auto up = fConvertVertex(eyeball->up);
	auto eyePose = physics::Transform{fConvertVertex(eyeball->origin),uquat::create_look_rotation(forward,up)};
	return bonePose *eyePose;
}
bool pragma::CAnimatedComponent::GetEyeballProjectionVectors(uint32_t eyeballIndex,Vector4 &outProjU,Vector4 &outProjV) const
{
	auto *eyeballData = GetEyeballData(eyeballIndex);
	if(eyeballData == nullptr)
		return false;
	outProjU = GetMatAsVec4(eyeballData->state.mat,0);
	outProjV = GetMatAsVec4(eyeballData->state.mat,1);
	return true;
}
void pragma::CAnimatedComponent::UpdateEyeball(const Eyeball &eyeball,uint32_t eyeballIndex)
{
	if(eyeballIndex >= m_eyeballData.size())
		return;

	// TODO: Material override parameters
	// TODO: Do we need the radius if we don't do raytracing?
	//const_cast<Eyeball&>(eyeball).radius = 0.5f;
	// TODO: dilation

	auto &eyeballData = m_eyeballData.at(eyeballIndex);
	auto &state = eyeballData.state;
	auto &config = eyeballData.config;

	auto &viewTarget = GetViewTarget();

	// move eyeball into worldspace
	auto tmp = convert_vertex(eyeball.origin);
	tmp.x = tmp.x +config.eyeShift.x *umath::sign(tmp.x);
	tmp.y = tmp.y +config.eyeShift.y *umath::sign(tmp.y);
	tmp.z = tmp.z +config.eyeShift.z *umath::sign(tmp.z);

	Vector3 bonePos;
	Quat boneRot;
	GetGlobalBonePosition(eyeball.boneIndex,bonePos,boneRot);
	physics::Transform bonePose {bonePos,boneRot};
	state.origin = bonePose *tmp;
	state.up = convert_vertex(eyeball.up);
	uvec::rotate(&state.up,bonePose.GetRotation());

	// TODO
	//VectorTransform( tmp, m_BoneToWorld[peyeball->bone], pstate->org );
	//VectorRotate( peyeball->up, m_BoneToWorld[peyeball->bone], pstate->up );
	// Bone Pos

	// look directly at target
	state.forward = viewTarget -state.origin;
	uvec::normalize(&state.forward);

	if(config.eyeMove == false)
	{
		// TODO
		//VectorRotate( eyeball.forward, boneToWorld[eyeball.bone], pstate.forward )
		//pstate.forward = -1 *pstate.forward -- ???
	}

	state.right = uvec::cross(state.forward,state.up);
	uvec::normalize(&state.right);

	// shift N degrees off of the target
	auto dz = eyeball.zOffset;
	state.forward = state.forward +(eyeball.zOffset +dz) *state.right;

	// Jitter
	// TODO: -0.02f,0.02f
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

	state.mat = Mat3x4(0);
	SetMatAsVec(state.mat,0,state.right *-static_cast<float>(scale));
	SetMatAsVec(state.mat,1,state.up *-static_cast<float>(scale));

	auto org = state.origin;// -CalcEyeballPose(eyeballIndex).GetOrigin();
	state.mat[0][3] = -uvec::dot(org,GetMatAsVec(state.mat,0) );// + 0.5;
	state.mat[1][3] = -uvec::dot(org,GetMatAsVec(state.mat,1) );// + 0.5;
}
#pragma optimize("",on)
