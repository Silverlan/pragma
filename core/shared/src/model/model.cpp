/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/model/model.h"
#include <pragma/engine.h>
#include "materialmanager.h"
#include "pragma/model/animation/activities.h"
#include "pragma/model/animation/meta_rig.hpp"
#include <mathutil/umath.h>
#include "pragma/physics/collisionmesh.h"
#include "pragma/model/modelmesh.h"
#include <sharedutils/util_string.h>
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/model/animation/flex_animation.hpp"
#include "pragma/file_formats/wmd.h"
#include "pragma/asset/util_asset.hpp"
#include "pragma/model/animation/skeleton.hpp"
#include "pragma/model/animation/bone.hpp"
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_ifile.hpp>
#include <stack>

extern DLLNETWORK Engine *engine;

std::shared_ptr<ModelMeshGroup> ModelMeshGroup::Create(const std::string &name) { return std::shared_ptr<ModelMeshGroup>(new ModelMeshGroup {name}); }
std::shared_ptr<ModelMeshGroup> ModelMeshGroup::Create(const ModelMeshGroup &other)
{
	auto r = std::shared_ptr<ModelMeshGroup>(new ModelMeshGroup {other.m_name});
	r->m_meshes.reserve(other.m_meshes.size());
	for(auto &mesh : other.m_meshes)
		r->m_meshes.push_back(mesh->Copy());
	return r;
}
ModelMeshGroup::ModelMeshGroup(const std::string &name) : m_name(name) {}
bool ModelMeshGroup::operator==(const ModelMeshGroup &other) const { return this == &other; }
bool ModelMeshGroup::operator!=(const ModelMeshGroup &other) const { return !operator==(other); }
const std::string &ModelMeshGroup::GetName() const { return m_name; }
std::vector<std::shared_ptr<ModelMesh>> &ModelMeshGroup::GetMeshes() { return m_meshes; }
void ModelMeshGroup::AddMesh(const std::shared_ptr<ModelMesh> &mesh) { m_meshes.push_back(mesh); }
uint32_t ModelMeshGroup::GetMeshCount() const { return static_cast<uint32_t>(m_meshes.size()); }
bool ModelMeshGroup::IsEqual(const ModelMeshGroup &other) const
{
	static_assert(sizeof(ModelMeshGroup) == 72, "Update this function when making changes to this class!");
	if(!(m_name == other.m_name && m_meshes.size() == other.m_meshes.size()))
		return false;
	for(auto i = decltype(m_meshes.size()) {0u}; i < m_meshes.size(); ++i) {
		if(m_meshes[i]->IsEqual(*other.m_meshes[i]) == false)
			return false;
	}
	return true;
}

/////////////////////////////////////

bool Eyeball::LidFlexDesc::operator==(const LidFlexDesc &other) const
{
	static_assert(sizeof(LidFlexDesc) == 28, "Update this function when making changes to this class!");
	return lidFlexIndex == other.lidFlexIndex && raiserFlexIndex == other.raiserFlexIndex && neutralFlexIndex == other.neutralFlexIndex && lowererFlexIndex == other.lowererFlexIndex && umath::abs(raiserValue - other.raiserValue) < 0.001f
	  && umath::abs(neutralValue - other.neutralValue) < 0.001f && umath::abs(lowererValue - other.lowererValue) < 0.001f;
}

bool Eyeball::operator==(const Eyeball &other) const
{
	static_assert(sizeof(Eyeball) == 152, "Update this function when making changes to this class!");
	return name == other.name && boneIndex == other.boneIndex && uvec::cmp(origin, other.origin) && umath::abs(zOffset - other.zOffset) < 0.001f && umath::abs(radius - other.radius) < 0.001f && uvec::cmp(up, other.up) && uvec::cmp(forward, other.forward)
	  && irisMaterialIndex == other.irisMaterialIndex && umath::abs(maxDilationFactor - other.maxDilationFactor) < 0.001f && umath::abs(irisUvRadius - other.irisUvRadius) < 0.001f && umath::abs(irisScale - other.irisScale) < 0.001f && upperLid == other.upperLid
	  && lowerLid == other.lowerLid;
}

/////////////////////////////////////

bool Model::MetaInfo::operator==(const MetaInfo &other) const
{
	static_assert(sizeof(MetaInfo) == 80, "Update this function when making changes to this class!");
	return includes == other.includes && texturePaths == other.texturePaths && textures == other.textures && flags == other.flags;
}

std::unordered_map<std::string, std::shared_ptr<Model>> Model::m_models;

Con::c_cout &operator<<(Con::c_cout &os, const Model &mdl)
{
	os << "Model[" << mdl.m_name << "]";
	return os;
}

Model::MetaInfo::MetaInfo() {}

Model::Model() : m_reference(Frame::Create(0)) { Construct(); }

Model::Model(NetworkState *nw, uint32_t numBones, const std::string &name) : Model()
{
	m_networkState = nw;
	m_name = name;
	for(auto i = decltype(numBones) {0}; i < numBones; ++i)
		m_bindPose.push_back(umat::identity());
}

Model::Model(const Model &other)
    : m_networkState(other.m_networkState), m_metaInfo(other.m_metaInfo), m_stateFlags(other.m_stateFlags), m_mass(other.m_mass), m_blendControllers(other.m_blendControllers), m_bodyGroups(other.m_bodyGroups), m_hitboxes(other.m_hitboxes), m_name(other.m_name),
      m_animationIDs(other.m_animationIDs), m_bindPose(other.m_bindPose), m_collisionMin(other.m_collisionMin), m_collisionMax(other.m_collisionMax), m_renderMin(other.m_renderMin), m_renderMax(other.m_renderMax), m_joints(other.m_joints), m_baseMeshes(other.m_baseMeshes),
      m_lods(other.m_lods), m_attachments(other.m_attachments), m_materials(other.m_materials), m_textureGroups(other.m_textureGroups), m_skeleton(std::make_unique<pragma::animation::Skeleton>(*other.m_skeleton)), m_reference(Frame::Create(*other.m_reference)),
      m_vertexCount(other.m_vertexCount), m_triangleCount(other.m_triangleCount), m_flexControllers(other.m_flexControllers), m_flexes(other.m_flexes), m_phonemeMap(other.m_phonemeMap)
{
	m_stateFlags |= StateFlags::AllMaterialsLoaded;
	m_meshGroups.reserve(other.m_meshGroups.size());
	for(auto &meshGroup : other.m_meshGroups)
		m_meshGroups.push_back(ModelMeshGroup::Create(*meshGroup));
	m_animations.reserve(other.m_animations.size());
	for(auto &anim : other.m_animations)
		m_animations.push_back(pragma::animation::Animation::Create(*anim));

	m_vertexAnimations.reserve(other.m_vertexAnimations.size());
	for(auto &anim : other.m_vertexAnimations)
		m_vertexAnimations.push_back(anim->Copy());

	m_collisionMeshes.reserve(other.m_collisionMeshes.size());
	for(auto &mesh : other.m_collisionMeshes)
		m_collisionMeshes.push_back(CollisionMesh::Create(*mesh));

	// Copy extension data
	std::stringstream extStream {};
	ufile::OutStreamFile extStreamFileOut {std::move(extStream)};
	other.m_extensions->Write(extStreamFileOut);

	m_extensions = udm::Property::Create(udm::Type::Element);
	ufile::InStreamFile extStreamFileIn {std::move(extStreamFileOut.MoveStream())};
	m_extensions->Read(extStreamFileIn);
	//
}

Model::~Model()
{
	for(auto &hCb : m_matLoadCallbacks) {
		if(hCb.IsValid() == true)
			hCb.Remove();
	}
	for(auto &hCb : m_onAllMatsLoadedCallbacks) {
		if(hCb.IsValid() == true)
			hCb.Remove();
	}
	m_joints.clear();
}

bool Model::IsEqual(const Model &other) const
{
	if(!(m_metaInfo == other.m_metaInfo && m_mass == other.m_mass && m_meshCount == other.m_meshCount && m_subMeshCount == other.m_subMeshCount && m_vertexCount == other.m_vertexCount && m_triangleCount == other.m_triangleCount
	     && umath::abs(m_maxEyeDeflection - other.m_maxEyeDeflection) < 0.0001f))
		return false;
	if(!(m_phonemeMap == other.m_phonemeMap && m_blendControllers == other.m_blendControllers && m_bodyGroups == other.m_bodyGroups && m_hitboxes == other.m_hitboxes && m_eyeballs == other.m_eyeballs /* && m_name == other.m_name*/))
		return false;
	if(!(m_animationIDs.size() == other.m_animationIDs.size() && m_flexAnimationNames == other.m_flexAnimationNames &&
	     /*m_bindPose == other.m_bindPose && */ uvec::cmp(m_eyeOffset, other.m_eyeOffset) && uvec::cmp(m_collisionMin, other.m_collisionMin) && uvec::cmp(m_collisionMax, other.m_collisionMax)))
		return false;
	if(!(m_flexControllers == other.m_flexControllers))
		return false;
	if(!(m_flexes == other.m_flexes))
		return false;
	if(!(uvec::cmp(m_renderMin, other.m_renderMin) && uvec::cmp(m_renderMax, other.m_renderMax) && m_joints == other.m_joints && m_baseMeshes == other.m_baseMeshes && m_lods == other.m_lods && m_attachments == other.m_attachments && m_objectAttachments == other.m_objectAttachments))
		return false;
	if(!(m_textureGroups == other.m_textureGroups && m_collisionMeshes.size() == other.m_collisionMeshes.size() && m_flexAnimations.size() == other.m_flexAnimations.size() && m_ikControllers.size() == other.m_ikControllers.size() && m_animations.size() == other.m_animations.size()
	     && m_meshGroups.size() == other.m_meshGroups.size() && static_cast<bool>(m_reference) == static_cast<bool>(other.m_reference) && static_cast<bool>(m_skeleton) == static_cast<bool>(other.m_skeleton)))
		return false;
	for(auto &pair : m_animationIDs) {
		if(other.m_animationIDs.find(pair.first) == other.m_animationIDs.end())
			return false;
	}
	for(auto i = decltype(m_meshGroups.size()) {0u}; i < m_meshGroups.size(); ++i) {
		if(m_meshGroups[i]->IsEqual(*other.m_meshGroups[i]) == false)
			return false;
	}
	for(auto i = decltype(m_animations.size()) {0u}; i < m_animations.size(); ++i) {
		if(*m_animations[i] != *other.m_animations[i])
			return false;
	}
	for(auto i = decltype(m_vertexAnimations.size()) {0u}; i < m_vertexAnimations.size(); ++i) {
		if(*m_vertexAnimations[i] != *other.m_vertexAnimations[i])
			return false;
	}
	for(auto i = decltype(m_ikControllers.size()) {0u}; i < m_ikControllers.size(); ++i) {
		if(*m_ikControllers[i] != *other.m_ikControllers[i])
			return false;
	}
	for(auto i = decltype(m_flexAnimations.size()) {0u}; i < m_flexAnimations.size(); ++i) {
		if(*m_flexAnimations[i] != *other.m_flexAnimations[i])
			return false;
	}
	for(auto i = decltype(m_collisionMeshes.size()) {0u}; i < m_collisionMeshes.size(); ++i) {
		if(*m_collisionMeshes[i] != *other.m_collisionMeshes[i])
			return false;
	}
	if(m_reference && *m_reference != *other.m_reference)
		return false;
	if(m_skeleton && *m_skeleton != *other.m_skeleton)
		return false;
#ifdef _WIN32
	static_assert(sizeof(Model) == 1024, "Update this function when making changes to this class!");
#endif
	return true;
}
bool Model::operator==(const Model &other) const { return this == &other; }
bool Model::operator!=(const Model &other) const { return !operator==(other); }
Model &Model::operator=(const Model &other)
{
	m_networkState = other.m_networkState;
	m_metaInfo = other.m_metaInfo;
	m_stateFlags = other.m_stateFlags;
	m_mass = other.m_mass;
	m_meshCount = other.m_meshCount;
	m_subMeshCount = other.m_subMeshCount;
	m_vertexCount = other.m_vertexCount;
	m_triangleCount = other.m_triangleCount;
	m_maxEyeDeflection = other.m_maxEyeDeflection;
	m_phonemeMap = other.m_phonemeMap;
	m_blendControllers = other.m_blendControllers;
	m_meshGroups = other.m_meshGroups;
	m_bodyGroups = other.m_bodyGroups;
	m_hitboxes = other.m_hitboxes;
	m_eyeballs = other.m_eyeballs;
	m_reference = other.m_reference;
	m_name = other.m_name;
	m_animations = other.m_animations;
	m_vertexAnimations = other.m_vertexAnimations;
	m_animationIDs = other.m_animationIDs;
	m_skeleton = other.m_skeleton;

	m_flexControllers = other.m_flexControllers;
	m_flexes = other.m_flexes;

	m_ikControllers = other.m_ikControllers;

	m_flexAnimations = other.m_flexAnimations;
	m_flexAnimationNames = other.m_flexAnimationNames;

	m_bindPose = other.m_bindPose;
	m_eyeOffset = other.m_eyeOffset;
	m_collisionMin = other.m_collisionMin;
	m_collisionMax = other.m_collisionMax;
	m_renderMin = other.m_renderMin;
	m_renderMax = other.m_renderMax;
	m_collisionMeshes = other.m_collisionMeshes;
	m_joints = other.m_joints;
	m_baseMeshes = other.m_baseMeshes;
	m_lods = other.m_lods;
	m_attachments = other.m_attachments;
	m_objectAttachments = other.m_objectAttachments;
	m_materials = other.m_materials;
	m_textureGroups = other.m_textureGroups;
	m_matLoadCallbacks = other.m_matLoadCallbacks;
	m_onAllMatsLoadedCallbacks = other.m_onAllMatsLoadedCallbacks;
	return *this;
}

