#include "mdl.h"
#include "mdl_animation.h"
#include "mdl_bone.h"
#include "quaternion64.h"
#include "quaternion48.h"
#include "vector48.h"
#include <pragma/model/animation/animation.h>
#pragma optimize("",off)
import::mdl::AnimationDesc::AnimationDesc(int32_t idx,const VFilePtr &f)
	: m_index(idx)
{
	auto offset = f->Tell();
	m_stdDesc = f->Read<mstudioanimdesc_t>();
	if(m_stdDesc.nameOffset != 0)
	{
		f->Seek(offset +m_stdDesc.nameOffset);
		auto name = f->ReadString();
		if(!name.empty() && name.front() == '@')
			name = name.substr(1);
		m_name = name;
	}
	m_frameCount = m_stdDesc.frameCount;
	m_fps = m_stdDesc.fps;

	f->Seek(offset +sizeof(mstudioanimdesc_t));
}

std::vector<std::shared_ptr<import::mdl::Animation>> &import::mdl::AnimationDesc::GetSectionAnimations() const {return m_sectionAnimations;}

void import::mdl::AnimationDesc::ReadMovements(const mdl::studiohdr_t &mdlHeader,uint64_t offset,uint64_t relOffset,const VFilePtr &f)
{
	auto &stdAnimDesc = GetStudioDesc();
	f->Seek(relOffset +stdAnimDesc.movementOffset);
	m_movements.resize(stdAnimDesc.movementCount);
	f->Read(m_movements.data(),stdAnimDesc.movementCount *sizeof(m_movements.front()));
}

void import::mdl::AnimationDesc::ReadAnimation(const mdl::studiohdr_t &mdlHeader,uint64_t offset,uint64_t relOffset,const VFilePtr &f)
{
	auto &stdAnimDesc = GetStudioDesc();
	if(stdAnimDesc.animBlock == 0 && ((stdAnimDesc.flags &STUDIO_ALLZEROS) == 0))
	{
		if(stdAnimDesc.flags &STUDIO_FRAMEANIM)
			;
		else
		{
			if(stdAnimDesc.sectionOffset != 0 && stdAnimDesc.sectionFrameCount > 0)
			{
				auto sectionFrameCount = stdAnimDesc.sectionFrameCount;
				auto sectionCount = static_cast<int32_t>(stdAnimDesc.frameCount /stdAnimDesc.sectionFrameCount) +2;
				for(auto i=decltype(sectionCount){0};i<sectionCount;++i)
					;
				f->Seek(offset +sizeof(stdAnimDesc) *m_index +stdAnimDesc.sectionOffset);

				std::vector<mstudioanimsections_t> stdAnimSections(sectionCount);
				f->Read(stdAnimSections.data(),stdAnimSections.size() *sizeof(stdAnimSections.front()));

				if(stdAnimDesc.animBlock == 0)
				{
					for(auto sectionIndex=decltype(sectionCount){0};sectionIndex<sectionCount;++sectionIndex)
					{
						f->Seek(offset +sizeof(stdAnimDesc) *m_index +m_sections[sectionIndex].animOffset);
						if(sectionIndex < sectionCount -2)
							sectionFrameCount = stdAnimDesc.sectionFrameCount;
						else
							sectionFrameCount = stdAnimDesc.frameCount -((sectionCount -2) *stdAnimDesc.sectionFrameCount);

						m_animation = std::make_shared<Animation>(mdlHeader,stdAnimDesc,f);
					}
				}
			}
			else if(stdAnimDesc.animBlock == 0)
			{
				f->Seek(offset +sizeof(stdAnimDesc) *m_index +stdAnimDesc.animOffset);
				m_animation = std::make_shared<Animation>(mdlHeader,stdAnimDesc,f);
			}
		}
	}
	if(stdAnimDesc.movementCount > 0)
		ReadMovements(mdlHeader,offset,relOffset,f);
}

