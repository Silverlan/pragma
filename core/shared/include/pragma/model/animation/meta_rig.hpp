/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_META_RIG_HPP__
#define __PRAGMA_META_RIG_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/util/global_string_table.hpp"
#include "pragma/model/animation/bone.hpp"
#include "pragma/types.hpp"
#include <unordered_map>
#include <string>

class Model;
namespace udm {
	struct AssetData;
};
namespace pragma::animation {
	enum class MetaRigBoneType : uint8_t {
		// Note: Child bone types must not be defined before their parent!
		Hips = 0,
		Pelvis,

		Spine,
		Spine1,
		Spine2,
		Spine3,

		Neck,
		Head,
		Jaw,
		LeftEar,
		RightEar,
		LeftEye,
		RightEye,
		CenterEye,

		LeftUpperArm,
		LeftLowerArm,
		LeftHand,

		RightUpperArm,
		RightLowerArm,
		RightHand,

		LeftUpperLeg,
		LeftLowerLeg,
		LeftFoot,
		LeftToe,

		RightUpperLeg,
		RightLowerLeg,
		RightFoot,
		RightToe,

		LeftThumb1,
		LeftThumb2,
		LeftThumb3,
		LeftIndexFinger1,
		LeftIndexFinger2,
		LeftIndexFinger3,
		LeftMiddleFinger1,
		LeftMiddleFinger2,
		LeftMiddleFinger3,
		LeftRingFinger1,
		LeftRingFinger2,
		LeftRingFinger3,
		LeftLittleFinger1,
		LeftLittleFinger2,
		LeftLittleFinger3,

		RightThumb1,
		RightThumb2,
		RightThumb3,
		RightIndexFinger1,
		RightIndexFinger2,
		RightIndexFinger3,
		RightMiddleFinger1,
		RightMiddleFinger2,
		RightMiddleFinger3,
		RightRingFinger1,
		RightRingFinger2,
		RightRingFinger3,
		RightLittleFinger1,
		RightLittleFinger2,
		RightLittleFinger3,

		TailBase,
		TailMiddle,
		TailMiddle1,
		TailTip,

		LeftWing,
		LeftWingMiddle,
		LeftWingTip,
		RightWing,
		RightWingMiddle,
		RightWingTip,
		Count,

		Invalid = std::numeric_limits<uint8_t>::max(),
	};
	constexpr const char *get_meta_rig_bone_type_name(MetaRigBoneType type);
	constexpr std::optional<MetaRigBoneType> get_meta_rig_bone_type_enum(const std::string_view &boneType);
	constexpr std::optional<MetaRigBoneType> get_meta_rig_bone_parent_type(MetaRigBoneType type);
	DLLNETWORK std::vector<pragma::animation::MetaRigBoneType> get_meta_rig_bone_children(MetaRigBoneType type);

	enum class BoneSide : uint8_t { Left = 0, Right, None };
	constexpr std::optional<BoneSide> get_meta_rig_bone_type_side(MetaRigBoneType type);

	enum class BlendShape : uint32_t {
		Neutral = 0,
		BrowDownLeft,
		BrowDownRight,
		BrowInnerUp,
		BrowOuterUpLeft,
		BrowOuterUpRight,
		CheekPuff,
		CheekSquintLeft,
		CheekSquintRight,
		EyeBlinkLeft,
		EyeBlinkRight,
		EyeLookDownLeft,
		EyeLookDownRight,
		EyeLookInLeft,
		EyeLookInRight,
		EyeLookOutLeft,
		EyeLookOutRight,
		EyeLookUpLeft,
		EyeLookUpRight,
		EyeSquintLeft,
		EyeSquintRight,
		EyeWideLeft,
		EyeWideRight,
		JawForward,
		JawLeft,
		JawOpen,
		JawRight,
		MouthClose,
		MouthDimpleLeft,
		MouthDimpleRight,
		MouthFrownLeft,
		MouthFrownRight,
		MouthFunnel,
		MouthLeft,
		MouthLowerDownLeft,
		MouthLowerDownRight,
		MouthPressLeft,
		MouthPressRight,
		MouthPucker,
		MouthRight,
		MouthRollLower,
		MouthRollUpper,
		MouthShrugLower,
		MouthShrugUpper,
		MouthSmileLeft,
		MouthSmileRight,
		MouthStretchLeft,
		MouthStretchRight,
		MouthUpperUpLeft,
		MouthUpperUpRight,
		NoseSneerLeft,
		NoseSneerRight,

		TongueOut,

		EyeDilationLeft,
		EyeDilationRight,
		EyeConstrictLeft,
		EyeConstrictRight,

		CheekSuckLeft,
		CheekSuckRight,

		MouthTightenerLeft,
		MouthTightenerRight,

		Count
	};
	constexpr const char *get_blend_shape_name(BlendShape blendShape);
	constexpr std::optional<BlendShape> get_blend_shape_enum(const std::string_view &name);

	struct DLLNETWORK MetaRigBone {
		pragma::animation::BoneId boneId = pragma::animation::INVALID_BONE_INDEX;
		std::pair<Vector3, Vector3> bounds;
		Quat normalizedRotationOffset; // Rotation offset from bone rotation to normalized rotation
	};