const PhonemeMap &Model::GetPhonemeMap() const { return const_cast<Model *>(this)->GetPhonemeMap(); }
PhonemeMap &Model::GetPhonemeMap() { return m_phonemeMap; }
udm::PropertyWrapper Model::GetExtensionData() const { return *m_extensions; }
void Model::Rotate(const Quat &rot)
{
	uvec::rotate(&m_collisionMin, rot);
	uvec::rotate(&m_collisionMax, rot);
	uvec::rotate(&m_renderMin, rot);
	uvec::rotate(&m_renderMax, rot);
	auto &skeleton = GetSkeleton();
	for(auto &colMesh : m_collisionMeshes)
		colMesh->Rotate(rot);
	for(auto &anim : m_animations) {
		if(anim->HasFlag(FAnim::Gesture) == true)
			continue; // Don't rotate delta animations
		anim->Rotate(skeleton, rot);
	}
	for(auto &vertAnim : m_vertexAnimations)
		vertAnim->Rotate(rot);
	for(auto &meshGroup : m_meshGroups) {
		for(auto &mesh : meshGroup->GetMeshes())
			mesh->Rotate(rot);
	}
	m_reference->Rotate(rot);
	GenerateBindPoseMatrices();
}

void Model::Translate(const Vector3 &t)
{
	m_collisionMin += t;
	m_collisionMax += t;
	m_renderMin += t;
	m_renderMax += t;
	auto &skeleton = GetSkeleton();
	for(auto &colMesh : m_collisionMeshes)
		colMesh->Translate(t);
	for(auto &anim : m_animations) {
		if(anim->HasFlag(FAnim::Gesture) == true)
			continue; // Don't rotate delta animations
		anim->Translate(skeleton, t);
	}
	for(auto &meshGroup : m_meshGroups) {
		for(auto &mesh : meshGroup->GetMeshes())
			mesh->Translate(t);
	}
	m_reference->Translate(t);
	GenerateBindPoseMatrices();
}

void Model::Scale(const Vector3 &scale)
{
	m_collisionMin *= scale;
	m_collisionMax *= scale;
	m_renderMin *= scale;
	m_renderMax *= scale;
	for(auto &colMesh : m_collisionMeshes)
		colMesh->Scale(scale);
	for(auto &anim : m_animations)
		anim->Scale(scale);
	for(auto &vertAnim : m_vertexAnimations)
		vertAnim->Scale(scale);
	for(auto &meshGroup : m_meshGroups) {
		for(auto &mesh : meshGroup->GetMeshes())
			mesh->Scale(scale);
	}
	m_reference->Scale(scale);
	GenerateBindPoseMatrices();
}

void Model::Mirror(pragma::Axis axis)
{
	Vector3 transform {1.f, 1.f, 1.f};
	m_collisionMin *= transform;
	m_collisionMax *= transform;
	uvec::to_min_max(m_collisionMin, m_collisionMax);

	m_renderMin *= transform;
	m_renderMax *= transform;
	uvec::to_min_max(m_renderMin, m_renderMax);

	for(auto &eb : m_eyeballs) {
		eb.forward *= transform;
		eb.origin *= transform;
		eb.up *= transform;
	}
	for(auto &[id, hb] : m_hitboxes) {
		hb.min *= transform;
		hb.max *= transform;
		uvec::to_min_max(hb.min, hb.max);
	}
	m_eyeOffset *= transform;

	for(auto &att : m_attachments) {
		att.offset *= transform;
		auto rot = uquat::create(att.angles);
		uquat::mirror_on_axis(rot, umath::to_integral(axis));
		att.angles = EulerAngles {rot};
	}
	for(auto &cmesh : m_collisionMeshes)
		cmesh->Mirror(axis);
	if(m_metaRig) {
		uquat::mirror_on_axis(m_metaRig->forwardFacingRotationOffset, umath::to_integral(axis));
		m_metaRig->min *= transform;
		m_metaRig->max *= transform;
		uvec::to_min_max(m_metaRig->min, m_metaRig->max);
	}
	for(auto &mg : m_meshGroups) {
		for(auto &m : mg->GetMeshes())
			m->Mirror(axis);
	}
	for(auto &va : m_vertexAnimations)
		va->Mirror(axis);
	m_reference->Mirror(axis);
	for(auto &anim : m_animations)
		anim->Mirror(axis);
	m_reference->Mirror(axis);

	GenerateBindPoseMatrices();
}

void Model::GenerateBindPoseMatrices()
{
	auto &bones = GetSkeleton().GetBones();
	for(auto i = decltype(bones.size()) {0}; i < bones.size(); ++i) {
		auto &pos = *m_reference->GetBonePosition(i);
		auto &rot = *m_reference->GetBoneOrientation(i);

		auto m = glm::toMat4(rot);
		m = glm::translate(m, pos);
		SetBindPoseBoneMatrix(i, m);
	}
}

uint32_t Model::GetVertexCount() const { return m_vertexCount; }
uint32_t Model::GetTriangleCount() const { return m_triangleCount; }
Model::MetaInfo &Model::GetMetaInfo() const { return m_metaInfo; }
void Model::GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const { return GetBodyGroupMeshes(bodyGroups, 0, outMeshes); }
void Model::GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, uint32_t lod, std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const
{
	auto meshIds = const_cast<Model *>(this)->GetBaseMeshes();
	meshIds.reserve(meshIds.size() + m_bodyGroups.size());
	for(auto i = decltype(bodyGroups.size()) {0}; i < bodyGroups.size(); ++i) {
		auto bg = bodyGroups[i];
		auto meshGroupId = std::numeric_limits<uint32_t>::max();
		if(const_cast<Model *>(this)->GetMesh(static_cast<uint32_t>(i), bg, meshGroupId) == true && meshGroupId != std::numeric_limits<uint32_t>::max())
			meshIds.push_back(meshGroupId);
	}
	const_cast<Model *>(this)->TranslateLODMeshes(lod, meshIds);
	const_cast<Model *>(this)->GetMeshes(meshIds, outMeshes);
}
void Model::GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, std::vector<std::shared_ptr<ModelSubMesh>> &outMeshes) const { return GetBodyGroupMeshes(bodyGroups, 0, outMeshes); }
void Model::GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, uint32_t lod, std::vector<std::shared_ptr<ModelSubMesh>> &outMeshes) const
{
	auto meshIds = const_cast<Model *>(this)->GetBaseMeshes();
	meshIds.reserve(meshIds.size() + m_bodyGroups.size());
	for(auto i = decltype(bodyGroups.size()) {0}; i < bodyGroups.size(); ++i) {
		auto bg = bodyGroups[i];
		auto meshGroupId = std::numeric_limits<uint32_t>::max();
		if(const_cast<Model *>(this)->GetMesh(static_cast<uint32_t>(i), bg, meshGroupId) == true && meshGroupId != std::numeric_limits<uint32_t>::max())
			meshIds.push_back(meshGroupId);
	}
	const_cast<Model *>(this)->TranslateLODMeshes(lod, meshIds);
	const_cast<Model *>(this)->GetSubMeshes(meshIds, outMeshes);
}
BodyGroup &Model::AddBodyGroup(const std::string &name)
{
	auto id = GetBodyGroupId(name);
	if(id == -1) {
		m_bodyGroups.push_back(BodyGroup(name));
		id = static_cast<int32_t>(m_bodyGroups.size() - 1);
	}
	return m_bodyGroups[id];
}
int32_t Model::GetBodyGroupId(const std::string &name)
{
	for(auto i = decltype(m_bodyGroups.size()) {0}; i < m_bodyGroups.size(); ++i) {
		auto &bg = m_bodyGroups[i];
		if(bg.name == name)
			return static_cast<int32_t>(i);
	}
	return -1;
}
BodyGroup *Model::GetBodyGroup(uint32_t id)
{
	if(id >= m_bodyGroups.size())
		return nullptr;
	return &m_bodyGroups[id];
}
uint32_t Model::GetBodyGroupCount() const { return static_cast<uint32_t>(m_bodyGroups.size()); }
std::vector<BodyGroup> &Model::GetBodyGroups() { return m_bodyGroups; }
void Model::Remove() { delete this; }
bool Model::IsRootBone(uint32_t boneId) const { return m_skeleton->IsRootBone(boneId); }

bool Model::GetReferenceBonePose(pragma::animation::BoneId boneId, umath::Transform &outPose, umath::CoordinateSpace space) const
{
	auto &ref = GetReference();
	auto numBones = ref.GetBoneCount();
	if(boneId >= numBones)
		return false;
	umath::ScaledTransform scaledPose;
	if(!GetReferenceBonePose(boneId, scaledPose, space))
		return false;
	outPose = scaledPose;
	return true;
}
bool Model::GetReferenceBonePose(pragma::animation::BoneId boneId, umath::ScaledTransform &outPose, umath::CoordinateSpace space) const { return GetReferenceBonePose(boneId, &outPose.GetOrigin(), &outPose.GetRotation(), &outPose.GetScale(), space); }
bool Model::GetReferenceBonePos(pragma::animation::BoneId boneId, Vector3 &outPos, umath::CoordinateSpace space) const { return GetReferenceBonePose(boneId, &outPos, nullptr, nullptr, space); }
bool Model::GetReferenceBoneRot(pragma::animation::BoneId boneId, Quat &outRot, umath::CoordinateSpace space) const { return GetReferenceBonePose(boneId, nullptr, &outRot, nullptr, space); }
bool Model::GetReferenceBoneScale(pragma::animation::BoneId boneId, Vector3 &outScale, umath::CoordinateSpace space) const { return GetReferenceBonePose(boneId, nullptr, nullptr, &outScale, space); }
bool Model::GetReferenceBonePose(pragma::animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale, umath::CoordinateSpace space) const
{
	auto &ref = GetReference();
	auto *t = ref.GetBoneTransform(boneId);
	if(!t)
		return false;
	switch(space) {
	case umath::CoordinateSpace::Object:
	case umath::CoordinateSpace::World:
		{
			if(optOutPos)
				*optOutPos = t->GetOrigin();
			if(optOutRot)
				*optOutRot = t->GetRotation();
			if(optOutScale) {
				auto *scale = ref.GetBoneScale(boneId);
				*optOutScale = scale ? *scale : uvec::IDENTITY_SCALE;
			}
			return true;
		}
	case umath::CoordinateSpace::Local:
		{
			auto &skel = GetSkeleton();
			auto bone = skel.GetBone(boneId).lock();
			if(!bone)
				return false;
			umath::ScaledTransform parentPose;
			umath::ScaledTransform pose;
			auto parent = bone->parent.lock();
			if(parent)
				GetReferenceBonePose(parent->ID, parentPose, umath::CoordinateSpace::Object);
			GetReferenceBonePose(boneId, pose, umath::CoordinateSpace::Object);
			pose = parentPose.GetInverse() * pose;
			if(optOutPos)
				*optOutPos = pose.GetOrigin();
			if(optOutRot)
				*optOutRot = pose.GetRotation();
			if(optOutScale)
				*optOutScale = pose.GetScale();
			return true;
		}
	}
	return false;
}

bool Model::GetLocalBonePosition(uint32_t animId, uint32_t frameId, uint32_t boneId, Vector3 &rPos, Quat &rRot, Vector3 *scale)
{
	rPos = Vector3 {0.f, 0.f, 0.f};
	rRot = uquat::identity();
	if(scale != nullptr)
		*scale = {1.f, 1.f, 1.f};
	auto &skeleton = GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return false;
	auto anim = GetAnimation(animId);
	if(anim == nullptr)
		return false;
	auto frame = anim->GetFrame(frameId);
	if(frame == nullptr)
		return false;
	auto *pos = frame->GetBonePosition(boneId);
	auto *rot = frame->GetBoneOrientation(boneId);
	if(scale != nullptr) {
		auto *pScale = frame->GetBoneScale(boneId);
		if(pScale != nullptr)
			*scale = *pScale;
	}
	if(pos == nullptr)
		return false;
	rPos = *pos;
	rRot = *rot;
	auto parent = bone->parent.lock();
	while(parent != nullptr) {
		auto *posParent = frame->GetBonePosition(parent->ID);
		auto *rotParent = frame->GetBoneOrientation(parent->ID);
		uvec::rotate(&rPos, *rotParent);
		rPos += *posParent;
		rRot = *rotParent * rRot;
		parent = parent->parent.lock();
	}
	return true;
}
util::WeakHandle<const Model> Model::GetHandle() const { return util::WeakHandle<const Model>(std::static_pointer_cast<const Model>(shared_from_this())); }
util::WeakHandle<Model> Model::GetHandle() { return util::WeakHandle<Model>(std::static_pointer_cast<Model>(shared_from_this())); }