void import::mdl::AnimationDesc::AdjustPositionAndRotation(const import::MdlInfo &info,uint32_t boneIndex,const Vector3 &position,const Vector3 &rotation,Vector3 &outPosition,Vector3 &outRotation)
{
	auto &bone = info.bones.at(boneIndex);
	if(bone->GetParent() == nullptr)
	{
		outPosition = Vector3{position.y,-position.x,position.z};
		outRotation = rotation;
		outRotation.z += umath::deg_to_rad(-90.f);
	}
	else
	{
		outPosition = position;
		outRotation = rotation;
	}
}

void import::mdl::AnimationDesc::AdjustPositionAndRotationByPiecewiseMovement(const import::MdlInfo &info,uint32_t frameIndex,uint32_t boneIndex,const Vector3 &position,const Vector3 &rotation,Vector3 &outPosition,Vector3 &outRotation)
{
	auto &bone = info.bones.at(boneIndex);
	outPosition = position;
	outRotation = rotation;
	auto &boneParent = bone->GetParent();
	if(boneParent != nullptr || m_movements.empty() || frameIndex == 0u)
		return;
	Vector3 vecPos {};
	Vector3 vecAngle {};
	auto previousFrameIndex = 0u;
	for(auto &movement : m_movements)
	{
		if(frameIndex <= movement.endframe)
		{
			auto f = (frameIndex -previousFrameIndex) /static_cast<double>(movement.endframe -previousFrameIndex);
			auto d = movement.v0 *f +0.5 *(movement.v1 -movement.v0) *f *f;
			vecPos += static_cast<float>(d) *movement.vector;
			vecAngle.y = vecAngle.y *(1.f -f) +umath::deg_to_rad(movement.angle) *f;
			break;
		}
		previousFrameIndex = movement.endframe;
		vecPos = movement.position;
		vecAngle.y = umath::deg_to_rad(movement.angle);
	}
	outPosition = position +vecPos;
	outRotation.z += vecAngle.y;
}

