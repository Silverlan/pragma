/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/frame.h"
#include "pragma/model/modelmesh.h"
#include "pragma/model/model.h"

static unsigned int get_bone_index(const std::vector<unsigned int> &boneList,unsigned int id)
{
	for(UInt i=0;i<boneList.size();i++)
	{
		if(boneList[i] == id)
			return CUInt32(i);
	}
	return 0;
}

static void get_global_bone_transforms(const Animation &anim,const Skeleton &skeleton,Frame &frame)
{
	auto &boneList = anim.GetBoneList();
	std::function<void(Frame&,const std::unordered_map<uint32_t,std::shared_ptr<Bone>>&,const Vector3&,const Quat&)> fGetGlobalBoneTransforms;
	fGetGlobalBoneTransforms = [&fGetGlobalBoneTransforms,&boneList](Frame &frame,const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,const Vector3 &posParent,const Quat &rotParent) {
		for(auto &pair : bones)
		{
			//auto &parent = pair.second->parent;
			auto idx = get_bone_index(boneList,pair.first);

			auto &bone = pair.second;
			assert(bone->ID == pair.first);
			auto &pos = *frame.GetBonePosition(idx);
			auto &rot = *frame.GetBoneOrientation(idx);
			uvec::rotate(&pos,rotParent);
			pos += posParent;
			rot = rotParent *rot;

			fGetGlobalBoneTransforms(frame,bone->children,pos,rot);
		}
	};
	fGetGlobalBoneTransforms(frame,skeleton.GetRootBones(),{},uquat::identity());
}

static void get_local_bone_transforms(const Animation &anim,const Skeleton &skeleton,Frame &frame)
{
	std::function<void(const Animation&,Frame&,const std::unordered_map<uint32_t,std::shared_ptr<Bone>>&)> fGetLocalBoneTransforms;
	fGetLocalBoneTransforms = [&fGetLocalBoneTransforms](const Animation &anim,Frame &frame,const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones) {
		auto &boneList = anim.GetBoneList();
		for(auto it=bones.begin();it!=bones.end();++it)
		{
			auto &bone = it->second;
			fGetLocalBoneTransforms(anim,frame,bone->children);

			auto parent = bone->parent.lock();
			if(parent != nullptr)
			{
				auto idx = get_bone_index(boneList,it->first);
				assert(bone->ID == it->first);
				auto parentIdx = get_bone_index(boneList,parent->ID);

				auto &pos = *frame.GetBonePosition(idx);
				pos -= *frame.GetBonePosition(parentIdx);
				auto inv = uquat::get_inverse(*frame.GetBoneOrientation(parentIdx));
				uvec::rotate(&pos,inv);
				auto &rot = *frame.GetBoneOrientation(idx);
				rot = inv *rot;
			}
		}
	};
	fGetLocalBoneTransforms(anim,frame,skeleton.GetRootBones());
}
/*
static void get_global_bone_transforms(Animation *anim,Frame *frame,std::unordered_map<unsigned int,Bone*> &bones,const Vector3 &origin=Vector3(0.f,0.f,0.f),const Quat &originRot=uquat::identity())
{
	std::vector<unsigned int> *boneList = anim->GetBoneList();
	for(auto it=bones.begin();it!=bones.end();++it)
	{
		auto *bone = it->second;

		auto idx = get_bone_index(boneList,it->first);
		assert(bone->ID == it->first);

		auto &pos = *frame->GetBonePosition(idx);
		uvec::rotate(&pos,originRot);
		pos += origin;
		auto &rot = *frame->GetBoneOrientation(idx);
		rot = originRot *rot;
		//t.UpdateMatrix();

		get_global_bone_transforms(anim,frame,bone->children,pos,rot);
	}
}

static void get_global_bone_transforms(Animation *anim,Skeleton *skeleton,Frame *frame,const Vector3 &origin=Vector3(0.f,0.f,0.f),const Quat &originRot=uquat::identity())
{
	get_global_bone_transforms(anim,frame,*skeleton->GetHierarchy(),origin,originRot);
}
*/
std::shared_ptr<Frame> Frame::Create(unsigned int numBones)
{
	return std::shared_ptr<Frame>(new Frame{numBones});
}
std::shared_ptr<Frame> Frame::Create(const Frame &other)
{
	return std::shared_ptr<Frame>(new Frame{other});
}

Frame::Frame(unsigned int numBones)
	: m_move(nullptr)
{
	m_bones.resize(numBones);
	std::fill(m_bones.begin(),m_bones.end(),umath::Transform{});
}

Frame::Frame(const Frame &other)
	: m_move(nullptr)
{
	m_bones = other.m_bones;
	m_scales = other.m_scales;
	if(other.m_move != nullptr)
		m_move = std::make_unique<Vector2>(*other.m_move);
}