	struct DLLNETWORK MetaRigBlendShape {
		FlexControllerId flexControllerId = pragma::animation::INVALID_FLEX_CONTROLLER_INDEX;
	};

	enum class RigType : uint8_t { Biped = 0, Quadruped };
	class Skeleton;
	struct DLLNETWORK MetaRig {
		static constexpr uint32_t FORMAT_VERSION = 1u;
		static constexpr auto PMRIG_IDENTIFIER = "PMRIG";
		static std::shared_ptr<MetaRig> Load(const Skeleton &skeleton, const udm::AssetData &data, std::string &outErr);
		std::array<MetaRigBone, umath::to_integral(MetaRigBoneType::Count)> bones;
		std::array<MetaRigBlendShape, umath::to_integral(BlendShape::Count)> blendShapes;

		bool Save(const Skeleton &skeleton, const udm::AssetData &outData, std::string &outErr);
		void DebugPrint(const Model &mdl);
		const MetaRigBone *GetBone(pragma::animation::MetaRigBoneType type) const;
		const MetaRigBlendShape *GetBlendShape(pragma::animation::BlendShape blendShape) const;
		pragma::animation::BoneId GetBoneId(const pragma::GString &type) const;
		pragma::animation::BoneId GetBoneId(pragma::animation::MetaRigBoneType type) const;
		RigType rigType = RigType::Biped;
		Quat forwardFacingRotationOffset = uquat::identity();
		pragma::SignedAxis forwardAxis = pragma::SignedAxis::Z;
		pragma::SignedAxis upAxis = pragma::SignedAxis::Y;
	  private:
		bool LoadFromAssetData(const Skeleton &skeleton, const udm::AssetData &data, std::string &outErr);
	};
};

constexpr const char *pragma::animation::get_meta_rig_bone_type_name(MetaRigBoneType type)
{
	switch(type) {
	case MetaRigBoneType::Head:
		return "head";
	case MetaRigBoneType::Neck:
		return "neck";
	case MetaRigBoneType::Jaw:
		return "jaw";
	case MetaRigBoneType::LeftUpperArm:
		return "left_upper_arm";
	case MetaRigBoneType::LeftLowerArm:
		return "left_lower_arm";
	case MetaRigBoneType::LeftHand:
		return "left_hand";
	case MetaRigBoneType::RightUpperArm:
		return "right_upper_arm";
	case MetaRigBoneType::RightLowerArm:
		return "right_lower_arm";
	case MetaRigBoneType::RightHand:
		return "right_hand";
	case MetaRigBoneType::LeftUpperLeg:
		return "left_upper_leg";
	case MetaRigBoneType::LeftLowerLeg:
		return "left_lower_leg";
	case MetaRigBoneType::LeftFoot:
		return "left_foot";
	case MetaRigBoneType::LeftToe:
		return "left_toe";
	case MetaRigBoneType::RightUpperLeg:
		return "right_upper_leg";
	case MetaRigBoneType::RightLowerLeg:
		return "right_lower_leg";
	case MetaRigBoneType::RightFoot:
		return "right_foot";
	case MetaRigBoneType::RightToe:
		return "right_toe";
	case MetaRigBoneType::Spine:
		return "spine";
	case MetaRigBoneType::Spine1:
		return "spine1";
	case MetaRigBoneType::Spine2:
		return "spine2";
	case MetaRigBoneType::Spine3:
		return "spine3";
	case MetaRigBoneType::Pelvis:
		return "pelvis";
	case MetaRigBoneType::Hips:
		return "hips";
	case MetaRigBoneType::LeftThumb1:
		return "left_thumb1";
	case MetaRigBoneType::LeftThumb2:
		return "left_thumb2";
	case MetaRigBoneType::LeftThumb3:
		return "left_thumb3";
	case MetaRigBoneType::LeftIndexFinger1:
		return "left_index_finger1";
	case MetaRigBoneType::LeftIndexFinger2:
		return "left_index_finger2";
	case MetaRigBoneType::LeftIndexFinger3:
		return "left_index_finger3";
	case MetaRigBoneType::LeftMiddleFinger1:
		return "left_middle_finger1";
	case MetaRigBoneType::LeftMiddleFinger2:
		return "left_middle_finger2";
	case MetaRigBoneType::LeftMiddleFinger3:
		return "left_middle_finger3";
	case MetaRigBoneType::LeftRingFinger1:
		return "left_ring_finger1";
	case MetaRigBoneType::LeftRingFinger2:
		return "left_ring_finger2";
	case MetaRigBoneType::LeftRingFinger3:
		return "left_ring_finger3";
	case MetaRigBoneType::LeftLittleFinger1:
		return "left_little_finger1";
	case MetaRigBoneType::LeftLittleFinger2:
		return "left_little_finger2";
	case MetaRigBoneType::LeftLittleFinger3:
		return "left_little_finger3";
	case MetaRigBoneType::RightThumb1:
		return "right_thumb1";
	case MetaRigBoneType::RightThumb2:
		return "right_thumb2";
	case MetaRigBoneType::RightThumb3:
		return "right_thumb3";
	case MetaRigBoneType::RightIndexFinger1:
		return "right_index_finger1";
	case MetaRigBoneType::RightIndexFinger2:
		return "right_index_finger2";
	case MetaRigBoneType::RightIndexFinger3:
		return "right_index_finger3";
	case MetaRigBoneType::RightMiddleFinger1:
		return "right_middle_finger1";
	case MetaRigBoneType::RightMiddleFinger2:
		return "right_middle_finger2";
	case MetaRigBoneType::RightMiddleFinger3:
		return "right_middle_finger3";
	case MetaRigBoneType::RightRingFinger1:
		return "right_ring_finger1";
	case MetaRigBoneType::RightRingFinger2:
		return "right_ring_finger2";
	case MetaRigBoneType::RightRingFinger3:
		return "right_ring_finger3";
	case MetaRigBoneType::RightLittleFinger1:
		return "right_little_finger1";
	case MetaRigBoneType::RightLittleFinger2:
		return "right_little_finger2";
	case MetaRigBoneType::RightLittleFinger3:
		return "right_little_finger3";
	case MetaRigBoneType::TailBase:
		return "tail_base";
	case MetaRigBoneType::TailMiddle:
		return "tail_middle";
	case MetaRigBoneType::TailMiddle1:
		return "tail_middle1";
	case MetaRigBoneType::TailTip:
		return "tail_tip";
	case MetaRigBoneType::LeftEar:
		return "left_ear";
	case MetaRigBoneType::RightEar:
		return "right_ear";
	case MetaRigBoneType::LeftEye:
		return "left_eye";
	case MetaRigBoneType::RightEye:
		return "right_eye";
	case MetaRigBoneType::CenterEye:
		return "center_eye";
	case MetaRigBoneType::LeftWing:
		return "left_wing";
	case MetaRigBoneType::LeftWingMiddle:
		return "left_wing_middle";
	case MetaRigBoneType::LeftWingTip:
		return "left_wing_tip";
	case MetaRigBoneType::RightWing:
		return "right_wing";
	case MetaRigBoneType::RightWingMiddle:
		return "right_wing_middle";
	case MetaRigBoneType::RightWingTip:
		return "right_wing_tip";
	}
	static_assert(umath::to_integral(MetaRigBoneType::Count) == 68, "Update this list when new types are added!");
	return nullptr;
}