std::shared_ptr<::Animation> import::mdl::AnimationDesc::CalcAnimation(const import::MdlInfo &info)
{
	auto &stdAnimDesc = GetStudioDesc();
	auto &header = info.header;
	auto &bones = info.bones;
	auto anim = ::Animation::Create();
	anim->SetFPS(stdAnimDesc.fps);
	anim->ReserveBoneIds(anim->GetBoneCount() +header.numbones);
	for(auto i=decltype(header.numbones){0};i<header.numbones;++i)
		anim->AddBoneId(i);
	for(auto frameIndex=decltype(stdAnimDesc.frameCount){0};frameIndex<stdAnimDesc.frameCount;++frameIndex)
	{
		// Calculate animation
		auto sectionIndex = stdAnimDesc.sectionFrameCount;
		auto sectionFrameIndex = frameIndex;
		if(sectionIndex != 0)
		{
			sectionIndex = static_cast<int32_t>(frameIndex /stdAnimDesc.sectionFrameCount);
			sectionFrameIndex = frameIndex -(sectionIndex *stdAnimDesc.sectionFrameCount);
		}
		const std::shared_ptr<import::mdl::Animation::Data> *frameData = nullptr;
		const std::vector<std::shared_ptr<import::mdl::Animation::Data>> *frames = nullptr;
		uint32_t animIndex = 0;
		auto bUseAnim = m_sectionAnimations.empty();
		if(bUseAnim == false)
		{
			if(sectionIndex >= m_sectionAnimations.size())
				bUseAnim = true;
			else
				frames = &m_sectionAnimations[sectionIndex]->GetData();
		}
		if(bUseAnim == true)
		{
			auto &anim = GetAnimation();
			if(anim == nullptr)
				continue;
			frames = &anim->GetData();
		}
		frameData = &(*frames)[animIndex];
			
		auto frame = Frame::Create(header.numbones);
		for(auto j=decltype(header.numbones){0};j<header.numbones;++j)
		{
			auto *stdAnim = (frameData != nullptr) ? &(*frameData)->GetStudioAnim() : nullptr;
			auto &bone = bones[j];
			auto boneId = bone->GetID();
			Vector3 boneAng {};
			Vector3 bonePos {};
			if(stdAnim != nullptr && stdAnim->boneIndex == boneId)
			{
				auto ang = (*frameData)->CalcBoneRotation(sectionFrameIndex,*bone);
				auto pos = (*frameData)->CalcBonePosition(sectionFrameIndex,*bone);
				frame->SetBonePosition(stdAnim->boneIndex,pos);
				frame->SetBoneOrientation(stdAnim->boneIndex,{});
				boneAng = ang;
				bonePos = pos;

				++animIndex;
				if(animIndex >= frames->size())
					frameData = nullptr;
				else
					frameData = &(*frames)[animIndex];
			}
			else
			{
				if(stdAnimDesc.flags &STUDIO_DELTA)
				{
					frame->SetBonePosition(boneId,{});
					frame->SetBoneOrientation(boneId,{});
				}
				else
				{
					auto ang = bone->GetAngles();
					frame->SetBonePosition(boneId,bone->GetPos());
					frame->SetBoneOrientation(boneId,bone->GetRot());
					boneAng = ang;
					bonePos = bone->GetPos();
				}
			}

			auto tmpPos = bonePos;
			auto tmpAng = boneAng;
			//AdjustPositionAndRotationByPiecewiseMovement(info,frameIndex,boneId,tmpPos,tmpAng,bonePos,boneAng);
			tmpPos = bonePos;
			tmpAng = boneAng;
			if(!(stdAnimDesc.flags &STUDIO_DELTA)) // TODO: This does not match Crowbar, but works for the antlion guard's charge hit attack animation
				AdjustPositionAndRotation(info,boneId,tmpPos,tmpAng,bonePos,boneAng);

			auto angBone = Vector3{boneAng.x,boneAng.y,boneAng.z};
			frame->SetBonePosition(boneId,{-bonePos.x,bonePos.y,bonePos.z});
			auto rot = Quat{0.0,angBone.x,angBone.y,angBone.z};
			//if(angBone.x == 0.f && angBone.y == 0.f && angBone.z == 0.f)
			//	std::cout<<"";
			//Quat rot {0.f,1.f,0.f,0.f};
			//if(umath::abs(angBone.x) > 0.001f || umath::abs(angBone.y) > 0.001f || umath::abs(angBone.z) > 0.001f)
			//	rot = Quat{0.0,angBone.x,angBone.y,angBone.z};

			frame->SetBoneOrientation(boneId,rot);

			auto *posBone = frame->GetBonePosition(boneId);
			auto *rotBone = frame->GetBoneOrientation(boneId);
		}
		anim->AddFrame(frame);
		//
	}
	if(m_movements.empty() == false)
	{
		auto flags = anim->GetFlags();
		struct MoveSegment
		{
			int32_t startFrame = -1;
			int32_t endFrame = -1;
			Vector3 startVel;
			Vector3 endVel;
		};
		std::vector<MoveSegment> movementSegments;
		auto fps = anim->GetFPS();
		auto velocityScaleFactor = 1.f;//1.f /static_cast<float>(fps);
		for(auto &move : m_movements)
		{
			if(move.motionflags &STUDIO_LX)
				flags |= FAnim::MoveZ;
			if(move.motionflags &STUDIO_LY)
				flags |= FAnim::MoveX;
			auto mvVec = move.vector;
			umath::swap(mvVec.x,mvVec.y); // z (Up-axis in source) is unused
			umath::negate(mvVec.x);
			umath::negate(mvVec.y);
			auto startVel = mvVec *move.v0;
			auto endVel = mvVec *move.v1;
			auto endFrame = move.endframe;

			movementSegments.push_back({});
			auto &moveSegment = movementSegments.back();
			moveSegment.endFrame = endFrame;
			moveSegment.startVel = startVel *velocityScaleFactor;
			moveSegment.endVel = endVel *velocityScaleFactor;
		}
		std::sort(movementSegments.begin(),movementSegments.end(),[](const MoveSegment &m0,const MoveSegment &m1) {
			return (m0.endFrame < m1.endFrame) ? true : false;
		});
		for(auto it=movementSegments.begin();it!=movementSegments.end();++it)
		{
			auto &mvSeg = *it;
			mvSeg.startFrame = (it != movementSegments.begin()) ? ((it -1)->endFrame +1) : 0;
		}
		anim->SetFlags(flags);

		for(auto &mvSeg : movementSegments)
		{
			auto frameRange = mvSeg.endFrame -mvSeg.startFrame;
			for(auto i=mvSeg.startFrame;i<=mvSeg.endFrame;++i)
			{
				auto factor = (frameRange > 0) ? ((i -mvSeg.startFrame) /static_cast<float>(frameRange)) : 1.f;
				auto v = uvec::lerp(mvSeg.startVel,mvSeg.endVel,factor);
				auto frame = anim->GetFrame(i);
				if(frame == nullptr)
					std::cout<<"WARNING: Invalid frame #"<<i<<" for move segment!"<<std::endl;
				else
					frame->SetMoveOffset(v.x,v.y);
			}
		}
		// Fill up empty move offsets
		auto numFrames = anim->GetFrameCount();
		for(auto i=decltype(numFrames){0};i<numFrames;++i)
		{
			auto &frame = *anim->GetFrame(i);
			if(frame.GetMoveOffset() == nullptr)
				frame.SetMoveOffset({});
		}
	}
	return anim;
}