bool Model::SetReferencePoses(const std::vector<umath::ScaledTransform> &poses, bool posesInParentSpace)
{
	auto &skeleton = GetSkeleton();
	if(skeleton.GetBoneCount() != poses.size())
		return false;
	std::vector<umath::ScaledTransform> *relPoses = nullptr;
	std::vector<umath::ScaledTransform> *absPoses = nullptr;
	std::vector<umath::ScaledTransform> t;
	t.resize(poses.size());
	if(posesInParentSpace) {
		relPoses = const_cast<std::vector<umath::ScaledTransform> *>(&poses);
		absPoses = &t;
		skeleton.TransformToGlobalSpace(*relPoses, *absPoses);
	}
	else {
		absPoses = const_cast<std::vector<umath::ScaledTransform> *>(&poses);
		relPoses = &t;
		skeleton.TransformToParentSpace(*absPoses, *relPoses);
	}

	auto &ref = GetReference();
	ref.SetBoneCount(poses.size());
	for(auto i = decltype(poses.size()) {0u}; i < poses.size(); ++i)
		ref.SetBonePose(i, (*absPoses)[i]);

	auto animId = LookupAnimation("reference");
	if(LookupAnimation("reference") != -1) {
		auto anim = pragma::animation::Animation::Create();
		animId = AddAnimation("reference", anim);
	}

	auto anim = GetAnimation(animId);
	if(anim->GetFrameCount() == 0)
		anim->AddFrame(Frame::Create(skeleton.GetBoneCount()));
	auto frame = anim->GetFrame(0);
	frame->SetBoneCount(skeleton.GetBoneCount());
	anim->SetBoneList({});

	for(auto i = decltype(poses.size()) {0u}; i < poses.size(); ++i)
		frame->SetBonePose(i, (*relPoses)[i]);
	return true;
}

Frame &Model::GetReference() { return *m_reference; }
const Frame &Model::GetReference() const { return *m_reference; }
void Model::SetReference(std::shared_ptr<Frame> frame) { m_reference = frame; }
const std::vector<JointInfo> &Model::GetJoints() const { return const_cast<Model *>(this)->GetJoints(); }
std::vector<JointInfo> &Model::GetJoints() { return m_joints; }
JointInfo &Model::AddJoint(JointType type, pragma::animation::BoneId child, pragma::animation::BoneId parent)
{
	m_joints.push_back(JointInfo(type, child, parent));
	return m_joints.back();
}

const std::vector<Eyeball> &Model::GetEyeballs() const { return const_cast<Model *>(this)->GetEyeballs(); }
std::vector<Eyeball> &Model::GetEyeballs() { return m_eyeballs; }
uint32_t Model::GetEyeballCount() const { return m_eyeballs.size(); }
const Eyeball *Model::GetEyeball(uint32_t idx) const { return const_cast<Model *>(this)->GetEyeball(idx); }
Eyeball *Model::GetEyeball(uint32_t idx) { return (idx < m_eyeballs.size()) ? &m_eyeballs.at(idx) : nullptr; }
void Model::AddEyeball(const Eyeball &eyeball) { m_eyeballs.push_back(eyeball); }

void Model::Construct()
{
	m_stateFlags |= StateFlags::Valid;
	m_name = "";
	m_skeleton = std::make_unique<pragma::animation::Skeleton>();
	m_mass = 0.f;
	m_extensions = udm::Property::Create(udm::Type::Element);
	uvec::zero(&m_collisionMin);
	uvec::zero(&m_collisionMax);
	uvec::zero(&m_renderMin);
	uvec::zero(&m_renderMax);
}

bool Model::Load(Model &mdl, NetworkState &nw, const udm::AssetData &data, std::string &outErr)
{
	assert(nw.GetGameState());
	if(mdl.LoadFromAssetData(*nw.GetGameState(), data, outErr) == false)
		return false;
	return true;
}

void Model::OnMaterialLoaded()
{
	auto bAllLoaded = true;
	for(auto &hMat : m_materials) {
		if(hMat && hMat->IsLoaded() == false) {
			bAllLoaded = false;
			break;
		}
	}
	for(auto it = m_matLoadCallbacks.begin(); it != m_matLoadCallbacks.end();) {
		if(!it->IsValid())
			it = m_matLoadCallbacks.erase(it);
		else
			++it;
	}
	umath::set_flag(m_stateFlags, StateFlags::AllMaterialsLoaded, bAllLoaded);
	if(bAllLoaded == true) {
		auto onAllMatsLoadedCallbacks = std::move(m_onAllMatsLoadedCallbacks);
		m_onAllMatsLoadedCallbacks.clear();
		for(auto &hCb : onAllMatsLoadedCallbacks) {
			if(hCb.IsValid() == true)
				hCb();
		}
	}
}
CallbackHandle Model::CallOnMaterialsLoaded(const std::function<void(void)> &f)
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::AllMaterialsLoaded) == true) {
		f();
		return {};
	}
	m_onAllMatsLoadedCallbacks.push_back(FunctionCallback<>::Create(f));
	return m_onAllMatsLoadedCallbacks.back();
}
void Model::AddLoadingMaterial(Material &mat, std::optional<uint32_t> index)
{
	umath::set_flag(m_stateFlags, StateFlags::AllMaterialsLoaded, false);
	if(index.has_value())
		m_materials.at(*index) = mat.GetHandle();
	else
		m_materials.push_back(mat.GetHandle());
	auto cb = mat.CallOnLoaded([this]() { OnMaterialLoaded(); });
	if(cb.IsValid() == true)
		m_matLoadCallbacks.push_back(cb);
}
uint32_t Model::AddTexture(const std::string &tex, Material *mat)
{
	auto &meta = GetMetaInfo();
	auto ntex = pragma::asset::get_normalized_path(tex, pragma::asset::Type::Material);
	auto it = std::find(meta.textures.begin(), meta.textures.end(), ntex);
	if(it != meta.textures.end())
		return it - meta.textures.begin();
	meta.textures.push_back(ntex);
	if(mat == nullptr)
		m_materials.push_back(msys::MaterialHandle {});
	else
		AddLoadingMaterial(*mat);
	return static_cast<uint32_t>(meta.textures.size() - 1);
}
bool Model::SetTexture(uint32_t texIdx, const std::string &tex, Material *mat)
{
	auto &meta = GetMetaInfo();
	if(texIdx < meta.textures.size()) {
		auto ntex = tex;
		ufile::remove_extension_from_filename(ntex, pragma::asset::get_supported_extensions(pragma::asset::Type::Material));
		meta.textures.at(texIdx) = ntex;
	}
	if(mat == nullptr)
		m_materials.at(texIdx) = {};
	else if(texIdx < m_materials.size())
		AddLoadingMaterial(*mat, texIdx);
	else
		return false;
	return true;
}
uint32_t Model::AddMaterial(uint32_t skin, Material *mat, const std::optional<std::string> &matName, std::optional<uint32_t> *optOutSkinTexIdx)
{
	auto texName = matName.has_value() ? *matName : mat->GetName();
	texName = pragma::asset::get_normalized_path(texName, pragma::asset::Type::Material);
	AddTexturePath(ufile::get_path_from_filename(texName));
	texName = ufile::get_file_from_filename(texName);
	auto r = AddTexture(texName, mat);
	if(skin < m_textureGroups.size()) {
		auto &textures = m_textureGroups.at(skin).textures;
		auto itTexId = std::find(textures.begin(), textures.end(), r);
		if(itTexId == textures.end()) {
			textures.push_back(r);
			itTexId = textures.end() - 1;
		}
		if(optOutSkinTexIdx)
			*optOutSkinTexIdx = itTexId - textures.begin();
	}
	return r;
}
bool Model::SetMaterial(uint32_t texIdx, Material *mat)
{
	auto texName = mat->GetName();
	AddTexturePath(ufile::get_path_from_filename(texName)); // TODO: Remove previous texture path if it is not in use anymore
	texName = ufile::get_file_from_filename(texName);
	ufile::remove_extension_from_filename(texName);
	return SetTexture(texIdx, texName, mat);
}
void Model::RemoveTexture(uint32_t idx)
{
	auto &meta = GetMetaInfo();
	if(idx < meta.textures.size())
		meta.textures.erase(meta.textures.begin() + idx);
	auto &materials = GetMaterials();
	if(idx < materials.size())
		materials.erase(materials.begin() + idx);
	for(auto &group : m_textureGroups) {
		for(auto it = group.textures.begin(); it != group.textures.end();) {
			auto &texId = *it;
			if(texId < idx)
				++it;
			else if(texId == idx)
				it = group.textures.erase(it);
			else {
				--texId;
				++it;
			}
		}
	}
}
void Model::ClearTextures()
{
	auto &meta = GetMetaInfo();
	meta.textures.clear();
	m_materials.clear();
	for(auto &group : m_textureGroups)
		group.textures.clear();
}
void Model::OnMaterialMissing(const std::string &) {}

bool Model::FindMaterial(const std::string &texture, std::string &matPath) const { return FindMaterial(texture, matPath, true); }
bool Model::FindMaterial(const std::string &texture, std::string &matPath, bool importIfNotFound) const
{
	auto &meta = GetMetaInfo();
	auto &texturePaths = meta.texturePaths;
	for(auto &path : texturePaths) {
		auto texPath = path + texture;
		auto foundPath = pragma::asset::find_file(texPath, pragma::asset::Type::Material);
		if(foundPath.has_value() || FileManager::Exists("materials\\" + texPath + ".vmt") || FileManager::Exists("materials\\" + texPath + ".vmat_c")) {
			matPath = texPath;
			return true;
		}
	}
	static auto bSkipPort = false;
	if(bSkipPort == true || engine->ShouldMountExternalGameResources() == false)
		return false;
	if(importIfNotFound) {
		// Material not found; Attempt to port
		for(auto &path : texturePaths) {
			auto texPath = path + texture;
			if(m_networkState->PortMaterial(texPath) == true) {
				bSkipPort = true;
				auto r = FindMaterial(texture, matPath);
				bSkipPort = false;
				return r;
			}
		}
	}
	return false;
}

void Model::PrecacheTexture(uint32_t texId, bool bReload)
{
	// TODO: lodMaterial Parameter
	// TODO: Virtual (Clientside override)
	// TODO: Protected +public
	//PrecacheTextureGroup(std::bind(&ServerState::LoadMaterial,server,std::placeholders::_1,std::placeholders::_2),i);
	if(texId >= m_metaInfo.textures.size())
		return;
	if(texId >= m_materials.size())
		m_materials.resize(texId + 1);
	auto &texture = m_metaInfo.textures.at(texId);
	std::string matPath;
	if(FindMaterial(texture, matPath) == false) {
		OnMaterialMissing(texture);
		return;
	}
	if(!matPath.empty()) {
		auto *mat = m_networkState->LoadMaterial(matPath, bReload);
		if(mat != nullptr)
			AddLoadingMaterial(*mat, texId);
	}
}

void Model::Validate()
{
	pragma::model::validate_value(m_maxEyeDeflection);
	for(auto &[name, info] : m_phonemeMap.phonemes) {
		for(auto &[name, val] : info.flexControllers)
			pragma::model::validate_value(val);
	}
	for(auto &eb : m_eyeballs) {
		pragma::model::validate_value(eb.irisScale);
		pragma::model::validate_value(eb.forward);
		pragma::model::validate_value(eb.irisUvRadius);
		pragma::model::validate_value(eb.maxDilationFactor);
		pragma::model::validate_value(eb.origin);
		pragma::model::validate_value(eb.radius);
		pragma::model::validate_value(eb.up);
		pragma::model::validate_value(eb.zOffset);
	}
	for(auto &[id, hb] : m_hitboxes) {
		pragma::model::validate_value(hb.min);
		pragma::model::validate_value(hb.max);
	}
	for(auto &fc : m_flexControllers) {
		pragma::model::validate_value(fc.min);
		pragma::model::validate_value(fc.max);
	}
	pragma::model::validate_value(m_eyeOffset);
	pragma::model::validate_value(m_collisionMin);
	pragma::model::validate_value(m_collisionMax);
	pragma::model::validate_value(m_renderMin);
	pragma::model::validate_value(m_renderMax);
	for(auto &pose : m_bindPose)
		pragma::model::validate_value(pose);
	for(auto &lod : m_lods)
		pragma::model::validate_value(lod.distance);
	for(auto &att : m_attachments) {
		pragma::model::validate_value(att.offset);
		pragma::model::validate_value(att.angles);
	}
	for(auto &cmesh : m_collisionMeshes)
		cmesh->Validate();
	if(m_metaRig) {
		pragma::model::validate_value(m_metaRig->forwardFacingRotationOffset);
		pragma::model::validate_value(m_metaRig->min);
		pragma::model::validate_value(m_metaRig->max);
	}
	for(auto &flexAnim : m_flexAnimations) {
		pragma::model::validate_value(flexAnim->GetFps());
		for(auto &frame : flexAnim->GetFrames()) {
			for(auto &v : frame->GetValues())
				pragma::model::validate_value(v);
		}
	}
	for(auto &mg : m_meshGroups) {
		for(auto &m : mg->GetMeshes()) {
			for(auto &sm : m->GetSubMeshes())
				sm->Validate();
		}
	}
	for(auto &va : m_vertexAnimations) {
		for(auto &ma : va->GetMeshAnimations()) {
			for(auto &mf : ma->GetFrames()) {
				auto n = mf->GetVertexCount();
				for(size_t i = 0; i < n; ++i) {
					float delta;
					if(mf->GetDeltaValue(i, delta))
						pragma::model::validate_value(delta);
					Vector3 n;
					if(mf->GetVertexNormal(i, n))
						pragma::model::validate_value(n);
					Vector3 v;
					if(mf->GetVertexPosition(i, v))
						pragma::model::validate_value(v);
				}
			}
		}
	}
	m_reference->Validate();
	for(auto &anim : m_animations)
		anim->Validate();
}

