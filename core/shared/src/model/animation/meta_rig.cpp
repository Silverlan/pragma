/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "pragma/model/animation/meta_rig.hpp"
#include "pragma/model/animation/skeleton.hpp"
#include "pragma/model/model.h"

pragma::animation::BoneId pragma::animation::MetaRig::GetBoneId(const pragma::GString &type) const
{
	auto eType = get_meta_rig_bone_type_enum(type);
	if(!eType)
		return pragma::animation::INVALID_BONE_INDEX;
	return GetBoneId(*eType);
}

pragma::animation::BoneId pragma::animation::MetaRig::GetBoneId(pragma::animation::MetaRigBoneType type) const
{
	auto i = umath::to_integral(type);
	if(i >= bones.size())
		return pragma::animation::INVALID_BONE_INDEX;
	return bones[i].boneId;
}

std::shared_ptr<pragma::animation::MetaRig> pragma::animation::MetaRig::Load(const Skeleton &skeleton, const udm::AssetData &data, std::string &outErr)
{
	auto metaRig = std::make_shared<MetaRig>();
	if(metaRig->LoadFromAssetData(skeleton, data, outErr) == false)
		return nullptr;
	return metaRig;
}

bool pragma::animation::MetaRig::LoadFromAssetData(const Skeleton &skeleton, const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PMRIG_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}

	auto &udmMetaRig = udm;
	udmMetaRig["rigType"] >> rigType;
	udmMetaRig["forwardFacingRotationOffset"] >> forwardFacingRotationOffset;
	udmMetaRig["forwardAxis"] >> forwardAxis;
	udmMetaRig["upAxis"] >> upAxis;
	udmMetaRig["bounds"]["min"] >> min;
	udmMetaRig["bounds"]["max"] >> max;
	for(auto &udmBone : udmMetaRig["bones"]) {
		std::string type;
		udmBone["type"] >> type;
		auto etype = pragma::animation::get_meta_rig_bone_type_enum(type);
		if(!etype)
			continue;
		std::string bone;
		udmBone["bone"] >> bone;
		auto boneId = skeleton.LookupBone(bone);
		if(boneId == pragma::animation::INVALID_BONE_INDEX)
			continue;
		auto &metaBone = bones[umath::to_integral(*etype)];
		metaBone.boneId = boneId;
		udmBone["normalizedRotationOffset"] >> metaBone.normalizedRotationOffset;
		udmBone["radius"] >> metaBone.radius;
		udmBone["length"] >> metaBone.length;

		auto udmBounds = udmBone["bounds"];
		udmBounds["min"] >> metaBone.bounds.first;
		udmBounds["max"] >> metaBone.bounds.second;
	}

	for(auto &udmBlendShape : udmMetaRig["blendShapes"]) {
		std::string type;
		udmBlendShape["type"] >> type;
		auto etype = pragma::animation::get_blend_shape_enum(type);
		if(!etype)
			continue;
		pragma::animation::FlexControllerId flexCId = pragma::animation::INVALID_FLEX_CONTROLLER_INDEX;
		udmBlendShape["flexControllerId"] >> flexCId;
		if(flexCId == pragma::animation::INVALID_FLEX_CONTROLLER_INDEX)
			continue;
		auto &blendShape = blendShapes[umath::to_integral(*etype)];
		blendShape.flexControllerId = flexCId;
	}
	return true;
}