void import::mdl::AnimationDesc::ReadAnimationSections(const mdl::studiohdr_t &mdlHeader,uint64_t offset,const VFilePtr &f)
{
	auto &stdAnimDesc = GetStudioDesc();
	if(!(stdAnimDesc.flags &STUDIO_ALLZEROS))
	{
		if(stdAnimDesc.flags &STUDIO_FRAMEANIM)
			;
		else
		{
			if(stdAnimDesc.sectionOffset != 0 && stdAnimDesc.sectionFrameCount > 0)
			{
				auto sectionCount = static_cast<int32_t>(stdAnimDesc.frameCount /stdAnimDesc.sectionFrameCount) +2;
				m_sections.reserve(sectionCount);
				f->Seek(offset +stdAnimDesc.sectionOffset);
				for(auto i=decltype(sectionCount){0};i<sectionCount;++i)
					m_sections.push_back(f->Read<mdl::mstudioanimsection_t>());
				if(stdAnimDesc.animBlock == 0)
				{
					auto numSections = m_sections.size();
					for(auto i=decltype(numSections){0};i<numSections;++i)
					{
						auto &stdSection = m_sections[i];
						f->Seek(offset +stdSection.animOffset);
						auto sectionFrameCount = (i < numSections -2) ? stdAnimDesc.sectionFrameCount : (stdAnimDesc.frameCount -((numSections -2) *stdAnimDesc.sectionFrameCount));
						m_sectionAnimations.push_back(std::make_shared<Animation>(mdlHeader,m_stdDesc,f));
					}
				}
			}
		}
	}
}

const std::vector<import::mdl::mstudioanimsection_t> &import::mdl::AnimationDesc::GetStudioSections() const {return m_sections;}
int32_t import::mdl::AnimationDesc::GetFrameCount() const {return m_frameCount;}
float import::mdl::AnimationDesc::GetFPS() const {return m_fps;}
const std::string &import::mdl::AnimationDesc::GetName() const {return m_name;}

void import::mdl::AnimationDesc::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Name: "<<GetName()<<std::endl;
	std::cout<<t<<"Frame Count: "<<GetFrameCount()<<std::endl;
	std::cout<<t<<"FPS: "<<GetFPS()<<std::endl;
//	std::cout<<t<<"Bone Count: "<<m_frames.front()->GetBoneTransforms().size()<<std::endl;
}


const std::shared_ptr<import::mdl::Animation> &import::mdl::AnimationDesc::GetAnimation() const {return m_animation;}
const import::mdl::mstudioanimdesc_t &import::mdl::AnimationDesc::GetStudioDesc() const {return m_stdDesc;}

////////////////////////