constexpr std::optional<pragma::animation::MetaRigBoneType> pragma::animation::get_meta_rig_bone_type_enum(const std::string_view &boneType)
{
	using namespace ustring::string_switch_ci;
	switch(ustring::string_switch_ci::hash(boneType)) {
	case "head"_:
		return pragma::animation::MetaRigBoneType::Head;
	case "neck"_:
		return pragma::animation::MetaRigBoneType::Neck;
	case "jaw"_:
		return pragma::animation::MetaRigBoneType::Jaw;
	case "left_upper_arm"_:
		return pragma::animation::MetaRigBoneType::LeftUpperArm;
	case "left_lower_arm"_:
		return pragma::animation::MetaRigBoneType::LeftLowerArm;
	case "left_hand"_:
		return pragma::animation::MetaRigBoneType::LeftHand;
	case "right_upper_arm"_:
		return pragma::animation::MetaRigBoneType::RightUpperArm;
	case "right_lower_arm"_:
		return pragma::animation::MetaRigBoneType::RightLowerArm;
	case "right_hand"_:
		return pragma::animation::MetaRigBoneType::RightHand;
	case "left_upper_leg"_:
		return pragma::animation::MetaRigBoneType::LeftUpperLeg;
	case "left_lower_leg"_:
		return pragma::animation::MetaRigBoneType::LeftLowerLeg;
	case "left_foot"_:
		return pragma::animation::MetaRigBoneType::LeftFoot;
	case "left_toe"_:
		return pragma::animation::MetaRigBoneType::LeftToe;
	case "right_upper_leg"_:
		return pragma::animation::MetaRigBoneType::RightUpperLeg;
	case "right_lower_leg"_:
		return pragma::animation::MetaRigBoneType::RightLowerLeg;
	case "right_foot"_:
		return pragma::animation::MetaRigBoneType::RightFoot;
	case "right_toe"_:
		return pragma::animation::MetaRigBoneType::RightToe;
	case "spine"_:
		return pragma::animation::MetaRigBoneType::Spine;
	case "spine1"_:
		return pragma::animation::MetaRigBoneType::Spine1;
	case "spine2"_:
		return pragma::animation::MetaRigBoneType::Spine2;
	case "spine3"_:
		return pragma::animation::MetaRigBoneType::Spine3;
	case "pelvis"_:
		return pragma::animation::MetaRigBoneType::Pelvis;
	case "hips"_:
		return pragma::animation::MetaRigBoneType::Hips;
	case "left_thumb1"_:
		return pragma::animation::MetaRigBoneType::LeftThumb1;
	case "left_thumb2"_:
		return pragma::animation::MetaRigBoneType::LeftThumb2;
	case "left_thumb3"_:
		return pragma::animation::MetaRigBoneType::LeftThumb3;
	case "left_index_finger1"_:
		return pragma::animation::MetaRigBoneType::LeftIndexFinger1;
	case "left_index_finger2"_:
		return pragma::animation::MetaRigBoneType::LeftIndexFinger2;
	case "left_index_finger3"_:
		return pragma::animation::MetaRigBoneType::LeftIndexFinger3;
	case "left_middle_finger1"_:
		return pragma::animation::MetaRigBoneType::LeftMiddleFinger1;
	case "left_middle_finger2"_:
		return pragma::animation::MetaRigBoneType::LeftMiddleFinger2;
	case "left_middle_finger3"_:
		return pragma::animation::MetaRigBoneType::LeftMiddleFinger3;
	case "left_ring_finger1"_:
		return pragma::animation::MetaRigBoneType::LeftRingFinger1;
	case "left_ring_finger2"_:
		return pragma::animation::MetaRigBoneType::LeftRingFinger2;
	case "left_ring_finger3"_:
		return pragma::animation::MetaRigBoneType::LeftRingFinger3;
	case "left_little_finger1"_:
		return pragma::animation::MetaRigBoneType::LeftLittleFinger1;
	case "left_little_finger2"_:
		return pragma::animation::MetaRigBoneType::LeftLittleFinger2;
	case "left_little_finger3"_:
		return pragma::animation::MetaRigBoneType::LeftLittleFinger3;
	case "right_thumb1"_:
		return pragma::animation::MetaRigBoneType::RightThumb1;
	case "right_thumb2"_:
		return pragma::animation::MetaRigBoneType::RightThumb2;
	case "right_thumb3"_:
		return pragma::animation::MetaRigBoneType::RightThumb3;
	case "right_index_finger1"_:
		return pragma::animation::MetaRigBoneType::RightIndexFinger1;
	case "right_index_finger2"_:
		return pragma::animation::MetaRigBoneType::RightIndexFinger2;
	case "right_index_finger3"_:
		return pragma::animation::MetaRigBoneType::RightIndexFinger3;
	case "right_middle_finger1"_:
		return pragma::animation::MetaRigBoneType::RightMiddleFinger1;
	case "right_middle_finger2"_:
		return pragma::animation::MetaRigBoneType::RightMiddleFinger2;
	case "right_middle_finger3"_:
		return pragma::animation::MetaRigBoneType::RightMiddleFinger3;
	case "right_ring_finger1"_:
		return pragma::animation::MetaRigBoneType::RightRingFinger1;
	case "right_ring_finger2"_:
		return pragma::animation::MetaRigBoneType::RightRingFinger2;
	case "right_ring_finger3"_:
		return pragma::animation::MetaRigBoneType::RightRingFinger3;
	case "right_little_finger1"_:
		return pragma::animation::MetaRigBoneType::RightLittleFinger1;
	case "right_little_finger2"_:
		return pragma::animation::MetaRigBoneType::RightLittleFinger2;
	case "right_little_finger3"_:
		return pragma::animation::MetaRigBoneType::RightLittleFinger3;
	case "tail_base"_:
		return pragma::animation::MetaRigBoneType::TailBase;
	case "tail_middle"_:
		return pragma::animation::MetaRigBoneType::TailMiddle;
	case "tail_middle1"_:
		return pragma::animation::MetaRigBoneType::TailMiddle1;
	case "tail_tip"_:
		return pragma::animation::MetaRigBoneType::TailTip;
	case "left_ear"_:
		return pragma::animation::MetaRigBoneType::LeftEar;
	case "right_ear"_:
		return pragma::animation::MetaRigBoneType::RightEar;
	case "left_eye"_:
		return pragma::animation::MetaRigBoneType::LeftEye;
	case "right_eye"_:
		return pragma::animation::MetaRigBoneType::RightEye;
	case "center_eye"_:
		return pragma::animation::MetaRigBoneType::CenterEye;
	case "left_wing"_:
		return pragma::animation::MetaRigBoneType::LeftWing;
	case "left_wing_middle"_:
		return pragma::animation::MetaRigBoneType::LeftWingMiddle;
	case "left_wing_tip"_:
		return pragma::animation::MetaRigBoneType::LeftWingTip;
	case "right_wing"_:
		return pragma::animation::MetaRigBoneType::RightWing;
	case "right_wing_middle"_:
		return pragma::animation::MetaRigBoneType::RightWingMiddle;
	case "right_wing_tip"_:
		return pragma::animation::MetaRigBoneType::RightWingTip;
	}
	static_assert(umath::to_integral(MetaRigBoneType::Count) == 68, "Update this list when new types are added!");
	return {};
}