bool pragma::animation::MetaRig::Save(const Skeleton &skeleton, udm::AssetDataArg outData, std::string &outErr) const
{
	outData.SetAssetType(PMRIG_IDENTIFIER);
	outData.SetAssetVersion(FORMAT_VERSION);
	auto udmMetaRig = *outData;

	udmMetaRig["rigType"] << rigType;
	udmMetaRig["forwardFacingRotationOffset"] << forwardFacingRotationOffset;
	udmMetaRig["forwardAxis"] << forwardAxis;
	udmMetaRig["upAxis"] << upAxis;
	udmMetaRig["bounds"]["min"] << min;
	udmMetaRig["bounds"]["max"] << max;

	size_t numValidMetaBones = 0;
	for(auto &metaBone : bones) {
		auto bone = skeleton.GetBone(metaBone.boneId);
		if(bone.expired())
			continue;
		++numValidMetaBones;
	}

	auto udmBones = udmMetaRig.AddArray("bones", numValidMetaBones);
	size_t idx = 0;
	for(size_t i = 0; i < bones.size(); ++i) {
		auto &metaBone = bones[i];
		auto bone = skeleton.GetBone(metaBone.boneId);
		if(bone.expired())
			continue;
		auto udmBone = udmBones[idx++];
		udmBone["type"] << pragma::animation::get_meta_rig_bone_type_name(static_cast<pragma::animation::MetaRigBoneType>(i));
		udmBone["bone"] << std::string {bone.lock()->name};
		udmBone["normalizedRotationOffset"] << metaBone.normalizedRotationOffset;

		udmBone["radius"] << metaBone.radius;
		udmBone["length"] << metaBone.length;

		auto udmBounds = udmBone["bounds"];
		udmBounds["min"] << metaBone.bounds.first;
		udmBounds["max"] << metaBone.bounds.second;
	}

	size_t numValidBlendShapes = 0;
	for(auto &blendShape : blendShapes) {
		if(blendShape.flexControllerId == pragma::animation::INVALID_FLEX_CONTROLLER_INDEX)
			continue;
		++numValidBlendShapes;
	}

	auto udmBlendShapes = udmMetaRig.AddArray("blendShapes", numValidBlendShapes);
	idx = 0;
	for(size_t i = 0; i < blendShapes.size(); ++i) {
		auto &blendShape = blendShapes[i];
		if(blendShape.flexControllerId == pragma::animation::INVALID_FLEX_CONTROLLER_INDEX)
			continue;
		auto udmBlendShape = udmBlendShapes[idx++];
		udmBlendShape["type"] << pragma::animation::get_blend_shape_name(static_cast<pragma::animation::BlendShape>(i));
		udmBlendShape["flexControllerId"] << blendShape.flexControllerId;
	}
	return true;
}

void pragma::animation::MetaRig::DebugPrint(const Model &mdl)
{
	auto &skeleton = mdl.GetSkeleton();
	std::stringstream ss;
	ss << "MetaRig\n";
	ss << "Rig type: " << magic_enum::enum_name(rigType) << "\n";
	EulerAngles forwardFacingRotationAngles {forwardFacingRotationOffset};
	ss << "Forward facing rotation offset " << forwardFacingRotationAngles.p << "," << forwardFacingRotationAngles.y << "," << forwardFacingRotationAngles.r << "\n";
	ss << "Forward axis: " << magic_enum::enum_name(forwardAxis) << "\n";
	ss << "Up axis: " << magic_enum::enum_name(upAxis) << "\n";
	ss << "Min: " << min.x << "," << min.y << "," << min.z << "\n";
	ss << "Max: " << max.x << "," << max.y << "," << max.z << "\n";
	auto printBone = [&ss, &skeleton](MetaRigBoneType boneType, const MetaRigBone &rigBone) {
		ss << "\t" << magic_enum::enum_name(boneType) << "\n";
		auto bone = skeleton.GetBone(rigBone.boneId).lock();
		ss << "\t\tBone: " << (bone ? bone->name : "NULL") << "\n";
		EulerAngles ang {rigBone.normalizedRotationOffset};
		ss << "\t\tNormalized rotation offset: " << ang.p << "," << ang.y << "," << ang.r << "\n";
		auto &[min, max] = rigBone.bounds;
		ss << "\t\tBounds: (" << min.x << "," << min.y << "," << min.z << ") (" << max.x << "," << max.y << "," << max.z << ")\n";
		ss << "\t\tRadius: " << rigBone.radius << "\n";
		ss << "\t\tLength: " << rigBone.length << "\n";
		ss << "\n";
	};
	for(size_t i = 0; i < bones.size(); ++i)
		printBone(static_cast<MetaRigBoneType>(i), bones[i]);
	ss << "\nBlend shapes:\n";
	for(size_t i = 0; i < umath::to_integral(BlendShape::Count); ++i) {
		auto blendShape = static_cast<BlendShape>(i);
		auto &blendShapeInfo = blendShapes[i];
		auto flexConId = blendShapeInfo.flexControllerId;
		ss << magic_enum::enum_name(blendShape) << " = ";
		auto *flexCon = mdl.GetFlexController(flexConId);
		if(!flexCon)
			ss << "NULL";
		else
			ss << flexCon->name;
		ss << "\n";
	}

	Con::cout << ss.str() << Con::endl;
}
std::optional<pragma::animation::MetaRigBoneType> pragma::animation::MetaRig::FindMetaBoneType(pragma::animation::BoneId boneId) const
{
	size_t i = 0;
	for(auto &bone : bones) {
		if(bone.boneId == boneId)
			return static_cast<pragma::animation::MetaRigBoneType>(i);
		++i;
	}
	return {};
}
const pragma::animation::MetaRigBlendShape *pragma::animation::MetaRig::GetBlendShape(pragma::animation::BlendShape blendShape) const
{
	auto i = umath::to_integral(blendShape);
	if(i >= blendShapes.size() || blendShapes[i].flexControllerId == pragma::animation::INVALID_FLEX_CONTROLLER_INDEX)
		return nullptr;
	return &blendShapes[i];
}
const pragma::animation::MetaRigBone *pragma::animation::MetaRig::GetBone(pragma::animation::MetaRigBoneType type) const
{
	auto i = umath::to_integral(type);
	if(i >= bones.size() || bones[i].boneId == pragma::animation::INVALID_BONE_INDEX)
		return nullptr;
	return &bones[i];
}

