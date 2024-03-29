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

pragma::animation::BoneId pragma::animation::MetaRig::GetBoneId(pragma::animation::MetaRigBoneType &type) const
{
	auto i = umath::to_integral(type);
	if(i >= bones.size())
		return pragma::animation::INVALID_BONE_INDEX;
	return bones[i].boneId;
}

void pragma::animation::MetaRig::DebugPrint(const Model &mdl)
{
	auto &skeleton = mdl.GetSkeleton();
	std::stringstream ss;
	ss << "MetaRig\n";
	auto printBone = [&ss, &skeleton](MetaRigBoneType boneType, const MetaRigBone &rigBone) {
		ss << "\t" << magic_enum::enum_name(boneType) << "\n";
		auto bone = skeleton.GetBone(rigBone.boneId).lock();
		ss << "\t\tBone: " << (bone ? bone->name : "NULL") << "\n";
		EulerAngles ang {rigBone.normalizedRotationOffset};
		ss << "\t\tNormalized rotation offset: " << ang.p << "," << ang.y << "," << ang.r << "\n";
		auto &[min, max] = rigBone.bounds;
		ss << "\t\tBounds: (" << min.x << "," << min.y << "," << min.z << ") (" << max.x << "," << max.y << "," << max.z << ")\n";
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
		return {MetaRigBoneType::Pelvis, MetaRigBoneType::Spine};
	case MetaRigBoneType::Pelvis:
		return {};
	case MetaRigBoneType::Spine:
		return {MetaRigBoneType::Spine1};
	case MetaRigBoneType::Spine1:
		return {MetaRigBoneType::Spine2};
	case MetaRigBoneType::Spine2:
		return {MetaRigBoneType::Spine3};
	case MetaRigBoneType::Spine3:
		return {MetaRigBoneType::Neck, MetaRigBoneType::LeftUpperArm, MetaRigBoneType::RightUpperArm};
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
	}
	static_assert(umath::to_integral(MetaRigBoneType::Count) == 68, "Update this list when new types are added!");
	return {};
}