constexpr std::optional<pragma::animation::BoneSide> pragma::animation::get_meta_rig_bone_type_side(pragma::animation::MetaRigBoneType type)
{
	switch(type) {
	case MetaRigBoneType::Hips:
	case MetaRigBoneType::Pelvis:
	case MetaRigBoneType::Spine:
	case MetaRigBoneType::Spine1:
	case MetaRigBoneType::Spine2:
	case MetaRigBoneType::Spine3:
	case MetaRigBoneType::Neck:
	case MetaRigBoneType::Head:
	case MetaRigBoneType::Jaw:
	case MetaRigBoneType::CenterEye:
	case MetaRigBoneType::TailBase:
	case MetaRigBoneType::TailMiddle:
	case MetaRigBoneType::TailMiddle1:
	case MetaRigBoneType::TailTip:
		return {};
	case MetaRigBoneType::LeftEar:
	case MetaRigBoneType::LeftEye:
	case MetaRigBoneType::LeftUpperArm:
	case MetaRigBoneType::LeftLowerArm:
	case MetaRigBoneType::LeftHand:
	case MetaRigBoneType::LeftUpperLeg:
	case MetaRigBoneType::LeftLowerLeg:
	case MetaRigBoneType::LeftFoot:
	case MetaRigBoneType::LeftToe:
	case MetaRigBoneType::LeftThumb1:
	case MetaRigBoneType::LeftThumb2:
	case MetaRigBoneType::LeftThumb3:
	case MetaRigBoneType::LeftIndexFinger1:
	case MetaRigBoneType::LeftIndexFinger2:
	case MetaRigBoneType::LeftIndexFinger3:
	case MetaRigBoneType::LeftMiddleFinger1:
	case MetaRigBoneType::LeftMiddleFinger2:
	case MetaRigBoneType::LeftMiddleFinger3:
	case MetaRigBoneType::LeftRingFinger1:
	case MetaRigBoneType::LeftRingFinger2:
	case MetaRigBoneType::LeftRingFinger3:
	case MetaRigBoneType::LeftLittleFinger1:
	case MetaRigBoneType::LeftLittleFinger2:
	case MetaRigBoneType::LeftLittleFinger3:
	case MetaRigBoneType::LeftWing:
	case MetaRigBoneType::LeftWingMiddle:
	case MetaRigBoneType::LeftWingTip:
		return pragma::animation::BoneSide::Left;
	case MetaRigBoneType::RightEar:
	case MetaRigBoneType::RightEye:
	case MetaRigBoneType::RightUpperArm:
	case MetaRigBoneType::RightLowerArm:
	case MetaRigBoneType::RightHand:
	case MetaRigBoneType::RightUpperLeg:
	case MetaRigBoneType::RightLowerLeg:
	case MetaRigBoneType::RightFoot:
	case MetaRigBoneType::RightToe:
	case MetaRigBoneType::RightThumb1:
	case MetaRigBoneType::RightThumb2:
	case MetaRigBoneType::RightThumb3:
	case MetaRigBoneType::RightIndexFinger1:
	case MetaRigBoneType::RightIndexFinger2:
	case MetaRigBoneType::RightIndexFinger3:
	case MetaRigBoneType::RightMiddleFinger1:
	case MetaRigBoneType::RightMiddleFinger2:
	case MetaRigBoneType::RightMiddleFinger3:
	case MetaRigBoneType::RightRingFinger1:
	case MetaRigBoneType::RightRingFinger2:
	case MetaRigBoneType::RightRingFinger3:
	case MetaRigBoneType::RightLittleFinger1:
	case MetaRigBoneType::RightLittleFinger2:
	case MetaRigBoneType::RightLittleFinger3:
	case MetaRigBoneType::RightWing:
	case MetaRigBoneType::RightWingMiddle:
	case MetaRigBoneType::RightWingTip:
		return pragma::animation::BoneSide::Right;
	}
	static_assert(umath::to_integral(MetaRigBoneType::Count) == 68, "Update this list when new types are added!");
	return {};
}