static float extract_anim_value(int32_t frameIndex,const std::vector<import::mdl::mstudioanimvalue_t> &animValues,float scale)
{
	auto k = frameIndex;

	frameIndex = 0;
	uint32_t animValueIndex = 0;
	while(animValues[animValueIndex].num.total <= k)
	{
		k -= animValues[animValueIndex].num.total;
		animValueIndex += animValues[animValueIndex].num.valid +1;
		if(animValueIndex >= animValues.size() || animValues[animValueIndex].num.total == 0)
			return 0.f;
	}
	if(animValues[animValueIndex].num.valid > k)
		return animValues[animValueIndex +k +1].value *scale;
	return animValues[animValueIndex +animValues[animValueIndex].num.valid].value *scale;
}

import::mdl::Animation::Data::Data(const VFilePtr &f)
{
	m_stdAnim = f->Read<mstudioanim_t>();
}

const import::mdl::mstudioanim_t &import::mdl::Animation::Data::GetStudioAnim() const {return m_stdAnim;}
const std::array<std::vector<import::mdl::mstudioanimvalue_t>,3> &import::mdl::Animation::Data::GetRotValues() const {return m_rotValues;}
const std::array<std::vector<import::mdl::mstudioanimvalue_t>,3> &import::mdl::Animation::Data::GetPosValues() const {return m_posValues;}
const Quaternion48 import::mdl::Animation::Data::GetRot48() const {return m_rot48;}
const Quaternion64 import::mdl::Animation::Data::GetRot64() const {return m_rot64;}
const Vector48 import::mdl::Animation::Data::GetPos48() const {return m_pos48;}

static Vector3 matrix_to_euler_angles(const Mat4 &M,int32_t i,int32_t j,int32_t k,bool bRepeat)
{
	Vector3 ea {};
	if(bRepeat == true)
	{
		auto sy = sqrtf(M[i][j] * M[i][j] + M[i][k] * M[i][k]);
		if(sy > 16 *FLT_EPSILON)
		{
			ea.x = umath::atan2(M[i][j], M[i][k]);
			ea.y = umath::atan2(sy,M[i][i]);
			ea.z = umath::atan2(M[j][i], -M[k][i]);
		}
		else
		{
			ea.x = umath::atan2(-M[j][k], M[j][j]);
			ea.y = umath::atan2(sy, M[i][i]);
			ea.z = 0;
		}
	}
	else
	{
		auto cy = sqrtf(M[i][i] * M[i][i] + M[j][i] * M[j][i]);
		if(cy > 16 * FLT_EPSILON)
		{
			ea.x = umath::atan2(M[k][j], M[k][k]);
			ea.y = umath::atan2(-M[k][i], cy);
			ea.z = umath::atan2(M[j][i], M[i][i]);
		}
		else
		{
			ea.x = umath::atan2(-M[j][k], M[j][j]);
			ea.y = umath::atan2(-M[k][i], cy);
			ea.z = 0;
		}
	}
	return ea;
}

Vector3 import::mdl::quaternion_to_euler_angles(const Quat &q)
{
	int32_t i = 0;
	int32_t j = 1;
	int32_t k = 2;
	auto repeat = false;

	auto Nq = q.x *q.x +q.y *q.y +q.z *q.z +q.w *q.w;
	auto s = 0.0;
	if(Nq > 0.0)
		s = 2.0 /Nq;
	auto xs = q.x *s;
	auto ys = q.y *s;
	auto zs = q.z *s;

	auto wx = q.w *xs;
	auto wy = q.w *ys;
	auto wz = q.w *zs;
	auto xx = q.x *xs;
	auto xy = q.x *ys;
	auto xz = q.x *zs;
	auto yy = q.y *ys;
	auto yz = q.y *zs;
	auto zz = q.z *zs;

	Mat4 M {};
	M[0][0] = 1.0 -(yy +zz);
	M[0][1] = xy -wz;
	M[0][2] = xz +wy;
	M[1][0] = xy +wz;
	M[1][1] = 1.0 -(xx +zz);
	M[1][2] = yz -wx;
	M[2][0] = xz -wy;
	M[2][1] = yz +wx;
	M[2][2] = 1.0 -(xx +yy);
	M[3][3] = 1.0;

	return matrix_to_euler_angles(M,i,j,k,repeat);
}