void Model::Optimize()
{
	auto &meshGroups = GetMeshGroups();
	for(auto &group : meshGroups) {
		// Group all sub-meshes for this mesh group by material
		std::unordered_map<uint32_t, std::vector<std::shared_ptr<ModelSubMesh>>> groupedMeshes;
		for(auto &mesh : group->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				auto texId = subMesh->GetSkinTextureIndex();
				auto it = groupedMeshes.find(texId);
				if(it == groupedMeshes.end())
					it = groupedMeshes.insert(decltype(groupedMeshes)::value_type(texId, {})).first;
				it->second.push_back(subMesh);
			}
		}

		// Merge all sub-meshes with same material, and clear (triangles) sub-meshes which aren't needed anymore
		for(auto &pair : groupedMeshes) {
			if(pair.second.size() == 1)
				continue;
			auto &meshFirst = pair.second.front();
			for(auto it = pair.second.begin() + 1; it != pair.second.end(); ++it) {
				auto &meshOther = *it;
				meshFirst->Merge(*meshOther);
				meshOther->ClearTriangles();
			}
		}

		// Remove obsolete sub-meshes from meshes
		auto &meshes = group->GetMeshes();
		for(auto it = meshes.begin(); it != meshes.end();) {
			auto &mesh = *it;
			auto &subMeshes = mesh->GetSubMeshes();
			for(auto it = subMeshes.begin(); it != subMeshes.end();) {
				auto &subMesh = *it;
				if(subMesh->GetTriangleCount() > 0) {
					++it;
					continue;
				}
				it = subMeshes.erase(it);
			}
			if(subMeshes.empty() == true)
				it = meshes.erase(it);
			else
				++it;
		}
	}
}

void Model::PrecacheMaterials() { LoadMaterials(true, false); }

void Model::RemoveUnusedMaterialReferences()
{
	if(m_metaInfo.textures.size() != m_materials.size())
		return;
	std::vector<bool> inUse;
	inUse.resize(m_metaInfo.textures.size(), false);
	for(auto &meshGroup : GetMeshGroups()) {
		for(auto &mesh : meshGroup->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				auto idx = subMesh->GetSkinTextureIndex();
				if(idx >= inUse.size())
					continue;
				inUse[idx] = true;
			}
		}
	}

	std::vector<uint32_t> oldIndexToNewIndex {};
	oldIndexToNewIndex.resize(m_metaInfo.textures.size(), std::numeric_limits<uint32_t>::max());
	uint32_t oldIdx = 0;
	for(auto it = m_metaInfo.textures.begin(); it != m_metaInfo.textures.end();) {
		util::ScopeGuard sg {[&oldIdx]() { ++oldIdx; }};
		auto idx = it - m_metaInfo.textures.begin();
		if(inUse[oldIdx] == false) {
			it = m_metaInfo.textures.erase(it);
			m_materials.erase(m_materials.begin() + idx);
			continue;
		}
		oldIndexToNewIndex[oldIdx] = idx;
		++it;
	}

	auto &texGroups = GetTextureGroups();
	for(auto &texGroup : texGroups) {
		for(auto it = texGroup.textures.begin(); it != texGroup.textures.end();) {
			auto idx = *it;
			if(oldIndexToNewIndex[idx] == std::numeric_limits<uint32_t>::max()) {
				it = texGroup.textures.erase(it);
				continue;
			}
			*it = oldIndexToNewIndex[idx];
			++it;
		}
	}

	for(auto &meshGroup : GetMeshGroups()) {
		for(auto &mesh : meshGroup->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				auto idx = subMesh->GetSkinTextureIndex();
				if(idx >= oldIndexToNewIndex.size())
					continue;
				subMesh->SetSkinTextureIndex(oldIndexToNewIndex[idx]);
			}
		}
	}
}

void Model::LoadMaterials(const std::vector<uint32_t> &textureGroupIds, bool precache, bool bReload)
{
	util::ScopeGuard resWatcherLock {};
	if(!precache) {
		// Loading materials may require saving materials / textures, which can trigger the resource watcher,
		// so we'll disable it temporarily. This is a bit of a messy solution...
		// TODO: Remove this once the VMT/VMAT conversion code has been removed from the material system!
		resWatcherLock = std::move(engine->ScopeLockResourceWatchers());
	}

	auto &meta = GetMetaInfo();
	auto &textures = meta.textures;
	//m_materials.clear();
	if(!precache)
		m_materials.resize(textures.size());

	std::vector<std::string> materialPaths;
	materialPaths.resize(textures.size());
	auto &textureGroups = GetTextureGroups();
	for(auto i = decltype(textureGroupIds.size()) {0}; i < textureGroupIds.size(); ++i) {
		auto &group = textureGroups[textureGroupIds[i]];
		for(auto texId : group.textures) {
			auto &texture = textures[texId];
			if(FindMaterial(texture, materialPaths[texId], !precache) == false) {
				if(!precache)
					OnMaterialMissing(texture);
			}
		}
	}
	for(auto i = decltype(materialPaths.size()) {0}; i < materialPaths.size(); ++i) {
		auto &matPath = materialPaths[i];
		if(!matPath.empty()) {
			if(precache) {
				m_networkState->PrecacheMaterial(matPath);
				continue;
			}
			auto *mat = m_networkState->LoadMaterial(matPath, bReload);
			if(mat != nullptr)
				AddLoadingMaterial(*mat, i);
		}
	}
	if(!precache)
		OnMaterialLoaded();
}

void Model::LoadMaterials(bool precache, bool bReload)
{
	if(!precache)
		m_stateFlags |= StateFlags::MaterialsLoadInitiated;
	auto &meta = GetMetaInfo();
	auto bDontPrecacheTexGroups = umath::is_flag_set(meta.flags, Model::Flags::DontPrecacheTextureGroups);
	std::vector<uint32_t> groupIds;
	auto &texGroups = GetTextureGroups();
	groupIds.reserve(texGroups.size());
	for(auto i = decltype(texGroups.size()) {0}; i < ((bDontPrecacheTexGroups == true) ? 1 : texGroups.size()); ++i)
		groupIds.push_back(static_cast<uint32_t>(i));
	LoadMaterials(groupIds, precache, bReload);
}

void Model::LoadMaterials(bool bReload) { LoadMaterials(false, bReload); }
TextureGroup *Model::CreateTextureGroup()
{
	m_textureGroups.push_back(TextureGroup {});
	auto &skin = m_textureGroups.back();
	//auto &meta = GetMetaInfo();
	//for(auto i=decltype(meta.textures.size()){0};i<meta.textures.size();++i)
	//	skin.textures.push_back(static_cast<uint32_t>(i)); // Only base-textures; See FWMD::Load
	return &skin;
}
void Model::PrecacheTextureGroup(uint32_t i)
{
	if(i >= m_textureGroups.size())
		return;
	LoadMaterials(std::vector<uint32_t> {i}, true, false);
}
void Model::PrecacheTextureGroups()
{
	for(auto i = decltype(m_textureGroups.size()) {0}; i < m_textureGroups.size(); ++i)
		PrecacheTextureGroup(static_cast<uint32_t>(i));
}
std::vector<std::string> &Model::GetTexturePaths() { return m_metaInfo.texturePaths; }
void Model::AddTexturePath(const std::string &path)
{
	auto npath = path;
	npath = FileManager::GetCanonicalizedPath(npath);
	if(npath.empty() == false && npath.back() != '/' && npath.back() != '\\')
		npath += '/';
	npath = util::Path::CreatePath(npath).GetString();
	auto it = std::find_if(m_metaInfo.texturePaths.begin(), m_metaInfo.texturePaths.end(), [&npath](const std::string &pathOther) { return ustring::compare(npath, pathOther, false); });
	if(it != m_metaInfo.texturePaths.end())
		return;
	m_metaInfo.texturePaths.push_back(npath);
}
void Model::RemoveTexturePath(uint32_t idx)
{
	if(idx >= m_metaInfo.texturePaths.size())
		return;
	m_metaInfo.texturePaths.erase(m_metaInfo.texturePaths.begin() + idx);
}
void Model::SetTexturePaths(const std::vector<std::string> &paths) { m_metaInfo.texturePaths = paths; }
std::optional<uint32_t> Model::GetMaterialIndex(const ModelSubMesh &mesh, uint32_t skinId) const
{
	auto idx = mesh.GetSkinTextureIndex();
	auto *texGroup = GetTextureGroup(skinId);
	if(texGroup == nullptr && skinId > 0)
		texGroup = GetTextureGroup(0u); // Fall back to default skin
	if(texGroup == nullptr || idx >= texGroup->textures.size())
		return {};
	return texGroup->textures.at(idx);
}
std::vector<msys::MaterialHandle> &Model::GetMaterials()
{
	if(!umath::is_flag_set(m_stateFlags, StateFlags::MaterialsLoadInitiated))
		LoadMaterials();
	return m_materials;
}
const std::vector<msys::MaterialHandle> &Model::GetMaterials() const { return const_cast<Model *>(this)->GetMaterials(); }
std::vector<std::string> &Model::GetTextures() { return m_metaInfo.textures; }
std::vector<TextureGroup> &Model::GetTextureGroups() { return m_textureGroups; }
Material *Model::GetMaterial(uint32_t texID)
{
	auto &materials = GetMaterials();
	if(texID >= materials.size())
		return nullptr;
	return materials[texID].get();
}
Material *Model::GetMaterial(uint32_t texGroup, uint32_t texID)
{
	if(m_textureGroups.empty())
		return nullptr;
	if(texGroup >= m_textureGroups.size())
		texGroup = 0;
	auto &skin = m_textureGroups[texGroup];
	if(texID >= skin.textures.size()) {
		if(texGroup == 0)
			return nullptr;
		return GetMaterial(0, texID);
	}
	texID = skin.textures[texID];
	auto &materials = GetMaterials();
	if(texID >= materials.size())
		return nullptr;
	return materials[texID].get();
}
TextureGroup *Model::GetTextureGroup(uint32_t i)
{
	if(i >= m_textureGroups.size())
		return nullptr;
	return &m_textureGroups[i];
}

const TextureGroup *Model::GetTextureGroup(uint32_t i) const { return const_cast<Model *>(this)->GetTextureGroup(i); }

uint32_t Model::GetLODCount() const { return static_cast<uint32_t>(m_lods.size()); }
uint32_t Model::GetLOD(uint32_t id) const
{
	if(id >= m_lods.size())
		return 0;
	return m_lods[id].lod;
}

LODInfo *Model::AddLODInfo(uint32_t lod, float distance, std::unordered_map<uint32_t, uint32_t> &replaceIds)
{
	LODInfo *info = nullptr;
	auto itLod = std::find_if(m_lods.begin(), m_lods.end(), [lod](const LODInfo &lodInfo) { return lod == lodInfo.lod; });
	if(itLod != m_lods.end())
		info = &(*itLod);
	else {
		itLod = std::find_if(m_lods.begin(), m_lods.end(), [lod](const LODInfo &lodInfo) { return lod < lodInfo.lod; });
		itLod = m_lods.insert(itLod, LODInfo {});
		info = &(*itLod);
	}
	info->lod = lod;
	info->distance = distance;
	for(auto &pair : replaceIds)
		info->meshReplacements.insert(pair);
	if(m_lods.size() > 1) {
		auto posInserted = itLod - m_lods.begin();
		// Copy all replacement meshes from lower LODs to this one (Unless this one already replaces the same mesh)
		if(posInserted > 0) {
			auto &prev = m_lods.at(posInserted - 1);
			for(auto it = prev.meshReplacements.begin(); it != prev.meshReplacements.end(); ++it) {
				auto itCur = info->meshReplacements.find(it->first);
				if(itCur == info->meshReplacements.end())
					info->meshReplacements[it->first] = it->second;
			}
		}
		//
		// If this LOD has been placed somewhere in the middle, add all the replacement meshes to higher LODs as well
		for(auto i = (posInserted + 1); i < m_lods.size(); i++) {
			auto &next = m_lods.at(i);
			for(auto it = next.meshReplacements.begin(); it != next.meshReplacements.end(); ++it) {
				auto itCur = info->meshReplacements.find(it->first);
				if(itCur == info->meshReplacements.end())
					info->meshReplacements[it->first] = it->second;
			}
		}
		//
	}
	return info;
}

LODInfo *Model::GetLODInfo(uint32_t lod)
{
	if(m_lods.empty() || lod == 0)
		return nullptr;
	for(auto i = decltype(m_lods.size()) {0}; i < m_lods.size(); ++i) {
		auto &info = m_lods[i];
		if(lod == info.lod)
			return &info;
		else if(info.lod > lod)
			return (i == 0) ? nullptr : &m_lods[i - 1];
	}
	return &m_lods.back();
}

const std::vector<LODInfo> &Model::GetLODs() const { return m_lods; }

bool Model::TranslateLODMeshes(uint32_t lod, std::vector<uint32_t> &meshIds)
{
	auto *lodInfo = GetLODInfo(lod);
	if(lodInfo != nullptr) {
		for(auto it = lodInfo->meshReplacements.begin(); it != lodInfo->meshReplacements.end(); ++it) {
			for(auto itMesh = meshIds.begin(); itMesh != meshIds.end();) {
				if(*itMesh == it->first) {
					if(it->second == MODEL_NO_MESH) {
						itMesh = meshIds.erase(itMesh);
						break;
					}
					else {
						*itMesh = it->second;
						break;
					}
				}
				++itMesh;
			}
		}
	}
	return (lodInfo != nullptr) ? true : false;
}