constexpr std::optional<pragma::animation::MetaRigBoneType> pragma::animation::get_meta_rig_bone_parent_type(MetaRigBoneType type)
{
	switch(type) {
	case MetaRigBoneType::Pelvis:
	case MetaRigBoneType::Spine:
		return MetaRigBoneType::Hips;
	case MetaRigBoneType::Spine1:
		return MetaRigBoneType::Spine;
	case MetaRigBoneType::Spine2:
		return MetaRigBoneType::Spine1;
	case MetaRigBoneType::Spine3:
		return MetaRigBoneType::Spine2;
	case MetaRigBoneType::Neck:
		return MetaRigBoneType::Spine3;
	case MetaRigBoneType::Head:
		return MetaRigBoneType::Neck;
	case MetaRigBoneType::Jaw:
		return MetaRigBoneType::Head;
	case MetaRigBoneType::LeftEar:
		return MetaRigBoneType::Head;
	case MetaRigBoneType::RightEar:
		return MetaRigBoneType::Head;
	case MetaRigBoneType::LeftEye:
		return MetaRigBoneType::Head;
	case MetaRigBoneType::RightEye:
		return MetaRigBoneType::Head;
	case MetaRigBoneType::CenterEye:
		return MetaRigBoneType::Head;
	case MetaRigBoneType::LeftUpperArm:
		return MetaRigBoneType::Spine3;
	case MetaRigBoneType::LeftLowerArm:
		return MetaRigBoneType::LeftUpperArm;
	case MetaRigBoneType::LeftHand:
		return MetaRigBoneType::LeftLowerArm;
	case MetaRigBoneType::RightUpperArm:
		return MetaRigBoneType::Spine3;
	case MetaRigBoneType::RightLowerArm:
		return MetaRigBoneType::RightUpperArm;
	case MetaRigBoneType::RightHand:
		return MetaRigBoneType::RightLowerArm;
	case MetaRigBoneType::LeftUpperLeg:
		return MetaRigBoneType::Hips;
	case MetaRigBoneType::LeftLowerLeg:
		return MetaRigBoneType::LeftUpperLeg;
	case MetaRigBoneType::LeftFoot:
		return MetaRigBoneType::LeftLowerLeg;
	case MetaRigBoneType::LeftToe:
		return MetaRigBoneType::LeftFoot;
	case MetaRigBoneType::RightUpperLeg:
		return MetaRigBoneType::Hips;
	case MetaRigBoneType::RightLowerLeg:
		return MetaRigBoneType::RightUpperLeg;
	case MetaRigBoneType::RightFoot:
		return MetaRigBoneType::RightLowerLeg;
	case MetaRigBoneType::RightToe:
		return MetaRigBoneType::RightFoot;
	case MetaRigBoneType::LeftThumb1:
		return MetaRigBoneType::LeftHand;
	case MetaRigBoneType::LeftThumb2:
		return MetaRigBoneType::LeftThumb1;
	case MetaRigBoneType::LeftThumb3:
		return MetaRigBoneType::LeftThumb2;
	case MetaRigBoneType::LeftIndexFinger1:
		return MetaRigBoneType::LeftHand;
	case MetaRigBoneType::LeftIndexFinger2:
		return MetaRigBoneType::LeftIndexFinger1;
	case MetaRigBoneType::LeftIndexFinger3:
		return MetaRigBoneType::LeftIndexFinger2;
	case MetaRigBoneType::LeftMiddleFinger1:
		return MetaRigBoneType::LeftHand;
	case MetaRigBoneType::LeftMiddleFinger2:
		return MetaRigBoneType::LeftMiddleFinger1;
	case MetaRigBoneType::LeftMiddleFinger3:
		return MetaRigBoneType::LeftMiddleFinger2;
	case MetaRigBoneType::LeftRingFinger1:
		return MetaRigBoneType::LeftHand;
	case MetaRigBoneType::LeftRingFinger2:
		return MetaRigBoneType::LeftRingFinger1;
	case MetaRigBoneType::LeftRingFinger3:
		return MetaRigBoneType::LeftRingFinger2;
	case MetaRigBoneType::LeftLittleFinger1:
		return MetaRigBoneType::LeftHand;
	case MetaRigBoneType::LeftLittleFinger2:
		return MetaRigBoneType::LeftLittleFinger1;
	case MetaRigBoneType::LeftLittleFinger3:
		return MetaRigBoneType::LeftLittleFinger2;
	case MetaRigBoneType::RightThumb1:
		return MetaRigBoneType::RightHand;
	case MetaRigBoneType::RightThumb2:
		return MetaRigBoneType::RightThumb1;
	case MetaRigBoneType::RightThumb3:
		return MetaRigBoneType::RightThumb2;
	case MetaRigBoneType::RightIndexFinger1:
		return MetaRigBoneType::RightHand;
	case MetaRigBoneType::RightIndexFinger2:
		return MetaRigBoneType::RightIndexFinger1;
	case MetaRigBoneType::RightIndexFinger3:
		return MetaRigBoneType::RightIndexFinger2;
	case MetaRigBoneType::RightMiddleFinger1:
		return MetaRigBoneType::RightHand;
	case MetaRigBoneType::RightMiddleFinger2:
		return MetaRigBoneType::RightMiddleFinger1;
	case MetaRigBoneType::RightMiddleFinger3:
		return MetaRigBoneType::RightMiddleFinger2;
	case MetaRigBoneType::RightRingFinger1:
		return MetaRigBoneType::RightHand;
	case MetaRigBoneType::RightRingFinger2:
		return MetaRigBoneType::RightRingFinger1;
	case MetaRigBoneType::RightRingFinger3:
		return MetaRigBoneType::RightRingFinger2;
	case MetaRigBoneType::RightLittleFinger1:
		return MetaRigBoneType::RightHand;
	case MetaRigBoneType::RightLittleFinger2:
		return MetaRigBoneType::RightLittleFinger1;
	case MetaRigBoneType::RightLittleFinger3:
		return MetaRigBoneType::RightLittleFinger2;
	case MetaRigBoneType::TailBase:
		return MetaRigBoneType::Hips;
	case MetaRigBoneType::TailMiddle:
		return MetaRigBoneType::TailBase;
	case MetaRigBoneType::TailMiddle1:
		return MetaRigBoneType::TailMiddle;
	case MetaRigBoneType::TailTip:
		return MetaRigBoneType::TailMiddle1;
	case MetaRigBoneType::LeftWing:
		return MetaRigBoneType::Spine;
	case MetaRigBoneType::LeftWingMiddle:
		return MetaRigBoneType::LeftWing;
	case MetaRigBoneType::LeftWingTip:
		return MetaRigBoneType::LeftWingMiddle;
	case MetaRigBoneType::RightWing:
		return MetaRigBoneType::Spine;
	case MetaRigBoneType::RightWingMiddle:
		return MetaRigBoneType::RightWing;
	case MetaRigBoneType::RightWingTip:
		return MetaRigBoneType::RightWingMiddle;
	}
	static_assert(umath::to_integral(MetaRigBoneType::Count) == 68, "Update this list when new types are added!");
	return {};
}