Quat import::mdl::euler_angles_to_quaternion(const Vector3 &ang)
{
	auto fSinPitch = umath::sin(ang.x *0.5f);
	auto fCosPitch = umath::cos(ang.x *0.5f);
	auto fSinYaw = umath::sin(ang.y *0.5f);
	auto fCosYaw = umath::cos(ang.y *0.5f);
	auto fSinRoll = umath::sin(ang.z *0.5f);
	auto fCosRoll = umath::cos(ang.z *0.5f);
	auto fCosPitchCosYaw = fCosPitch *fCosYaw;
	auto fSinPitchSinYaw = fSinPitch *fSinYaw;
	return Quat(
		fCosRoll *fCosPitchCosYaw +fSinRoll *fSinPitchSinYaw,
		fSinRoll *fCosPitchCosYaw -fCosRoll *fSinPitchSinYaw,
		fCosRoll *fSinPitch *fCosYaw +fSinRoll *fCosPitch *fSinYaw,
		fCosRoll *fCosPitch *fSinYaw -fSinRoll *fSinPitch *fCosYaw
	);
}

Vector3 import::mdl::Animation::Data::CalcBoneRotation(uint32_t frameIndex,const Bone &bone) const
{
	auto &stdAnim = GetStudioAnim();
	if(stdAnim.flags &STUDIO_ANIM_RAWROT)
		return quaternion_to_euler_angles(GetRot48()());
	else if(stdAnim.flags &STUDIO_ANIM_RAWROT2)
		return quaternion_to_euler_angles(GetRot64()());
	
	if(!(stdAnim.flags &STUDIO_ANIM_ANIMROT))
	{
		if(stdAnim.flags &STUDIO_ANIM_DELTA)
			return {0.f,0.f,0.f};
		return bone.GetAngles();
	}
	Vector3 ang {};
	auto &rotScale = bone.GetRotScale();
	auto &rotValues = GetRotValues();
	for(int8_t i=0;i<3;++i)
	{
		if(rotValues[i].empty())
			continue;
		ang[i] = extract_anim_value(frameIndex,rotValues[i],rotScale[i]);
	}

	if(!(stdAnim.flags &STUDIO_ANIM_DELTA))
		ang += bone.GetAngles();
	return ang;
}

Vector3 import::mdl::Animation::Data::CalcBonePosition(uint32_t frameIndex,const Bone &bone) const
{
	auto &stdAnim = GetStudioAnim();
	if(stdAnim.flags &STUDIO_ANIM_RAWPOS)
		return GetPos48().ToVector3();
	else if(!(stdAnim.flags &STUDIO_ANIM_ANIMPOS))
	{
		if(stdAnim.flags &STUDIO_ANIM_DELTA)
			return {};
		return bone.GetPos();
	}
	Vector3 pos {};
	auto &posScale = bone.GetPosScale();
	auto &posValues = GetPosValues();
	for(int8_t i=0;i<3;++i)
	{
		if(posValues[i].empty())
			continue;
		pos[i] = extract_anim_value(frameIndex,posValues[i],posScale[i]);
	}

	if(!(stdAnim.flags &STUDIO_ANIM_DELTA))
		pos += bone.GetPos();
	return pos;
}

////////////////////////

void import::mdl::Animation::ReadMdlAnimValues(const mdl::studiohdr_t &header,const import::mdl::mstudioanimdesc_t &stdAnimDesc,std::vector<mstudioanimvalue_t> &stdAnimValues,const VFilePtr &f)
{
	auto frameCountRemainingToBeChecked = stdAnimDesc.frameCount;
	stdAnimValues.reserve(frameCountRemainingToBeChecked);
	while(frameCountRemainingToBeChecked > 0)
	{
		stdAnimValues.push_back(f->Read<mstudioanimvalue_t>());
		auto &animValue = stdAnimValues.back();
		auto currentTotal = animValue.num.total;
		if(currentTotal == 0)
		{
			stdAnimValues.erase(stdAnimValues.end() -1);
			break;
		}
		frameCountRemainingToBeChecked -= currentTotal;

		auto validCount = animValue.num.valid;
		for(auto i=decltype(validCount){0};i<validCount;++i)
			stdAnimValues.push_back(f->Read<mstudioanimvalue_t>());
	}
}