const std::vector<uint32_t> &Model::GetBaseMeshes() const { return const_cast<Model *>(this)->GetBaseMeshes(); }
std::vector<uint32_t> &Model::GetBaseMeshes() { return m_baseMeshes; }
uint32_t Model::GetMeshGroupCount() const { return static_cast<uint32_t>(m_meshGroups.size()); }
uint32_t Model::GetMeshCount() const { return m_meshCount; }
uint32_t Model::GetSubMeshCount() const { return m_subMeshCount; }
uint32_t Model::GetCollisionMeshCount() const { return static_cast<uint32_t>(m_collisionMeshes.size()); }
ModelMesh *Model::GetMesh(uint32_t meshGroupIdx, uint32_t meshIdx)
{
	auto meshGroup = GetMeshGroup(meshGroupIdx);
	if(meshGroup == nullptr)
		return nullptr;
	auto &meshes = meshGroup->GetMeshes();
	return (meshIdx < meshes.size()) ? meshes[meshIdx].get() : nullptr;
}
ModelSubMesh *Model::GetSubMesh(uint32_t meshGroupIdx, uint32_t meshIdx, uint32_t subMeshIdx)
{
	auto *mesh = GetMesh(meshGroupIdx, meshIdx);
	if(mesh == nullptr)
		return nullptr;
	auto &subMesh = mesh->GetSubMeshes();
	return (subMeshIdx < subMesh.size()) ? subMesh[subMeshIdx].get() : nullptr;
}
Bool Model::GetMesh(uint32_t bodyGroupId, uint32_t groupId, uint32_t &outMeshId)
{
	auto *bodyGroup = GetBodyGroup(bodyGroupId);
	if(bodyGroup == nullptr)
		return false;
	if(groupId >= bodyGroup->meshGroups.size())
		return false;
	outMeshId = bodyGroup->meshGroups[groupId];
	return true;
}
void Model::GetMeshes(const std::vector<uint32_t> &meshIds, std::vector<std::shared_ptr<ModelMesh>> &outMeshes)
{
	auto numGroups = m_meshGroups.size();
	for(auto meshId : meshIds) {
		if(meshId < numGroups) {
			auto &group = m_meshGroups[meshId];
			auto &groupMeshes = group->GetMeshes();
			outMeshes.reserve(outMeshes.size() + groupMeshes.size());
			for(auto it = groupMeshes.begin(); it != groupMeshes.end(); ++it)
				outMeshes.push_back(*it);
		}
	}
}
void Model::GetSubMeshes(const std::vector<uint32_t> &meshIds, std::vector<std::shared_ptr<ModelSubMesh>> &outMeshes)
{
	auto numGroups = m_meshGroups.size();
	for(auto meshId : meshIds) {
		if(meshId < numGroups) {
			auto &group = m_meshGroups[meshId];
			auto &groupMeshes = group->GetMeshes();
			outMeshes.reserve(outMeshes.size() + groupMeshes.size());
			for(auto it = groupMeshes.begin(); it != groupMeshes.end(); ++it) {
				auto &mesh = *it;
				outMeshes.reserve(outMeshes.size() + mesh->GetSubMeshCount());
				for(auto &subMesh : mesh->GetSubMeshes())
					outMeshes.push_back(subMesh);
			}
		}
	}
}
void Model::SetCollisionBounds(const Vector3 &min, const Vector3 &max)
{
	m_collisionMin = min;
	m_collisionMax = max;
}
void Model::SetRenderBounds(const Vector3 &min, const Vector3 &max)
{
	m_renderMin = min;
	m_renderMax = max;
}

void Model::CalculateRenderBounds()
{
	Vector3 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector3 max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	auto numMeshGroups = m_meshGroups.size();
	if(numMeshGroups == 0) {
		min = Vector3 {0.f, 0.f, 0.f};
		max = Vector3 {0.f, 0.f, 0.f};
	}
	else {
		for(auto &group : m_meshGroups) {
			auto &meshes = group->GetMeshes();
			for(auto &mesh : meshes) {
				Vector3 meshMin;
				Vector3 meshMax;
				mesh->GetBounds(meshMin, meshMax);
				uvec::min(&min, meshMin);
				uvec::max(&max, meshMax);
			}
		}
	}
	m_renderMin = min;
	m_renderMax = max;
}

Vector3 Model::GetOrigin() const
{
	if(m_collisionMeshes.empty() || m_collisionMeshes.front() == nullptr)
		return {};
	return m_collisionMeshes.front()->GetOrigin();
}

const Vector3 &Model::GetEyeOffset() const { return m_eyeOffset; }
void Model::SetEyeOffset(const Vector3 &offset) { m_eyeOffset = offset; }

void Model::CalculateCollisionBounds()
{
	Vector3 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector3 max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	auto numMeshes = m_collisionMeshes.size();
	if(numMeshes == 0) {
		min = Vector3 {0.f, 0.f, 0.f};
		max = Vector3 {0.f, 0.f, 0.f};
	}
	else {
		for(auto &colMesh : m_collisionMeshes) {
			Vector3 colMin;
			Vector3 colMax;
			colMesh->GetAABB(&colMin, &colMax);
			uvec::min(&min, colMin);
			uvec::max(&max, colMax);
		}
	}
	m_collisionMin = min;
	m_collisionMax = max;
}