const FlexFrameData &Frame::GetFlexFrameData() const {return const_cast<Frame*>(this)->GetFlexFrameData();}
FlexFrameData &Frame::GetFlexFrameData() {return m_flexFrameData;}

std::vector<uint32_t> Frame::GetLocalRootBoneIds(const Animation &anim,const Skeleton &skeleton) const
{
	auto &boneIds = anim.GetBoneList();
	auto &rootBones = skeleton.GetRootBones();
	std::vector<uint32_t> localRootBoneIds;
	localRootBoneIds.reserve(rootBones.size());
	for(auto i=decltype(boneIds.size()){0};i<boneIds.size();++i)
	{
		auto boneId = boneIds[i];
		auto it = rootBones.find(boneId);
		if(it == rootBones.end())
			continue;
		localRootBoneIds.push_back(static_cast<uint32_t>(i));
	}
	return localRootBoneIds;
}

void Frame::Rotate(const Quat &rot)
{
	for(auto &pose : m_bones)
		pose.RotateGlobal(rot);
}
void Frame::Translate(const Vector3 &t)
{
	for(auto &pose : m_bones)
		pose.TranslateGlobal(t);
}

void Frame::Rotate(const Skeleton &skeleton,const Quat &rot)
{
	for(auto &pair : skeleton.GetRootBones())
	{
		auto id = pair.first;
		m_bones.at(id).RotateGlobal(rot);
	}
}
void Frame::Translate(const Skeleton &skeleton,const Vector3 &t)
{
	for(auto &pair : skeleton.GetRootBones())
	{
		auto id = pair.first;
		m_bones.at(id).TranslateGlobal(t);
	}
}

void Frame::Rotate(const Animation &anim,const Skeleton &skeleton,const Quat &rot)
{
	auto localRootBoneIds = GetLocalRootBoneIds(anim,skeleton);
	for(auto id : localRootBoneIds)
		m_bones.at(id).RotateGlobal(rot);
}
void Frame::Translate(const Animation &anim,const Skeleton &skeleton,const Vector3 &t)
{
	auto localRootBoneIds = GetLocalRootBoneIds(anim,skeleton);
	for(auto id : localRootBoneIds)
		m_bones.at(id).TranslateGlobal(t);
}
void Frame::Scale(const Vector3 &scale)
{
	for(auto &t : m_bones)
		t.SetOrigin(t.GetOrigin() *scale);
}

const std::vector<umath::Transform> &Frame::GetBoneTransforms() const {return const_cast<Frame*>(this)->GetBoneTransforms();}
const std::vector<Vector3> &Frame::GetBoneScales() const {return const_cast<Frame*>(this)->GetBoneScales();}
std::vector<umath::Transform> &Frame::GetBoneTransforms() {return m_bones;}
std::vector<Vector3> &Frame::GetBoneScales() {return m_scales;}
umath::Transform *Frame::GetBoneTransform(uint32_t idx) {return (idx < m_bones.size()) ? &m_bones.at(idx) : nullptr;}
const umath::Transform *Frame::GetBoneTransform(uint32_t idx) const {return const_cast<Frame*>(this)->GetBoneTransform(idx);}
bool Frame::GetBonePose(uint32_t boneId,umath::ScaledTransform &outTransform) const
{
	if(boneId >= m_bones.size())
		return false;
	auto &t = m_bones.at(boneId);
	outTransform = umath::ScaledTransform{t.GetOrigin(),t.GetRotation(),(boneId < m_scales.size()) ? m_scales.at(boneId) : Vector3{1.f,1.f,1.f}};
	return true;
}
void Frame::SetBonePose(uint32_t boneId,const umath::ScaledTransform &pose)
{
	if(boneId >= m_bones.size())
		return;
	m_bones.at(boneId) = pose;
	if(boneId < m_scales.size())
		m_scales.at(boneId) = pose.GetScale();
}

void Frame::SetBonePose(uint32_t boneId,const umath::Transform &pose)
{
	if(boneId >= m_bones.size())
		return;
	m_bones.at(boneId) = pose;
}

/*
	All animations are usually localized by the model compiler, except for the bind/reference pose, which is created on the fly when loading the model.
	This is the only case where ::Localize is called here.
*/
void Frame::Localize(const Animation &anim,const Skeleton &skeleton)
{
	get_local_bone_transforms(anim,skeleton,*this);
}

void Frame::Globalize(const Animation &anim,const Skeleton &skeleton)
{
	get_global_bone_transforms(anim,skeleton,*this);
}