import::mdl::Animation::Animation(const mdl::studiohdr_t &mdlHeader,const import::mdl::mstudioanimdesc_t &stdAnimDesc,const VFilePtr &f)
{
	auto offset = f->Tell();

	m_data.reserve(mdlHeader.numbones);
	for(auto i=decltype(mdlHeader.numbones){0};i<mdlHeader.numbones;++i)
	{
		auto offset = f->Tell();
		m_data.push_back(std::shared_ptr<Data>(new Data(f)));

		auto &anim = m_data.back();
		auto &stdAnim = anim->GetStudioAnim();
		if(stdAnim.boneIndex >= mdlHeader.numbones)
			;//break;
		if((stdAnim.flags &STUDIO_LX) || (stdAnim.flags &STUDIO_LY))
			;//break;

		if(stdAnim.flags &STUDIO_ANIM_DELTA)
			;
		if(stdAnim.flags &STUDIO_ANIM_RAWROT2)
			anim->m_rot64 = f->Read<Quaternion64>();
		if(stdAnim.flags &STUDIO_ANIM_RAWROT)
			anim->m_rot48 = f->Read<Quaternion48>();
		if(stdAnim.flags &STUDIO_ANIM_RAWPOS)
			anim->m_pos48 = f->Read<Vector48>();

		std::array<mstudioanim_valueptr_t,3> rotOffsets;
		auto rotOffset = f->Tell();
		if(stdAnim.flags &STUDIO_ANIM_ANIMROT)
			rotOffsets = f->Read<decltype(rotOffsets)>();

		std::array<mstudioanim_valueptr_t,3> posOffsets;
		auto posOffset = f->Tell();
		if(stdAnim.flags &STUDIO_ANIM_ANIMPOS)
			posOffsets = f->Read<decltype(posOffsets)>();
		if(stdAnim.flags &STUDIO_ANIM_ANIMROT)
		{
			auto &rotValues = anim->m_rotValues;
			for(auto i=decltype(rotOffsets.size()){0};i<rotOffsets.size();++i)
			{
				auto &v = rotOffsets[i];
				if(v == 0)
					continue;
				f->Seek(rotOffset +v);
				ReadMdlAnimValues(mdlHeader,stdAnimDesc,rotValues[i],f);
			}
		}
		if(stdAnim.flags &STUDIO_ANIM_ANIMPOS)
		{
			auto &posValues = anim->m_posValues;
			for(auto i=decltype(posOffsets.size()){0};i<posOffsets.size();++i)
			{
				auto &v = posOffsets[i];
				if(v == 0)
					continue;
				f->Seek(posOffset +v);
				ReadMdlAnimValues(mdlHeader,stdAnimDesc,posValues[i],f);
			}
		}
		if(stdAnim.nextSourceMdlAnimationOffset == 0)
			break;
		f->Seek(offset +stdAnim.nextSourceMdlAnimationOffset);
	}
}

short ftofix16(float num)
{
 
	short i, f;
   
	if (fabs(num) > 2047.999f) {
		printf("Error: number out of range (num=%f)\n", num);
	}
 
	i = (short)num;
	f = (short)(fabs(num * 16)) & 15;
	return (i << 4) | f;
}

float fix16tof(int n)
{
	float s = 1.0f;
	if (n < 0) {
		s = -1.0f;
		n = -n;
	}
	return s * ((float)(n >> 4) + ((n & 15) / 16.0f));
}

//////////////////////