void Model::Update(ModelUpdateFlags flags)
{
	if((flags & ModelUpdateFlags::UpdateChildren) != ModelUpdateFlags::None) {
		for(auto &group : m_meshGroups) {
			auto &meshes = group->GetMeshes();
			for(auto &mesh : meshes)
				mesh->Update(flags);
		}
		for(auto &colMesh : m_collisionMeshes)
			colMesh->Update(flags);
	}
	if((flags & ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None) {
		CalculateCollisionBounds();
		CalculateRenderBounds();
	}
	if((flags & ModelUpdateFlags::UpdatePrimitiveCounts) != ModelUpdateFlags::None) {
		m_meshCount = 0;
		m_subMeshCount = 0;
		m_vertexCount = 0;
		m_triangleCount = 0;
		for(auto &group : m_meshGroups) {
			auto &meshes = group->GetMeshes();
			m_meshCount += static_cast<uint32_t>(meshes.size());
			for(auto &mesh : meshes) {
				m_subMeshCount += mesh->GetSubMeshCount();
				m_vertexCount += mesh->GetVertexCount();
				m_triangleCount += mesh->GetTriangleCount();
			}
		}
	}
}

void Model::GetCollisionBounds(Vector3 &min, Vector3 &max) const
{
	min = m_collisionMin;
	max = m_collisionMax;
}

void Model::GetRenderBounds(Vector3 &min, Vector3 &max) const
{
	min = m_renderMin;
	max = m_renderMax;
}

bool Model::IntersectAABB(Vector3 &min, Vector3 &max)
{
	if(umath::intersection::aabb_aabb(m_collisionMin, m_collisionMax, min, max) == umath::intersection::Intersect::Outside)
		return false;
	for(int i = 0; i < m_collisionMeshes.size(); i++)
		if(m_collisionMeshes[i]->IntersectAABB(&min, &max))
			return true;
	return false;
}

void Model::ClearCache() { m_models.clear(); }

const std::string &Model::GetName() const { return m_name; }

bool Model::IsValid() const { return umath::is_flag_set(m_stateFlags, StateFlags::Valid); }

void Model::AddMesh(const std::string &meshGroup, const std::shared_ptr<ModelMesh> &mesh)
{
	auto group = AddMeshGroup(meshGroup);
	group->AddMesh(mesh);
}

std::vector<std::shared_ptr<ModelMesh>> *Model::GetMeshes(const std::string &meshGroup)
{
	auto group = GetMeshGroup(meshGroup);
	if(group == nullptr)
		return nullptr;
	return &group->GetMeshes();
}

std::vector<std::shared_ptr<ModelMeshGroup>> &Model::GetMeshGroups() { return m_meshGroups; }
const std::vector<std::shared_ptr<ModelMeshGroup>> &Model::GetMeshGroups() const { return m_meshGroups; }
std::shared_ptr<ModelMeshGroup> Model::GetMeshGroup(const std::string &meshGroup)
{
	auto it = std::find_if(m_meshGroups.begin(), m_meshGroups.end(), [&meshGroup](std::shared_ptr<ModelMeshGroup> &group) { return ustring::compare(group->GetName(), meshGroup, false); });
	if(it == m_meshGroups.end())
		return nullptr;
	return *it;
}
std::shared_ptr<ModelMeshGroup> Model::GetMeshGroup(uint32_t groupId)
{
	if(groupId >= m_meshGroups.size())
		return nullptr;
	return m_meshGroups[groupId];
}

bool Model::GetMeshGroupId(const std::string &meshGroup, uint32_t &groupId) const
{
	auto it = std::find_if(m_meshGroups.begin(), m_meshGroups.end(), [&meshGroup](const std::shared_ptr<ModelMeshGroup> &group) { return ustring::compare(group->GetName(), meshGroup, false); });
	if(it == m_meshGroups.end())
		return false;
	groupId = it - m_meshGroups.begin();
	return true;
}

std::shared_ptr<ModelMeshGroup> Model::AddMeshGroup(const std::string &meshGroup, uint32_t &meshGroupId)
{
	auto it = std::find_if(m_meshGroups.begin(), m_meshGroups.end(), [&meshGroup](const std::shared_ptr<ModelMeshGroup> &group) { return ustring::compare(group->GetName(), meshGroup, false); });
	if(it == m_meshGroups.end()) {
		auto mg = ModelMeshGroup::Create(meshGroup);
		AddMeshGroup(mg);
		it = m_meshGroups.end() - 1;
	}
	meshGroupId = it - m_meshGroups.begin();
	return *it;
}

std::shared_ptr<ModelMeshGroup> Model::AddMeshGroup(const std::string &meshGroup)
{
	uint32_t meshGroupId = 0u;
	return AddMeshGroup(meshGroup, meshGroupId);
}
void Model::AddMeshGroup(std::shared_ptr<ModelMeshGroup> &meshGroup) { m_meshGroups.push_back(meshGroup); }

std::vector<std::shared_ptr<CollisionMesh>> &Model::GetCollisionMeshes() { return m_collisionMeshes; }
const std::vector<std::shared_ptr<CollisionMesh>> &Model::GetCollisionMeshes() const { return m_collisionMeshes; }

std::optional<uint32_t> Model::AssignDistinctMaterial(const ModelMeshGroup &group, const ModelMesh &mesh, ModelSubMesh &subMesh)
{
	auto &meshes = const_cast<ModelMeshGroup &>(group).GetMeshes();
	auto itMesh = std::find_if(meshes.begin(), meshes.end(), [&mesh](const std::shared_ptr<ModelMesh> &meshOther) { return meshOther.get() == &mesh; });
	if(itMesh == meshes.end())
		return {};
	auto &subMeshes = const_cast<ModelMesh &>(mesh).GetSubMeshes();
	auto itSubMesh = std::find_if(subMeshes.begin(), subMeshes.end(), [&subMesh](const std::shared_ptr<ModelSubMesh> &subMeshOther) { return subMeshOther.get() == &subMesh; });
	if(itSubMesh == subMeshes.end())
		return {};
	auto meshIdx = itMesh - meshes.begin();
	auto subMeshIdx = itSubMesh - subMeshes.begin();

	auto matIdx = GetMaterialIndex(subMesh);
	if(matIdx.has_value() == false)
		return {};
	auto &materials = GetMaterials();
	auto hMat = materials.at(*matIdx);
	if(!hMat)
		return {};
	auto strPath = hMat->GetAbsolutePath();
	if(strPath.has_value() == false)
		return {};
	util::Path path {*strPath};
	auto ext = path.GetFileExtension();
	path.RemoveFileExtension();
	path += '_' + group.GetName();
	path += '_' + std::to_string(meshIdx);
	path += '_' + std::to_string(subMeshIdx);

	util::Path rootPath {};
	while(path.GetFront() != "materials") // TODO: What if inside addon called "materials"?
	{
		auto front = path.GetFront();
		rootPath += util::Path::CreatePath(std::string {front});
		path.PopFront();
	}
	auto mpath = path.GetString();
	if(ext.has_value())
		mpath += '.' + *ext;
	path.PopFront();

	if(FileManager::Exists(mpath) == false) {
		auto savePath = pragma::asset::relative_path_to_absolute_path(path, pragma::asset::Type::Material, rootPath.GetString());
		std::string err;
		if(hMat->Save(savePath.GetString(), err) == false)
			return {};
	}
	auto *matNew = m_networkState->LoadMaterial(path.GetString());
	if(matNew == nullptr)
		return {};
	auto numSkins = GetTextureGroups().size();
	std::optional<uint32_t> newSkinTexId {};
	for(auto i = decltype(numSkins) {0u}; i < numSkins; ++i) {
		if(i == 0u)
			AddMaterial(i, matNew, {}, &newSkinTexId);
		else
			AddMaterial(i, matNew);
	}
	if(newSkinTexId.has_value())
		subMesh.SetSkinTextureIndex(*newSkinTexId);
	return newSkinTexId;
}

void Model::AddCollisionMesh(const std::shared_ptr<CollisionMesh> &mesh) { m_collisionMeshes.push_back(mesh); }

std::optional<float> Model::CalcFlexWeight(uint32_t flexId, const std::function<std::optional<float>(uint32_t)> &fFetchFlexControllerWeight, const std::function<std::optional<float>(uint32_t)> &fFetchFlexWeight) const
{
	auto val = fFetchFlexWeight(flexId);
	if(val.has_value())
		return val;
	auto *flex = GetFlex(flexId);
	if(flex == nullptr)
		return {};
	auto &ops = flex->GetOperations();
	std::stack<float> opStack {};
	for(auto &op : ops) {
		switch(op.type) {
		case Flex::Operation::Type::None:
			break;
		case Flex::Operation::Type::Const:
			opStack.push(op.d.value);
			break;
		case Flex::Operation::Type::Fetch:
			{
				auto val = fFetchFlexControllerWeight(op.d.index);
				if(val.has_value() == false)
					return {};
				opStack.push(*val);
				break;
			}
		case Flex::Operation::Type::Fetch2:
			{
				auto val = CalcFlexWeight(op.d.index, fFetchFlexControllerWeight, fFetchFlexWeight);
				if(val.has_value() == false)
					return {};
				opStack.push(*val);
				break;
			}
		case Flex::Operation::Type::Add:
			{
				if(opStack.size() < 2)
					return {};
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l += r;
				break;
			}
		case Flex::Operation::Type::Sub:
			{
				if(opStack.size() < 2)
					return {};
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l -= r;
				break;
			}
		case Flex::Operation::Type::Mul:
			{
				if(opStack.size() < 2)
					return {};
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l *= r;
				break;
			}
		case Flex::Operation::Type::Div:
			{
				if(opStack.size() < 2)
					return {};
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				if(r != 0.f)
					l /= r;
				break;
			}
		case Flex::Operation::Type::Neg:
			{
				if(opStack.size() < 1)
					return {};
				auto &r = opStack.top();
				r = -r;
				break;
			}
		case Flex::Operation::Type::Exp:
			break;
		case Flex::Operation::Type::Open:
			break;
		case Flex::Operation::Type::Close:
			break;
		case Flex::Operation::Type::Comma:
			break;
		case Flex::Operation::Type::Max:
			{
				if(opStack.size() < 2)
					return {};
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l = umath::max(l, r);
				break;
			}
		case Flex::Operation::Type::Min:
			{
				if(opStack.size() < 2)
					return false;
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l = umath::min(l, r);
				break;
			}
		case Flex::Operation::Type::TwoWay0:
			{
				auto val = fFetchFlexControllerWeight(op.d.index);
				if(val.has_value() == false)
					return {};
				opStack.push(1.f - (umath::min(umath::max(*val + 1.f, 0.f), 1.f)));
				break;
			}
		case Flex::Operation::Type::TwoWay1:
			{
				auto val = fFetchFlexControllerWeight(op.d.index);
				if(val.has_value() == false)
					return {};
				opStack.push(umath::min(umath::max(*val, 0.f), 1.f));
				break;
			}
		case Flex::Operation::Type::NWay:
			{
				auto v = fFetchFlexControllerWeight(op.d.index);
				if(v.has_value() == false)
					return {};
				auto valueControllerIndex = op.d.index;
				opStack.pop();

				auto flValue = fFetchFlexControllerWeight(valueControllerIndex);
				if(flValue.has_value() == false)
					return {};

				auto filterRampW = opStack.top();
				opStack.pop();
				auto filterRampZ = opStack.top();
				opStack.pop();
				auto filterRampY = opStack.top();
				opStack.pop();
				auto filterRampX = opStack.top();
				opStack.pop();

				auto greaterThanX = umath::min(1.f, (-umath::min(0.f, (filterRampX - *flValue))));
				auto lessThanY = umath::min(1.f, (-umath::min(0.f, (*flValue - filterRampY))));
				auto remapX = umath::min(umath::max((*flValue - filterRampX) / (filterRampY - filterRampX), 0.f), 1.f);
				auto greaterThanEqualY = -(umath::min(1.f, (-umath::min(0.f, (*flValue - filterRampY)))) - 1.f);
				auto lessThanEqualZ = -(umath::min(1.f, (-umath::min(0.f, (filterRampZ - *flValue)))) - 1.f);
				auto greaterThanZ = umath::min(1.f, (-umath::min(0.f, (filterRampZ - *flValue))));
				auto lessThanW = umath::min(1.f, (-umath::min(0.f, (*flValue - filterRampW))));
				auto remapZ = (1.f - (umath::min(umath::max((*flValue - filterRampZ) / (filterRampW - filterRampZ), 0.f), 1.f)));

				auto expValue = ((greaterThanX * lessThanY) * remapX) + (greaterThanEqualY * lessThanEqualZ) + ((greaterThanZ * lessThanW) * remapZ);

				opStack.push(expValue * *v);
				break;
			}
		case Flex::Operation::Type::Combo:
			{
				if(opStack.size() < op.d.index)
					return {};
				auto v = (op.d.index > 0) ? 1.f : 0.f;
				;
				for(auto i = decltype(op.d.index) {0}; i < op.d.index; ++i) {
					v *= opStack.top();
					opStack.pop();
				}
				opStack.push(v);
				break;
			}
		case Flex::Operation::Type::Dominate:
			{
				if(opStack.size() < op.d.index + 1)
					return {};
				auto v = (op.d.index > 0) ? 1.f : 0.f;
				for(auto i = decltype(op.d.index) {0}; i < op.d.index; ++i) {
					v *= opStack.top();
					opStack.pop();
				}
				opStack.top() *= 1.f - v;
				break;
			}
		case Flex::Operation::Type::DMELowerEyelid:
			{
				auto pCloseLidV = fFetchFlexControllerWeight(op.d.index);
				if(pCloseLidV.has_value() == false)
					return {};
				auto &pCloseLidVController = *GetFlexController(op.d.index);
				auto flCloseLidV = (umath::min(umath::max((*pCloseLidV - pCloseLidVController.min) / (pCloseLidVController.max - pCloseLidVController.min), 0.f), 1.f));

				auto closeLidIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();

				auto pCloseLid = fFetchFlexControllerWeight(closeLidIndex);
				if(pCloseLid.has_value() == false)
					return {};
				auto &pCloseLidController = *GetFlexController(closeLidIndex);
				auto flCloseLid = (umath::min(umath::max((*pCloseLid - pCloseLidController.min) / (pCloseLidController.max - pCloseLidController.min), 0.f), 1.f));

				opStack.pop();

				auto eyeUpDownIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();
				auto pEyeUpDown = fFetchFlexControllerWeight(eyeUpDownIndex);
				if(pEyeUpDown.has_value() == false)
					return {};
				auto &pEyeUpDownController = *GetFlexController(eyeUpDownIndex);
				auto flEyeUpDown = (-1.f + 2.f * (umath::min(umath::max((*pEyeUpDown - pEyeUpDownController.min) / (pEyeUpDownController.max - pEyeUpDownController.min), 0.f), 1.f)));

				opStack.push(umath::min(1.f, (1.f - flEyeUpDown)) * (1 - flCloseLidV) * flCloseLid);
				break;
			}
		case Flex::Operation::Type::DMEUpperEyelid:
			{
				auto pCloseLidV = fFetchFlexControllerWeight(op.d.index);
				if(pCloseLidV.has_value() == false)
					return {};
				auto &pCloseLidVController = *GetFlexController(op.d.index);
				auto flCloseLidV = (umath::min(umath::max((*pCloseLidV - pCloseLidVController.min) / (pCloseLidVController.max - pCloseLidVController.min), 0.f), 1.f));

				auto closeLidIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();

				auto pCloseLid = fFetchFlexControllerWeight(closeLidIndex);
				if(pCloseLid.has_value() == false)
					return {};
				auto &pCloseLidController = *GetFlexController(closeLidIndex);
				auto flCloseLid = (umath::min(umath::max((*pCloseLid - pCloseLidController.min) / (pCloseLidController.max - pCloseLidController.min), 0.f), 1.f));

				opStack.pop();

				auto eyeUpDownIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();
				auto pEyeUpDown = fFetchFlexControllerWeight(eyeUpDownIndex);
				if(pEyeUpDown.has_value() == false)
					return {};
				auto &pEyeUpDownController = *GetFlexController(eyeUpDownIndex);
				auto flEyeUpDown = (-1.f + 2.f * (umath::min(umath::max((*pEyeUpDown - pEyeUpDownController.min) / (pEyeUpDownController.max - pEyeUpDownController.min), 0.f), 1.f)));

				opStack.push(umath::min(1.f, (1.f + flEyeUpDown)) * flCloseLidV * flCloseLid);
				break;
			}
		}
	}
	if(opStack.size() != 1) // If we don't have a single result left on the stack something went wrong
		return {};
	return opStack.top();
}

uint32_t Model::AddAnimation(const std::string &name, const std::shared_ptr<pragma::animation::Animation> &anim)
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = m_animationIDs.find(lname);
	if(it != m_animationIDs.end()) {
		m_animations.at(it->second) = anim;
		return it->second;
	}
	m_animations.push_back(anim);
	return m_animationIDs[name] = static_cast<uint32_t>(m_animations.size() - 1);
}

int32_t Model::LookupAnimation(const std::string &name) const
{
	auto it = m_animationIDs.find(name);
	if(it == m_animationIDs.end())
		return -1;
	return it->second;
}

void Model::GetAnimations(std::unordered_map<std::string, uint32_t> **anims) { *anims = &m_animationIDs; }

const pragma::animation::Skeleton &Model::GetSkeleton() const { return *m_skeleton; }
pragma::animation::Skeleton &Model::GetSkeleton() { return *m_skeleton; }

const std::shared_ptr<pragma::animation::MetaRig> &Model::GetMetaRig() const { return m_metaRig; }
void Model::ClearMetaRig() { m_metaRig = nullptr; }
void Model::GenerateStandardMetaRigReferenceBonePoses(const pragma::animation::MetaRig &metaRig, const pragma::animation::Skeleton &skeleton, const Frame &refFrame, std::vector<umath::ScaledTransform> &outPoses)
{
	std::vector<umath::ScaledTransform> relPoses;
	auto relRef = Frame::Create(refFrame);
	relRef->Localize(skeleton);

	std::unordered_map<pragma::animation::BoneId, pragma::animation::MetaRigBoneType> boneIdToMetaId;
	for(size_t i = 0; i < metaRig.bones.size(); ++i) {
		auto &metaBone = metaRig.bones[i];
		if(metaBone.boneId == pragma::animation::INVALID_BONE_INDEX)
			continue;
		boneIdToMetaId[metaBone.boneId] = static_cast<pragma::animation::MetaRigBoneType>(i);
	}

	auto numBones = skeleton.GetBoneCount();
	outPoses.resize(numBones);
	std::function<void(const pragma::animation::Bone &, const umath::ScaledTransform &)> generateStandardBonePose = nullptr;
	generateStandardBonePose = [&metaRig, &generateStandardBonePose, &relRef, &outPoses, &boneIdToMetaId](const pragma::animation::Bone &bone, const umath::ScaledTransform &parentPose) {
		umath::ScaledTransform relRefPose;
		relRef->GetBonePose(bone.ID, relRefPose);
		auto pose = parentPose * relRefPose;

		auto it = boneIdToMetaId.find(bone.ID);
		if(it != boneIdToMetaId.end()) {
			auto metaBoneId = it->second;
			auto &metaBone = *metaRig.GetBone(metaBoneId);
			auto refRot = pragma::animation::get_meta_rig_reference_rotation(metaBoneId);
			if(refRot)
				pose.SetRotation(*refRot * uquat::get_inverse(metaBone.normalizedRotationOffset));
		}
		outPoses[bone.ID] = pose;

		for(auto &[childId, child] : bone.children)
			generateStandardBonePose(*child, pose);
	};
	for(auto &[boneId, bone] : skeleton.GetRootBones())
		generateStandardBonePose(*bone, {});

	auto invBaseRot = uquat::get_inverse(metaRig.forwardFacingRotationOffset);
	for(auto &pose : outPoses)
		pose.RotateGlobal(invBaseRot);
}
bool Model::GenerateStandardMetaRigReferenceBonePoses(std::vector<umath::ScaledTransform> &outPoses) const
{
	if(!m_metaRig)
		return false;
	GenerateStandardMetaRigReferenceBonePoses(*m_metaRig, *m_skeleton, *m_reference, outPoses);
	return true;
}
std::optional<pragma::animation::MetaRigBoneType> Model::GetMetaRigBoneParentId(pragma::animation::MetaRigBoneType type) const
{
	if(!m_metaRig)
		return {};
	auto parentId = pragma::animation::get_meta_rig_bone_parent_type(type);
	if(parentId == pragma::animation::MetaRigBoneType::Spine3) {
		for(auto candidate : {pragma::animation::MetaRigBoneType::Spine3, pragma::animation::MetaRigBoneType::Spine2, pragma::animation::MetaRigBoneType::Spine1, pragma::animation::MetaRigBoneType::Spine}) {
			auto *bone = m_metaRig->GetBone(candidate);
			if(!bone)
				continue;
			parentId = candidate;
			break;
		}
	}
	if(!parentId || !m_metaRig->GetBone(*parentId))
		return {};
	return parentId;
}
std::optional<umath::ScaledTransform> Model::GetMetaRigReferencePose(pragma::animation::MetaRigBoneType type) const
{
	auto &metaRig = GetMetaRig();
	if(!metaRig)
		return {};
	auto &ref = GetReference();
	auto *bone = metaRig->GetBone(type);
	if(!bone || bone->boneId == pragma::animation::INVALID_BONE_INDEX)
		return {};
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(bone->boneId, pose))
		return {};
	pose.SetRotation(pose.GetRotation() * bone->normalizedRotationOffset);
	return pose;
}
bool Model::GenerateMetaRig()
{
	if(umath::is_flag_set(m_metaInfo.flags, Flags::GeneratedMetaRig))
		return false;
	umath::set_flag(m_metaInfo.flags, Flags::GeneratedMetaRig);
	auto libRig = m_networkState->InitializeLibrary("pr_rig");
	if(!libRig)
		return false;
	auto *generateMetaRig = libRig->FindSymbolAddress<bool (*)(const Model &, pragma::animation::MetaRig &)>("generate_meta_rig");
	if(!generateMetaRig)
		return false;
	auto metaRig = std::make_shared<pragma::animation::MetaRig>();
	if(!generateMetaRig(*this, *metaRig))
		return false;
	m_metaRig = std::move(metaRig);
	GenerateMetaBlendShapes();
	return true;
}
bool Model::GenerateMetaBlendShapes()
{
	if(umath::is_flag_set(m_metaInfo.flags, Flags::GeneratedMetaBlendShapes))
		return false;
	umath::set_flag(m_metaInfo.flags, Flags::GeneratedMetaBlendShapes);
	auto libRig = m_networkState->InitializeLibrary("pr_rig");
	if(!libRig)
		return false;
	auto *generateMetaBlendShapes = libRig->FindSymbolAddress<bool (*)(Model &)>("generate_meta_blend_shapes");
	if(!generateMetaBlendShapes)
		return false;
	if(!generateMetaBlendShapes(*this))
		return false;
	return true;
}
void Model::ApplyPostImportProcessing()
{
	if(GenerateMetaRig()) {
		// Probably a character model
		if(m_hitboxes.empty())
			GenerateHitboxes();
		// Low-level LODs are used for fast BVH intersections
		GenerateLowLevelLODs(*m_networkState->GetGameState());
	}
}