Vector2 *Frame::GetMoveOffset() {return m_move.get();}
void Frame::GetMoveOffset(float *x,float *z)
{
	if(m_move == nullptr)
	{
		*x = 0.f;
		*z = 0.f;
		return;
	}
	*x = m_move->x;
	*z = m_move->y;
}
uint32_t Frame::GetBoneCount() const {return static_cast<uint32_t>(m_bones.size());}
void Frame::SetBoneCount(uint32_t numBones) {m_bones.resize(numBones);}
void Frame::SetMoveOffset(float x,float z)
{
	if(m_move == nullptr)
		m_move = std::make_unique<Vector2>(0,0);
	m_move->x = x;
	m_move->y = z;
}
void Frame::SetMoveOffset(Vector2 move)
{
	SetMoveOffset(move.x,move.y);
}
void Frame::SetBonePosition(unsigned int boneID,const Vector3 &pos)
{
	if(boneID >= m_bones.size())
		return;
	m_bones.at(boneID).SetOrigin(pos);
}
void Frame::SetBoneOrientation(unsigned int boneID,const Quat &orientation)
{
	if(boneID >= m_bones.size())
		return;
	m_bones.at(boneID).SetRotation(orientation);
}
void Frame::UpdateScales()
{
	if(m_scales.size() != m_bones.size())
		m_scales.resize(m_bones.size(),Vector3(1.f,1.f,1.f));
}
void Frame::SetBoneScale(uint32_t boneId,const Vector3 &scale)
{
	UpdateScales();
	if(boneId >= m_scales.size())
		return;
	m_scales.at(boneId) = scale;
}
const Vector3 *Frame::GetBoneScale(uint32_t boneId) const {return const_cast<Frame&>(*this).GetBoneScale(boneId);}
Vector3 *Frame::GetBoneScale(uint32_t boneId)
{
	if(boneId >= m_scales.size())
		return nullptr;
	return &m_scales.at(boneId);
}
const Vector3 *Frame::GetBonePosition(unsigned int boneID) const {return const_cast<Frame&>(*this).GetBonePosition(boneID);}
const Quat *Frame::GetBoneOrientation(unsigned int boneID) const {return const_cast<Frame&>(*this).GetBoneOrientation(boneID);}
Vector3 *Frame::GetBonePosition(unsigned int boneID)
{
	if(boneID >= m_bones.size())
		return nullptr;
	return &m_bones.at(boneID).GetOrigin();
}
Quat *Frame::GetBoneOrientation(unsigned int boneID)
{
	if(boneID >= m_bones.size())
		return nullptr;
	return &m_bones.at(boneID).GetRotation();
}
bool Frame::GetBoneMatrix(unsigned int boneID,Mat4 *mat)
{
	if(boneID >= m_bones.size())
		return false;
	*mat = m_bones.at(boneID).ToMatrix();
	return true;
}
bool Frame::HasScaleTransforms() const {return !m_scales.empty();}
std::pair<Vector3,Vector3> Frame::CalcRenderBounds(const Animation &anim,const Model &mdl) const
{
	auto *t = const_cast<Frame*>(this);
	auto transforms = m_bones;
	t->Globalize(anim,mdl.GetSkeleton());
	auto numBones = GetBoneCount();
	std::vector<Mat4> boneMatrices(numBones);
	auto &reference = mdl.GetReference();
	for(auto i=decltype(boneMatrices.size()){0};i<boneMatrices.size();++i)
	{
		auto *posBind = reference.GetBonePosition(static_cast<uint32_t>(i));
		auto *rotBind = reference.GetBoneOrientation(static_cast<uint32_t>(i));
		if(posBind == nullptr || rotBind == nullptr)
			continue;
		auto &mat = boneMatrices[i];
		auto &pos = *t->GetBonePosition(static_cast<uint32_t>(i));
		auto rot = *t->GetBoneOrientation(static_cast<uint32_t>(i)) *glm::inverse(*rotBind);
		mat = glm::translate(mat,*posBind);
		mat = mat *glm::toMat4(rot);
		mat = glm::translate(mat,-(*posBind));
		mat = glm::translate(pos -(*posBind)) *mat;
	}
	std::pair<Vector3,Vector3> renderBounds = {
		{std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()},
		{std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()}
	};
	for(auto &meshGroup : mdl.GetMeshGroups())
	{
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto &verts = subMesh->GetVertices();
				auto &vertWeights = subMesh->GetVertexWeights();
				if(vertWeights.size() < verts.size())
					continue;
				for(auto i=decltype(verts.size()){0};i<verts.size();++i)
				{
					auto &v = verts[i];
					auto &weight = vertWeights[i];
					Vector4 pos {};
					Vector4 vpos {v.position.x,v.position.y,v.position.z,0.f};
					for(uint8_t j=0;j<4;++j)
					{
						auto *posBone = t->GetBonePosition(weight.boneIds[j]);
						if(posBone == nullptr)
							continue;
						pos += weight.weights[j] *(boneMatrices[weight.boneIds[j]] *vpos);
					}
					for(uint8_t j=0;j<3;++j)
					{
						if(pos[j] < renderBounds.first[j])
							renderBounds.first[j] = pos[j];
						if(pos[j] > renderBounds.second[j])
							renderBounds.second[j] = pos[j];
					}
				}
			}
		}
	}
	t->m_bones = transforms;
	return renderBounds;
}