Quat import::mdl::Animation::CalcBoneRotation(const mdl::mstudiobone_t &bone,const mstudioanim_t &panim,const std::array<mstudioanimvalue_t,3> &animValues,int32_t frame,float s)
{
	// Source: https://github.com/marrub--/hlua/blob/master/utils/mdlviewer/studio_render.cpp
	Quat rot {};

	Vector4 q1;
	Vector4 q2;
	Vector3 angle1;
	Vector3 angle2;

	for(auto i=decltype(animValues.size()){0};i<animValues.size();++i)
	{
		auto &animVal = animValues[i];
		auto *panimvalue = &animVal;
		//if(panim->offset[i +3] == 0)
		//	angle2[i] = angle1[i] = pbone->value[i +3]; // default;
		//else
		{
			auto k = frame;
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid +1;
			}
			// Bah, missing blend!
			if(panimvalue->num.valid > k)
			{
				angle1[i] = panimvalue[k +1].value;

				if (panimvalue->num.valid > k +1)
					angle2[i] = panimvalue[k +2].value;
				else
				{
					if(panimvalue->num.total > k +1)
						angle2[i] = angle1[i];
					else
						angle2[i] = panimvalue[panimvalue->num.valid +2].value;
				}
			}
			else
			{
				angle1[i] = panimvalue[panimvalue->num.valid].value;
				if (panimvalue->num.total > k + 1)
					angle2[i] = angle1[i];
				else
					angle2[i] = panimvalue[panimvalue->num.valid +2].value;
			}
			//angle1[i] = pbone->value[i +3] +angle1[i] *pbone->scale[i +3];
			//angle2[i] = pbone->value[i +3] +angle2[i] *pbone->scale[i +3];
		}

		/*if(bone.bonecontroller[i +3] != -1)
		{
			angle1[j] += m_adj[bone.bonecontroller[i +3]];
			angle2[j] += m_adj[bone.bonecontroller[i+3]];
		}*/ // TODO
	}

	/*if (!VectorCompare( angle1, angle2 ))
	{
		AngleQuaternion( angle1, q1 );
		AngleQuaternion( angle2, q2 );
		QuaternionSlerp( q1, q2, s, q );
	}
	else
	{
		AngleQuaternion( angle1, q );
	}*/
	return rot;
}

Vector3 import::mdl::Animation::CalcBonePosition(const mdl::Bone &bone,const std::array<mstudioanimvalue_t,3> &animValues,int32_t frame,float s)
{
	// Source: https://github.com/marrub--/hlua/blob/master/utils/mdlviewer/studio_render.cpp
	auto &posScale = bone.GetPosScale();
	Vector3 pos {};
	for(auto i=decltype(animValues.size()){0};i<animValues.size();++i)
	{
		auto &animVal = animValues[i];
		auto k = frame;

		auto *panimvalue = &animVal;
		// find span of values that includes the frame we want
		while(panimvalue->num.total <= k)
		{
			k -= panimvalue->num.total;
			panimvalue += panimvalue->num.valid +1;
		}
		// if we're inside the span
		if(panimvalue->num.valid > k)
		{
			// and there's more data in the span
			if(panimvalue->num.valid > k +1)
				pos[i] += (panimvalue[k +1].value *(1.0 -s) +s *panimvalue[k +2].value) *posScale[i];
			else
				pos[i] += panimvalue[k +1].value *posScale[i];
		}
		else
		{
			// are we at the end of the repeating values section and there's another section with data?
			if(panimvalue->num.total <= k +1)
				pos[i] += (panimvalue[panimvalue->num.valid].value *(1.0 -s) +s *panimvalue[panimvalue->num.valid +2].value) *posScale[i];
			else
				pos[i] += panimvalue[panimvalue->num.valid].value *posScale[i];
		}
		//if(bone.bonecontroller[i] != -1)
		//	pos[i] += m_adj[bone.bonecontroller[i]]; // TODO
	}
	return pos;
}


import::mdl::Animation::Animation(const std::vector<std::shared_ptr<mdl::Bone>> &bones,uint32_t animBlockOffset,const std::vector<mdl::mstudioanimblock_t> &animBlocks,const VFilePtr &f)
{}

const std::vector<std::shared_ptr<import::mdl::Animation::Data>> &import::mdl::Animation::GetData() const {return m_data;}
#pragma optimize("",on)