std::vector<pragma::animation::MetaRigBoneType> pragma::animation::get_meta_rig_bone_children(MetaRigBoneType type)
{
	switch(type) {
	case MetaRigBoneType::Hips:
		return {MetaRigBoneType::Pelvis, MetaRigBoneType::Spine, MetaRigBoneType::LeftUpperLeg, MetaRigBoneType::RightUpperLeg};
	case MetaRigBoneType::Pelvis:
		return {};
	case MetaRigBoneType::Spine:
		return {MetaRigBoneType::Spine1};
	case MetaRigBoneType::Spine1:
		return {MetaRigBoneType::Spine2};
	case MetaRigBoneType::Spine2:
		return {MetaRigBoneType::Spine3};
	case MetaRigBoneType::Spine3:
		return {MetaRigBoneType::Neck, MetaRigBoneType::LeftUpperArm, MetaRigBoneType::RightUpperArm, MetaRigBoneType::LeftBreastBase, MetaRigBoneType::RightBreastBase};
	case MetaRigBoneType::Neck:
		return {MetaRigBoneType::Head};
	case MetaRigBoneType::Head:
		return {MetaRigBoneType::Jaw, MetaRigBoneType::LeftEar, MetaRigBoneType::RightEar, MetaRigBoneType::LeftEye, MetaRigBoneType::RightEye, MetaRigBoneType::CenterEye};
	case MetaRigBoneType::Jaw:
		return {};
	case MetaRigBoneType::LeftEar:
		return {};
	case MetaRigBoneType::RightEar:
		return {};
	case MetaRigBoneType::LeftEye:
		return {};
	case MetaRigBoneType::RightEye:
		return {};
	case MetaRigBoneType::CenterEye:
		return {};
	case MetaRigBoneType::LeftUpperArm:
		return {MetaRigBoneType::LeftLowerArm};
	case MetaRigBoneType::LeftLowerArm:
		return {MetaRigBoneType::LeftHand};
	case MetaRigBoneType::LeftHand:
		return {MetaRigBoneType::LeftThumb1, MetaRigBoneType::LeftIndexFinger1, MetaRigBoneType::LeftMiddleFinger1, MetaRigBoneType::LeftRingFinger1, MetaRigBoneType::LeftLittleFinger1};
	case MetaRigBoneType::RightUpperArm:
		return {MetaRigBoneType::RightLowerArm};
	case MetaRigBoneType::RightLowerArm:
		return {MetaRigBoneType::RightHand};
	case MetaRigBoneType::RightHand:
		return {MetaRigBoneType::RightThumb1, MetaRigBoneType::RightIndexFinger1, MetaRigBoneType::RightMiddleFinger1, MetaRigBoneType::RightRingFinger1, MetaRigBoneType::RightLittleFinger1};
	case MetaRigBoneType::LeftUpperLeg:
		return {MetaRigBoneType::LeftLowerLeg};
	case MetaRigBoneType::LeftLowerLeg:
		return {MetaRigBoneType::LeftFoot};
	case MetaRigBoneType::LeftFoot:
		return {MetaRigBoneType::LeftToe};
	case MetaRigBoneType::LeftToe:
		return {};
	case MetaRigBoneType::RightUpperLeg:
		return {MetaRigBoneType::RightLowerLeg};
	case MetaRigBoneType::RightLowerLeg:
		return {MetaRigBoneType::RightFoot};
	case MetaRigBoneType::RightFoot:
		return {MetaRigBoneType::RightToe};
	case MetaRigBoneType::RightToe:
		return {};
	case MetaRigBoneType::LeftThumb1:
		return {MetaRigBoneType::LeftThumb2};
	case MetaRigBoneType::LeftThumb2:
		return {MetaRigBoneType::LeftThumb3};
	case MetaRigBoneType::LeftThumb3:
		return {};
	case MetaRigBoneType::LeftIndexFinger1:
		return {MetaRigBoneType::LeftIndexFinger2};
	case MetaRigBoneType::LeftIndexFinger2:
		return {MetaRigBoneType::LeftIndexFinger3};
	case MetaRigBoneType::LeftIndexFinger3:
		return {};
	case MetaRigBoneType::LeftMiddleFinger1:
		return {MetaRigBoneType::LeftMiddleFinger2};
	case MetaRigBoneType::LeftMiddleFinger2:
		return {MetaRigBoneType::LeftMiddleFinger3};
	case MetaRigBoneType::LeftMiddleFinger3:
		return {};
	case MetaRigBoneType::LeftRingFinger1:
		return {MetaRigBoneType::LeftRingFinger2};
	case MetaRigBoneType::LeftRingFinger2:
		return {MetaRigBoneType::LeftRingFinger3};
	case MetaRigBoneType::LeftRingFinger3:
		return {};
	case MetaRigBoneType::LeftLittleFinger1:
		return {MetaRigBoneType::LeftLittleFinger2};
	case MetaRigBoneType::LeftLittleFinger2:
		return {MetaRigBoneType::LeftLittleFinger3};
	case MetaRigBoneType::LeftLittleFinger3:
		return {};
	case MetaRigBoneType::RightThumb1:
		return {MetaRigBoneType::RightThumb2};
	case MetaRigBoneType::RightThumb2:
		return {MetaRigBoneType::RightThumb3};
	case MetaRigBoneType::RightThumb3:
		return {};
	case MetaRigBoneType::RightIndexFinger1:
		return {MetaRigBoneType::RightIndexFinger2};
	case MetaRigBoneType::RightIndexFinger2:
		return {MetaRigBoneType::RightIndexFinger3};
	case MetaRigBoneType::RightIndexFinger3:
		return {};
	case MetaRigBoneType::RightMiddleFinger1:
		return {MetaRigBoneType::RightMiddleFinger2};
	case MetaRigBoneType::RightMiddleFinger2:
		return {MetaRigBoneType::RightMiddleFinger3};
	case MetaRigBoneType::RightMiddleFinger3:
		return {};
	case MetaRigBoneType::RightRingFinger1:
		return {MetaRigBoneType::RightRingFinger2};
	case MetaRigBoneType::RightRingFinger2:
		return {MetaRigBoneType::RightRingFinger3};
	case MetaRigBoneType::RightRingFinger3:
		return {};
	case MetaRigBoneType::RightLittleFinger1:
		return {MetaRigBoneType::RightLittleFinger2};
	case MetaRigBoneType::RightLittleFinger2:
		return {MetaRigBoneType::RightLittleFinger3};
	case MetaRigBoneType::RightLittleFinger3:
		return {};
	case MetaRigBoneType::TailBase:
		return {MetaRigBoneType::TailMiddle};
	case MetaRigBoneType::TailMiddle:
		return {MetaRigBoneType::TailMiddle1};
	case MetaRigBoneType::TailMiddle1:
		return {MetaRigBoneType::TailTip};
	case MetaRigBoneType::TailTip:
		return {};
	case MetaRigBoneType::LeftWing:
		return {MetaRigBoneType::LeftWingMiddle};
	case MetaRigBoneType::LeftWingMiddle:
		return {MetaRigBoneType::LeftWingTip};
	case MetaRigBoneType::LeftWingTip:
		return {};
	case MetaRigBoneType::RightWing:
		return {MetaRigBoneType::RightWingMiddle};
	case MetaRigBoneType::RightWingMiddle:
		return {MetaRigBoneType::RightWingTip};
	case MetaRigBoneType::RightWingTip:
		return {};
	case MetaRigBoneType::LeftBreastBase:
		return {MetaRigBoneType::LeftBreastMiddle};
	case MetaRigBoneType::LeftBreastMiddle:
		return {MetaRigBoneType::LeftBreastTip};
	case MetaRigBoneType::RightBreastBase:
		return {MetaRigBoneType::RightBreastMiddle};
	case MetaRigBoneType::RightBreastMiddle:
		return {MetaRigBoneType::RightBreastTip};
	}
	static_assert(umath::to_integral(MetaRigBoneType::Count) == 74, "Update this list when new types are added!");
	return {};
}