std::shared_ptr<pragma::animation::Animation> Model::GetAnimation(uint32_t ID) const
{
	if(ID >= m_animations.size())
		return nullptr;
	return m_animations[ID];
}
uint32_t Model::GetAnimationCount() const { return static_cast<uint32_t>(m_animations.size()); }
std::shared_ptr<ModelMesh> Model::CreateMesh() const { return std::make_shared<ModelMesh>(); }
std::shared_ptr<ModelSubMesh> Model::CreateSubMesh() const { return std::make_shared<ModelSubMesh>(); }
void Model::RemoveBone(pragma::animation::BoneId boneId)
{
	auto &bones = m_skeleton->GetBones();
	if(boneId >= bones.size())
		return;
	auto &bone = bones[boneId];
	for(auto &[boneId, child] : bone->children)
		child->parent = bone->parent;
	auto parent = bone->parent.lock();
	if(parent) {
		for(auto &[boneId, child] : bone->children)
			parent->children[boneId] = child;
	}
	else {
		auto &rootBones = m_skeleton->GetRootBones();
		auto it = rootBones.find(boneId);
		if(it != rootBones.end())
			rootBones.erase(it);
		for(auto &[boneId, child] : bone->children)
			rootBones[boneId] = child;
	}

	bones.erase(bones.begin() + boneId);

	auto removeFromFrame = [](Frame &frame, pragma::animation::BoneId boneId) {
		auto &poses = frame.GetBoneTransforms();
		if(boneId < poses.size())
			poses.erase(poses.begin() + boneId);

		auto &scales = frame.GetBoneScales();
		if(boneId < scales.size())
			scales.erase(scales.begin() + boneId);
	};
	removeFromFrame(*m_reference, boneId);
	for(auto &anim : GetAnimations()) {
		auto &boneList = const_cast<std::vector<uint16_t> &>(anim->GetBoneList());
		if(boneList.empty()) {
			for(auto &frame : anim->GetFrames())
				removeFromFrame(*frame, boneId);
			continue;
		}
		auto &boneMap = const_cast<std::unordered_map<uint32_t, uint32_t> &>(anim->GetBoneMap());
		auto it = boneMap.find(boneId);
		if(it == boneMap.end())
			continue;
		auto localBoneId = it->second;
		for(auto &frame : anim->GetFrames())
			removeFromFrame(*frame, localBoneId);
		boneList.erase(boneList.begin() + localBoneId);
		boneMap.erase(it);

		for(auto &id : boneList) {
			if(id >= boneId)
				--id;
		}

		std::unordered_map<uint32_t, uint32_t> newBoneMap;
		for(auto &pair : boneMap) {
			auto globalId = pair.first;
			auto localId = pair.second;
			if(boneId >= globalId)
				--globalId;
			if(localId >= localBoneId)
				--localId;
			newBoneMap[globalId] = localId;
		}
		boneMap = newBoneMap;
	}

	std::unordered_map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> newRootBones;
	for(auto &[boneId, bone] : m_skeleton->GetRootBones()) {
		auto newBoneId = boneId;
		if(boneId >= newBoneId)
			--newBoneId;
		newRootBones[newBoneId] = bone;
	}
	m_skeleton->GetRootBones() = newRootBones;

	for(auto &bone : m_skeleton->GetBones()) {
		std::unordered_map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> newChildren;
		for(auto &[boneId, bone] : bone->children) {
			auto newBoneId = boneId;
			if(boneId >= newBoneId)
				--newBoneId;
			newChildren[newBoneId] = bone;
		}
		bone->children = newChildren;
	}
}
float Model::CalcBoneLength(pragma::animation::BoneId boneId) const
{
	umath::ScaledTransform pose;
	m_reference->GetBonePose(boneId, pose);

	auto bone = m_skeleton->GetBone(boneId).lock();
	if(!bone)
		return 0.f;
	auto n = bone->children.size();
	if(n == 0) {
		if(!bone->parent.expired())
			return CalcBoneLength(bone->parent.lock()->ID); // Assume same length as parent
		return 0.f;
	}
	Vector3 avgChildPos {};
	for(auto &[childId, child] : bone->children) {
		umath::ScaledTransform childPose;
		m_reference->GetBonePose(childId, childPose);
		avgChildPos += childPose.GetOrigin();
	}
	avgChildPos /= static_cast<float>(n);
	return uvec::distance(pose.GetOrigin(), avgChildPos);
}
bool Model::HasVertexWeights() const
{
	for(auto &meshGroup : GetMeshGroups()) {
		for(auto &mesh : meshGroup->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				if(subMesh->GetVertexWeights().empty() == false)
					return true;
			}
		}
	}
	return false;
}
const std::vector<std::shared_ptr<pragma::animation::Animation>> &Model::GetAnimations() const { return const_cast<Model *>(this)->GetAnimations(); }
std::vector<std::shared_ptr<pragma::animation::Animation>> &Model::GetAnimations() { return m_animations; }
bool Model::GetAnimationName(uint32_t animId, std::string &name) const
{
	auto it = std::find_if(m_animationIDs.begin(), m_animationIDs.end(), [animId](const std::pair<std::string, uint32_t> &pair) { return (pair.second == animId) ? true : false; });
	if(it == m_animationIDs.end()) {
		name = "";
		return false;
	}
	name = it->first;
	return true;
}
std::string Model::GetAnimationName(uint32_t animId) const
{
	std::string r;
	GetAnimationName(animId, r);
	return r;
}

const std::vector<Attachment> &Model::GetAttachments() const { return const_cast<Model *>(this)->GetAttachments(); }
std::vector<Attachment> &Model::GetAttachments() { return m_attachments; }

void Model::AddAttachment(const std::string &name, uint32_t boneID, Vector3 offset, EulerAngles angles)
{
	auto lname = name;
	ustring::to_lower(lname);
	m_attachments.push_back(Attachment {});
	auto &att = m_attachments.back();
	att.name = lname;
	att.bone = boneID;
	att.offset = offset;
	att.angles = angles;
}
Attachment *Model::GetAttachment(uint32_t attachmentID)
{
	if(attachmentID >= m_attachments.size())
		return nullptr;
	return &m_attachments[attachmentID];
}
Attachment *Model::GetAttachment(const std::string &name)
{
	auto id = LookupAttachment(name);
	return GetAttachment(id);
}
void Model::RemoveAttachment(const std::string &name)
{
	auto attId = LookupAttachment(name);
	if(attId < 0)
		return;
	RemoveAttachment(attId);
}
void Model::RemoveAttachment(uint32_t idx)
{
	if(idx >= m_attachments.size())
		return;
	m_attachments.erase(m_attachments.begin() + idx);
}
int32_t Model::LookupBone(const std::string &name) const
{
	auto &skeleton = GetSkeleton();
	return skeleton.LookupBone(name);
}
std::optional<uint32_t> Model::LookupFlexAnimation(const std::string &name) const
{
	auto it = std::find(m_flexAnimationNames.begin(), m_flexAnimationNames.end(), name);
	if(it == m_flexAnimationNames.end())
		return {};
	return it - m_flexAnimationNames.begin();
}
uint32_t Model::AddFlexAnimation(const std::string &name, FlexAnimation &anim)
{
	auto id = LookupFlexAnimation(name);
	if(id.has_value()) {
		m_flexAnimations[*id] = anim.shared_from_this();
		return *id;
	}
	m_flexAnimationNames.push_back(name);
	m_flexAnimations.push_back(anim.shared_from_this());
	return m_flexAnimationNames.size() - 1;
}
FlexAnimation *Model::GetFlexAnimation(uint32_t idx) { return (idx < m_flexAnimations.size()) ? m_flexAnimations[idx].get() : nullptr; }
const std::string *Model::GetFlexAnimationName(uint32_t idx) const { return (idx < m_flexAnimationNames.size()) ? &m_flexAnimationNames[idx] : nullptr; }
int32_t Model::LookupAttachment(const std::string &name)
{
	auto lname = name;
	ustring::to_lower(lname);
	for(auto i = decltype(m_attachments.size()) {0}; i < m_attachments.size(); ++i) {
		auto &att = m_attachments[i];
		if(att.name == lname)
			return static_cast<int32_t>(i);
	}
	return -1;
}

std::optional<umath::ScaledTransform> Model::CalcReferenceAttachmentPose(int32_t attId) const
{
	auto *att = const_cast<Model *>(this)->GetAttachment(attId);
	if(att == nullptr)
		return {};
	umath::ScaledTransform t {att->offset, uquat::create(att->angles)};
	auto &reference = GetReference();
	auto *bonePos = reference.GetBonePosition(att->bone);
	auto *boneRot = reference.GetBoneOrientation(att->bone);
	auto *boneScale = reference.GetBoneScale(att->bone);
	t = umath::ScaledTransform {bonePos ? *bonePos : Vector3 {}, boneRot ? *boneRot : uquat::identity(), boneScale ? *boneScale : Vector3 {1.f, 1.f, 1.f}} * t;
	return t;
}

std::optional<umath::ScaledTransform> Model::CalcReferenceBonePose(int32_t boneId) const
{
	auto &reference = GetReference();
	auto *bonePos = reference.GetBonePosition(boneId);
	auto *boneRot = reference.GetBoneOrientation(boneId);
	auto *boneScale = reference.GetBoneScale(boneId);
	return umath::ScaledTransform {bonePos ? *bonePos : Vector3 {}, boneRot ? *boneRot : uquat::identity(), boneScale ? *boneScale : Vector3 {1.f, 1.f, 1.f}};
}

uint32_t Model::GetBoneCount() const { return m_skeleton->GetBoneCount(); }

Mat4 *Model::GetBindPoseBoneMatrix(uint32_t boneID)
{
	if(boneID >= m_bindPose.size())
		return nullptr;
	return &m_bindPose[boneID];
}

void Model::SetBindPoseBoneMatrix(uint32_t boneID, Mat4 mat)
{
	if(boneID >= m_bindPose.size()) {
		auto numBones = m_skeleton->GetBoneCount();
		if(numBones > m_bindPose.size() && boneID < numBones)
			m_bindPose.resize(numBones);
		else
			return;
	}
	m_bindPose[boneID] = mat;
}