constexpr const char *pragma::animation::get_blend_shape_name(BlendShape blendShape)
{
	switch(blendShape) {
	case BlendShape::Neutral:
		return "_neutral";
	case BlendShape::BrowDownLeft:
		return "browDownLeft";
	case BlendShape::BrowDownRight:
		return "browDownRight";
	case BlendShape::BrowInnerUp:
		return "browInnerUp";
	case BlendShape::BrowOuterUpLeft:
		return "browOuterUpLeft";
	case BlendShape::BrowOuterUpRight:
		return "browOuterUpRight";
	case BlendShape::CheekPuff:
		return "cheekPuff";
	case BlendShape::CheekSquintLeft:
		return "cheekSquintLeft";
	case BlendShape::CheekSquintRight:
		return "cheekSquintRight";
	case BlendShape::EyeBlinkLeft:
		return "eyeBlinkLeft";
	case BlendShape::EyeBlinkRight:
		return "eyeBlinkRight";
	case BlendShape::EyeLookDownLeft:
		return "eyeLookDownLeft";
	case BlendShape::EyeLookDownRight:
		return "eyeLookDownRight";
	case BlendShape::EyeLookInLeft:
		return "eyeLookInLeft";
	case BlendShape::EyeLookInRight:
		return "eyeLookInRight";
	case BlendShape::EyeLookOutLeft:
		return "eyeLookOutLeft";
	case BlendShape::EyeLookOutRight:
		return "eyeLookOutRight";
	case BlendShape::EyeLookUpLeft:
		return "eyeLookUpLeft";
	case BlendShape::EyeLookUpRight:
		return "eyeLookUpRight";
	case BlendShape::EyeSquintLeft:
		return "eyeSquintLeft";
	case BlendShape::EyeSquintRight:
		return "eyeSquintRight";
	case BlendShape::EyeWideLeft:
		return "eyeWideLeft";
	case BlendShape::EyeWideRight:
		return "eyeWideRight";
	case BlendShape::JawForward:
		return "jawForward";
	case BlendShape::JawLeft:
		return "jawLeft";
	case BlendShape::JawOpen:
		return "jawOpen";
	case BlendShape::JawRight:
		return "jawRight";
	case BlendShape::MouthClose:
		return "mouthClose";
	case BlendShape::MouthDimpleLeft:
		return "mouthDimpleLeft";
	case BlendShape::MouthDimpleRight:
		return "mouthDimpleRight";
	case BlendShape::MouthFrownLeft:
		return "mouthFrownLeft";
	case BlendShape::MouthFrownRight:
		return "mouthFrownRight";
	case BlendShape::MouthFunnel:
		return "mouthFunnel";
	case BlendShape::MouthLeft:
		return "mouthLeft";
	case BlendShape::MouthLowerDownLeft:
		return "mouthLowerDownLeft";
	case BlendShape::MouthLowerDownRight:
		return "mouthLowerDownRight";
	case BlendShape::MouthPressLeft:
		return "mouthPressLeft";
	case BlendShape::MouthPressRight:
		return "mouthPressRight";
	case BlendShape::MouthPucker:
		return "mouthPucker";
	case BlendShape::MouthRight:
		return "mouthRight";
	case BlendShape::MouthRollLower:
		return "mouthRollLower";
	case BlendShape::MouthRollUpper:
		return "mouthRollUpper";
	case BlendShape::MouthShrugLower:
		return "mouthShrugLower";
	case BlendShape::MouthShrugUpper:
		return "mouthShrugUpper";
	case BlendShape::MouthSmileLeft:
		return "mouthSmileLeft";
	case BlendShape::MouthSmileRight:
		return "mouthSmileRight";
	case BlendShape::MouthStretchLeft:
		return "mouthStretchLeft";
	case BlendShape::MouthStretchRight:
		return "mouthStretchRight";
	case BlendShape::MouthUpperUpLeft:
		return "mouthUpperUpLeft";
	case BlendShape::MouthUpperUpRight:
		return "mouthUpperUpRight";
	case BlendShape::NoseSneerLeft:
		return "noseSneerLeft";
	case BlendShape::NoseSneerRight:
		return "noseSneerRight";
	case BlendShape::TongueOut:
		return "tongueOut";
	case BlendShape::EyeDilationLeft:
		return "eyeDilationLeft";
	case BlendShape::EyeDilationRight:
		return "eyeDilationRight";
	case BlendShape::EyeConstrictLeft:
		return "eyeConstrictLeft";
	case BlendShape::EyeConstrictRight:
		return "eyeConstrictRight";
	case BlendShape::CheekSuckLeft:
		return "cheekSuckLeft";
	case BlendShape::CheekSuckRight:
		return "cheekSuckRight";
	case BlendShape::MouthTightenerLeft:
		return "mouthTightenerLeft";
	case BlendShape::MouthTightenerRight:
		return "mouthTightenerRight";
	default:
		return "";
	}
	static_assert(umath::to_integral(BlendShape::Count) == 61, "Update this list when new blend shape types are added!");
}
constexpr std::optional<pragma::animation::BlendShape> pragma::animation::get_blend_shape_enum(const std::string_view &name)
{
	using namespace ustring::string_switch_ci;
	switch(ustring::string_switch_ci::hash(name)) {
	case "_neutral"_:
		return BlendShape::Neutral;
	case "browdownleft"_:
		return BlendShape::BrowDownLeft;
	case "browdownright"_:
		return BlendShape::BrowDownRight;
	case "browinnerup"_:
		return BlendShape::BrowInnerUp;
	case "browouterupleft"_:
		return BlendShape::BrowOuterUpLeft;
	case "browouterupright"_:
		return BlendShape::BrowOuterUpRight;
	case "cheekpuff"_:
		return BlendShape::CheekPuff;
	case "cheeksquintleft"_:
		return BlendShape::CheekSquintLeft;
	case "cheeksquintright"_:
		return BlendShape::CheekSquintRight;
	case "eyeblinkleft"_:
		return BlendShape::EyeBlinkLeft;
	case "eyeblinkright"_:
		return BlendShape::EyeBlinkRight;
	case "eyelookdownleft"_:
		return BlendShape::EyeLookDownLeft;
	case "eyelookdownright"_:
		return BlendShape::EyeLookDownRight;
	case "eyelookinleft"_:
		return BlendShape::EyeLookInLeft;
	case "eyelookinright"_:
		return BlendShape::EyeLookInRight;
	case "eyelookoutleft"_:
		return BlendShape::EyeLookOutLeft;
	case "eyelookoutright"_:
		return BlendShape::EyeLookOutRight;
	case "eyelookupleft"_:
		return BlendShape::EyeLookUpLeft;
	case "eyelookupright"_:
		return BlendShape::EyeLookUpRight;
	case "eyesquintleft"_:
		return BlendShape::EyeSquintLeft;
	case "eyesquintright"_:
		return BlendShape::EyeSquintRight;
	case "eyewideleft"_:
		return BlendShape::EyeWideLeft;
	case "eyewideright"_:
		return BlendShape::EyeWideRight;
	case "jawforward"_:
		return BlendShape::JawForward;
	case "jawleft"_:
		return BlendShape::JawLeft;
	case "jawopen"_:
		return BlendShape::JawOpen;
	case "jawright"_:
		return BlendShape::JawRight;
	case "mouthclose"_:
		return BlendShape::MouthClose;
	case "mouthdimpleleft"_:
		return BlendShape::MouthDimpleLeft;
	case "mouthdimpleright"_:
		return BlendShape::MouthDimpleRight;
	case "mouthfrownleft"_:
		return BlendShape::MouthFrownLeft;
	case "mouthfrownright"_:
		return BlendShape::MouthFrownRight;
	case "mouthfunnel"_:
		return BlendShape::MouthFunnel;
	case "mouthleft"_:
		return BlendShape::MouthLeft;
	case "mouthlowerdownleft"_:
		return BlendShape::MouthLowerDownLeft;
	case "mouthlowerdownright"_:
		return BlendShape::MouthLowerDownRight;
	case "mouthpressleft"_:
		return BlendShape::MouthPressLeft;
	case "mouthpressright"_:
		return BlendShape::MouthPressRight;
	case "mouthpucker"_:
		return BlendShape::MouthPucker;
	case "mouthright"_:
		return BlendShape::MouthRight;
	case "mouthrolllower"_:
		return BlendShape::MouthRollLower;
	case "mouthrollupper"_:
		return BlendShape::MouthRollUpper;
	case "mouthshruglower"_:
		return BlendShape::MouthShrugLower;
	case "mouthshrugupper"_:
		return BlendShape::MouthShrugUpper;
	case "mouthsmileleft"_:
		return BlendShape::MouthSmileLeft;
	case "mouthsmileright"_:
		return BlendShape::MouthSmileRight;
	case "mouthstretchleft"_:
		return BlendShape::MouthStretchLeft;
	case "mouthstretchright"_:
		return BlendShape::MouthStretchRight;
	case "mouthupperupleft"_:
		return BlendShape::MouthUpperUpLeft;
	case "mouthupperupright"_:
		return BlendShape::MouthUpperUpRight;
	case "nosesneerleft"_:
		return BlendShape::NoseSneerLeft;
	case "nosesneerright"_:
		return BlendShape::NoseSneerRight;
	}
	static_assert(umath::to_integral(BlendShape::Count) == 61, "Update this list when new blend shape types are added!");
	return {};
}

#endif