void Model::SetMass(float mass) { m_mass = mass; }
float Model::GetMass() const { return m_mass; }
uint8_t Model::GetAnimationActivityWeight(uint32_t animation) const
{
	if(animation >= m_animations.size())
		return 0;
	return m_animations[animation]->GetActivityWeight();
}
Activity Model::GetAnimationActivity(uint32_t animation) const
{
	if(animation >= m_animations.size())
		return Activity::Invalid;
	return m_animations[animation]->GetActivity();
}
float Model::GetAnimationDuration(uint32_t animation)
{
	if(animation >= m_animations.size())
		return 0.f;
	return m_animations[animation]->GetDuration();
}
int Model::SelectFirstAnimation(Activity activity) const
{
	auto it = std::find_if(m_animations.begin(), m_animations.end(), [activity](const std::shared_ptr<pragma::animation::Animation> &anim) { return anim->GetActivity() == activity; });
	if(it == m_animations.end())
		return -1;
	return it - m_animations.begin();
}
int32_t Model::SelectWeightedAnimation(Activity activity, int32_t animIgnore)
{
	std::vector<int32_t> animations;
	std::vector<uint8_t> weights;
	uint32_t weightSum = 0;
	auto bIgnoreIsCandidate = false;
	for(auto i = decltype(m_animations.size()) {0}; i < m_animations.size(); ++i) {
		auto &anim = m_animations[i];
		if(i != animIgnore || !anim->HasFlag(FAnim::NoRepeat)) {
			if(anim->GetActivity() == activity) {
				weightSum += anim->GetActivityWeight();
				animations.push_back(static_cast<int32_t>(i));
				weights.push_back(anim->GetActivityWeight());
			}
		}
		else
			bIgnoreIsCandidate = true;
	}
	if(animations.empty()) {
		if(bIgnoreIsCandidate == true)
			return animIgnore;
		return -1;
	}
	if(animations.size() == 1 || weightSum == 0)
		return animations.front();
	int r = umath::random(0, weightSum - 1);
	for(auto animId : animations) {
		auto &anim = m_animations[animId];
		auto weight = anim->GetActivityWeight();
		if(r < weight)
			return animId;
		r -= weight;
	}
	return -1;
}
void Model::GetAnimations(Activity activity, std::vector<uint32_t> &animations)
{
	for(auto i = decltype(m_animations.size()) {0}; i < m_animations.size(); ++i) {
		auto &anim = m_animations[i];
		if(anim->GetActivity() == activity)
			animations.push_back(static_cast<int32_t>(i));
	}
}
void Model::AddBlendController(const std::string &name, int32_t min, int32_t max, bool loop)
{
	auto lname = name;
	ustring::to_lower(lname);
	m_blendControllers.push_back(BlendController {});
	auto &blend = m_blendControllers.back();
	blend.name = name;
	blend.min = min;
	blend.max = max;
	blend.loop = loop;
}
const BlendController *Model::GetBlendController(uint32_t id) const { return const_cast<Model *>(this)->GetBlendController(id); }
BlendController *Model::GetBlendController(uint32_t id)
{
	if(id >= m_blendControllers.size())
		return nullptr;
	return &m_blendControllers[id];
}
const BlendController *Model::GetBlendController(const std::string &name) const { return const_cast<Model *>(this)->GetBlendController(name); }
BlendController *Model::GetBlendController(const std::string &name)
{
	auto it = std::find_if(m_blendControllers.begin(), m_blendControllers.end(), [&name](BlendController &bc) { return (bc.name == name) ? true : false; });
	if(it == m_blendControllers.end())
		return nullptr;
	return &(*it);
}
int32_t Model::LookupBlendController(const std::string &name)
{
	for(auto i = decltype(m_blendControllers.size()) {0}; i < m_blendControllers.size(); ++i) {
		if(m_blendControllers[i].name == name)
			return static_cast<int32_t>(i);
	}
	return -1;
}
const std::vector<BlendController> &Model::GetBlendControllers() const { return const_cast<Model *>(this)->GetBlendControllers(); }
std::vector<BlendController> &Model::GetBlendControllers() { return m_blendControllers; }

void Model::UpdateShape(const std::vector<SurfaceMaterial> *)
{
	for(auto &cmesh : m_collisionMeshes)
		cmesh->UpdateShape();
}
std::optional<umath::ScaledTransform> Model::GetReferenceBonePose(pragma::animation::BoneId boneId) const
{
	auto &ref = GetReference();
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId, pose))
		return {};
	return pose;
}
std::optional<pragma::SignedAxis> Model::FindBoneAxisForDirection(pragma::animation::BoneId boneId, const Vector3 &dir) const
{
	auto refPose = GetReferenceBonePose(boneId);
	if(!refPose)
		return {};
	auto &rotBone1 = refPose->GetRotation();
	auto forward = uquat::forward(rotBone1);
	auto right = uquat::right(rotBone1);
	auto up = uquat::up(rotBone1);
	auto df = uvec::dot(dir, forward);
	auto dr = uvec::dot(dir, right);
	auto du = uvec::dot(dir, up);
	auto dfa = umath::abs(df);
	auto dra = umath::abs(dr);
	auto dua = umath::abs(du);
	if(dfa >= umath::max(dra, dua))
		return (df < 0) ? pragma::SignedAxis::NegZ : pragma::SignedAxis::Z; // Forward
	else if(dra >= umath::max(dfa, dua))
		return (dr < 0) ? pragma::SignedAxis::NegX : pragma::SignedAxis::X; // Right
	return (du < 0) ? pragma::SignedAxis::NegY : pragma::SignedAxis::Y;     // Up
}
std::optional<pragma::SignedAxis> Model::FindBoneTwistAxis(pragma::animation::BoneId boneId) const
{
	auto refPose = GetReferenceBonePose(boneId);
	if(!refPose)
		return {};

	auto bone = GetSkeleton().GetBone(boneId).lock();
	std::vector<Vector3> normalList;
	normalList.reserve(bone->children.size());
	for(auto &pair : bone->children) {
		auto pose = GetReferenceBonePose(pair.first);
		if(pose) {
			auto normal = pose->GetOrigin() - refPose->GetOrigin();
			uvec::normalize(&normal);
			normalList.push_back(normal);
		}
	}
	Vector3 norm = uvec::FORWARD;
	if(!normalList.empty())
		norm = uvec::calc_average(normalList);
	else {
		if(!bone->parent.expired()) {
			auto refPoseParent = GetReferenceBonePose(bone->parent.lock()->ID);
			if(refPoseParent)
				norm = (refPose->GetOrigin() - refPoseParent->GetOrigin());
		}
	}

	auto dirFromBone0ToBone1 = norm;
	uvec::normalize(&dirFromBone0ToBone1);
	return FindBoneAxisForDirection(boneId, dirFromBone0ToBone1);
}
Quat Model::GetTwistAxisRotationOffset(pragma::SignedAxis axis)
{
	switch(axis) {
	case pragma::SignedAxis::X:
	case pragma::SignedAxis::NegX:
		return uquat::create(EulerAngles(0.0, pragma::is_negative_axis(axis) ? -90.f : 90.f, 0.f));
	case pragma::SignedAxis::Y:
	case pragma::SignedAxis::NegY:
		return uquat::create(EulerAngles(pragma::is_negative_axis(axis) ? -90.f : 90.0, 0.f, 0.f));
	case pragma::SignedAxis::NegZ:
		return uquat::create(EulerAngles(0.0, 0.f, 180.f));
	case pragma::SignedAxis::Z:
		break;
	}
	return uquat::identity();
}

bool Model::GenerateCollisionMeshes(bool convex, float mass, const std::optional<std::string> &surfaceMaterial)
{
	if(!GetCollisionMeshes().empty())
		return false;
	for(auto &mg : m_meshGroups) {
		for(auto &mesh : mg->GetMeshes()) {
			for(auto &sm : mesh->GetSubMeshes()) {
				if(sm->GetGeometryType() != ModelSubMesh::GeometryType::Triangles)
					continue;
				auto numVerts = sm->GetVertexCount() / 20;
				numVerts = umath::clamp<uint32_t>(numVerts, 8, 100);
				auto simplified = sm->Simplify(numVerts);
				auto cmesh = CollisionMesh::Create(m_networkState->GetGameState());
				auto &cverts = cmesh->GetVertices();
				auto &verts = simplified->GetVertices();
				cverts.reserve(verts.size());
				for(auto &v : verts)
					cverts.push_back(v.position);

				if(!convex) {
					auto &ctris = cmesh->GetTriangles();
					simplified->VisitIndices([&ctris](auto *indexDataSrc, uint32_t numIndicesSrc) {
						ctris.reserve(numIndicesSrc);
						for(uint32_t i = 0; i < numIndicesSrc; ++i) {
							auto idx = indexDataSrc[i];
							if(idx >= std::numeric_limits<uint16_t>::max()) {
								// Max allowed index value exceeded
								ctris.clear();
								return;
							}
							ctris.push_back(indexDataSrc[i]);
						}
					});
				}
				cmesh->CalculateVolumeAndCom();
				cmesh->SetConvex(convex);
				cmesh->SetMass(mass);
				if(surfaceMaterial)
					cmesh->SetSurfaceMaterial(*surfaceMaterial);
				cmesh->UpdateShape();

				AddCollisionMesh(cmesh);
			}
		}
	}
	CalculateCollisionBounds();
	return true;
}

bool Model::GenerateLowLevelLODs(Game &game)
{
	if(umath::is_flag_set(m_metaInfo.flags, Flags::GeneratedLODs))
		return false;
	umath::set_flag(m_metaInfo.flags, Flags::GeneratedLODs);
	auto &lods = GetLODs();
	std::string suffix = "_lod_gen";

	if(!lods.empty()) {
		for(auto &lodInfo : lods) {
			for(auto &pair : lodInfo.meshReplacements) {
				auto mg = GetMeshGroup(pair.second);
				if(!mg)
					continue;
				auto &name = mg->GetName();
				if(name.length() >= suffix.length() && name.substr(name.length() - suffix.length()) == suffix)
					return false; // Generated LODs already exist
			}
		}

		auto &lastLodInfo = lods.back();
		auto hasMeshesWithHighVertexCount = false;
		for(auto &pair : lastLodInfo.meshReplacements) {
			auto mg = GetMeshGroup(pair.second);
			if(!mg)
				continue;
			for(auto &m : mg->GetMeshes()) {
				for(auto &sm : m->GetSubMeshes()) {
					if(sm->GetVertexCount() > 600) {
						hasMeshesWithHighVertexCount = true;
						goto endLoop;
					}
				}
			}
		}
	endLoop:

		if(!hasMeshesWithHighVertexCount)
			return false; // LODs with low vertex count already exist, no need to generate any
	}

	auto &baseMeshGroups = GetBaseMeshes();
	std::unordered_set<uint32_t> mgIds;
	mgIds.reserve(baseMeshGroups.size());
	for(auto idx : baseMeshGroups)
		mgIds.insert(idx);
	for(auto &bg : m_bodyGroups) {
		mgIds.reserve(mgIds.size() + bg.meshGroups.size());
		for(auto idx : bg.meshGroups)
			mgIds.insert(idx);
	}

	std::unordered_map<uint32_t, uint32_t> replaceIds;
	auto aggressiveness = 6.0;
	for(auto mgId : mgIds) {
		auto mg = GetMeshGroup(mgId);
		if(!mg)
			continue;
		auto &name = mg->GetName();
		auto lodName = name + suffix;
		uint32_t lodMgId;
		auto mgLod = AddMeshGroup(lodName, lodMgId);
		for(auto &mesh : mg->GetMeshes()) {
			auto mLod = mesh->Copy();
			mLod->GetSubMeshes().clear();

			for(auto &subMesh : mesh->GetSubMeshes()) {
				// Determine a suitable (subjective) target vertex count
				auto targetVertexCount = subMesh->GetVertexCount() / 10;
				auto min = umath::min(subMesh->GetVertexCount(), static_cast<uint32_t>(12));
				targetVertexCount = umath::clamp(targetVertexCount, min, static_cast<uint32_t>(300));

				auto lodSubMesh = subMesh->Simplify(targetVertexCount, aggressiveness);
				if(lodSubMesh->GetVertexCount() == subMesh->GetVertexCount())
					continue;
				mLod->AddSubMesh(lodSubMesh);
			}
			if(mLod->GetSubMeshes().empty())
				continue;
			mgLod->AddMesh(mLod);
		}
		replaceIds[mgId] = lodMgId;
	}

	if(replaceIds.empty())
		return false;

	uint32_t lod = 100;
	float distance = 5'000.f;
	if(!lods.empty()) {
		// Use the highest LOD as reference
		auto &lodLast = lods.back();
		distance = lodLast.distance * 2.f;
		lod = lodLast.lod * 2;
	}
	AddLODInfo(lod, distance, replaceIds);
	return true;
}

void Model::TransformBone(pragma::animation::BoneId boneId, const umath::Transform &t, umath::CoordinateSpace space)
{
	auto bone = m_skeleton->GetBone(boneId).lock();
	if(!bone)
		return;
	switch(space) {
	case umath::CoordinateSpace::View:
	case umath::CoordinateSpace::Screen:
		return;
	case umath::CoordinateSpace::Object:
	case umath::CoordinateSpace::World:
		{
			umath::ScaledTransform parentInvPose {};
			auto parent = bone->parent.lock();
			if(parent) {
				m_reference->GetBonePose(parent->ID, parentInvPose);
				parentInvPose = parentInvPose.GetInverse();
			}

			umath::ScaledTransform curPose;
			GetReferenceBonePose(boneId, curPose);
			auto oldRelPose = parentInvPose * curPose;
			auto newPose = t * curPose;
			m_reference->SetBonePose(bone->ID, newPose);

			auto newRelPose = parentInvPose * newPose;
			auto oldPoseToNewPose = oldRelPose.GetInverse() * newRelPose;

			for(auto &anim : m_animations) {
				if(anim->HasFlag(FAnim::Gesture))
					continue;
				auto &boneMap = anim->GetBoneMap();
				auto it = boneMap.find(bone->ID);
				if(it == boneMap.end())
					continue;
				auto i = it->second;
				for(auto &frame : anim->GetFrames()) {
					umath::ScaledTransform pose;
					if(!frame->GetBonePose(i, pose))
						continue;
					pose = pose * oldPoseToNewPose;
					frame->SetBonePose(i, pose);
				}
			}
			break;
		}
	case umath::CoordinateSpace::Local:
		if(bone->parent.expired())
			return TransformBone(boneId, t, umath::CoordinateSpace::World);
		umath::ScaledTransform parentPose;
		m_reference->GetBonePose(bone->parent.lock()->ID, parentPose);

		umath::ScaledTransform curPose;
		GetReferenceBonePose(boneId, curPose);

		auto newPose = parentPose.GetInverse() * curPose;
		newPose = t * newPose;
		newPose = parentPose * newPose;

		return TransformBone(boneId, newPose, umath::CoordinateSpace::World);
	}
}

Vector3 pragma::model::get_mirror_transform_vector(pragma::Axis axis)
{
	switch(axis) {
	case Axis::X:
		return Vector3 {-1.f, 0.f, 0.f};
	case Axis::Y:
		return Vector3 {0.f, -1.f, 0.f};
	case Axis::Z:
		return Vector3 {0.f, 0.f, -1.f};
	}
	return Vector3 {1.f, 1.f, 1.f};
}
