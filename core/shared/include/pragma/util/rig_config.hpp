/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __IK_RIG_CONFIG_HPP__
#define __IK_RIG_CONFIG_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <cinttypes>
#include <optional>
#include <mathutil/umath.h>
#include <mathutil/eulerangles.h>
#include <mathutil/transform.hpp>
#include <udm_types.hpp>
#include "pragma/game/game_coordinate_system.hpp"
#include "pragma/util/global_string_table.hpp"

namespace pragma::ik {
	// We'll assume 73 units to be roughly the size of a human and use that as a reference, i.e. a scale of 1.0 represents 73 game units
	constexpr double REFERENCE_HUMAN_UNIT_SIZE = 73.0;
	// This is the size of the ik rig of the reference model (test/ik_reference) divided by REFERENCE_HUMAN_UNIT_SIZE.
	// The default radius/length of the standard meta rig bones were determined using this model, so this scaling factor can be used to
	// apply them to rigs of different sizes.
	constexpr double REFERENCE_META_RIG_SCALE = 0.0782465711;

	struct DLLNETWORK RigConfigBone {
		pragma::GString name;
		bool locked = false;
		// TODO: What units are these? Meters?
		float length = 1.f;
		float radius = 0.25f;

		// If not set, bone pose will be used instead
		std::optional<umath::Transform> ikPose {};
	};

	struct DLLNETWORK RigConfigControl {
		enum class Type : uint8_t {
			Drag = 0,
			State,
			OrientedDrag,
			Revolute,
			Axis,
			Count,
		};
		pragma::GString bone;
		Type type = Type::Drag;
		float maxForce = -1.f;
		float rigidity = 1.f;
	};

	struct DLLNETWORK RigConfigConstraint {
		enum class Type : uint8_t {
			Fixed = 0,
			Hinge,
			BallSocket,
			Count,
		};
		pragma::GString bone0;
		pragma::GString bone1;
		Type type = Type::Fixed;
		SignedAxis axis = SignedAxis::Z;
		EulerAngles minLimits;
		EulerAngles maxLimits;
		umath::ScaledTransform offsetPose {};

		float rigidity = 1'000.f;
		float maxForce = -1.f;
	};

	struct DLLNETWORK RigConfigJoint {
		enum class Type : uint8_t {
			BallSocketJoint = 0,
			SwingLimit,
			TwistLimit,
			SwivelHingeJoint,
			TwistJoint,
			AngularJoint,
			DistanceJoint,
			ParentJoint,

			Count,
		};
		pragma::GString bone0;
		pragma::GString bone1;
		Type type = Type::BallSocketJoint;
		std::optional<Vector3> axisA {};
		std::optional<Vector3> axisB {};
		std::optional<umath::Degree> maxAngle {};
		float rigidity = 1.f;
		union {
			std::optional<Vector3> anchorPosition {};
			std::optional<Vector3> anchorPositionA;
		};
		std::optional<Vector3> anchorPositionB {};
		std::optional<Vector3> measurementAxisA {};
	};

	using PRigConfigBone = std::shared_ptr<RigConfigBone>;
	using PRigConfigControl = std::shared_ptr<RigConfigControl>;
	using PRigConfigConstraint = std::shared_ptr<RigConfigConstraint>;
	using PRigConfigJoint = std::shared_ptr<RigConfigJoint>;
	class DLLNETWORK RigConfig {
	  public:
		static constexpr auto PIKR_EXTENSION_BINARY = "pikr_b";
		static constexpr auto PIKR_EXTENSION_ASCII = "pikr";

		static std::optional<RigConfig> load(const std::string &fileName);
		static std::optional<RigConfig> load_from_udm_data(udm::LinkedPropertyWrapper &prop);
		static const std::vector<std::string> &get_supported_extensions();

		RigConfig();
		void DebugPrint() const;
		void ToUdmData(udm::LinkedPropertyWrapper &udmData) const;

		PRigConfigBone AddBone(const pragma::GString &name);
		PRigConfigBone FindBone(const pragma::GString &name);
		void RemoveBone(const pragma::GString &name);
		bool HasBone(const pragma::GString &name) const;
		bool IsBoneLocked(const pragma::GString &name) const;
		void SetBoneLocked(const pragma::GString &name, bool locked);

		void RemoveControl(const pragma::GString &name);
		bool HasControl(const pragma::GString &name) const;

		PRigConfigControl AddControl(const pragma::GString &bone, RigConfigControl::Type type, float rigidity = 1.f);

		void RemoveJoints(const pragma::GString &bone);
		void RemoveJoints(const pragma::GString &bone0, const pragma::GString &bone1);
		void RemoveJoint(const RigConfigJoint &joint);
		PRigConfigJoint AddBallSocketJoint(const pragma::GString &bone0, const pragma::GString &bone1, const Vector3 &anchorPosition, float rigidity = 1.f);
		PRigConfigJoint AddSwingLimit(const pragma::GString &bone0, const pragma::GString &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Degree maxAngle, float rigidity = 1.f);
		PRigConfigJoint AddTwistLimit(const pragma::GString &bone0, const pragma::GString &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Degree maxAngle, float rigidity = 1.f, const std::optional<Vector3> &measurementAxisA = {});
		PRigConfigJoint AddSwivelHingeJoint(const pragma::GString &bone0, const pragma::GString &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity = 1.f);
		PRigConfigJoint AddTwistJoint(const pragma::GString &bone0, const pragma::GString &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity);
		PRigConfigJoint AddDistanceJoint(const pragma::GString &bone0, const pragma::GString &bone1, float rigidity = 1.f);
		PRigConfigJoint AddAngularJoint(const pragma::GString &bone0, const pragma::GString &bone1, float rigidity = 1.f);
		PRigConfigJoint AddParentJoint(const pragma::GString &bone0, const pragma::GString &bone1);

		void RemoveConstraints(const pragma::GString &bone);
		void RemoveConstraints(const pragma::GString &bone0, const pragma::GString &bone1);
		void RemoveConstraint(const RigConfigConstraint &constraint);
		void RemoveControl(const RigConfigControl &control);
		void RemoveBone(const RigConfigBone &bone);
		PRigConfigConstraint AddFixedConstraint(const pragma::GString &bone0, const pragma::GString &bone1);
		PRigConfigConstraint AddHingeConstraint(const pragma::GString &bone0, const pragma::GString &bone1, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation = uquat::identity());
		PRigConfigConstraint AddBallSocketConstraint(const pragma::GString &bone0, const pragma::GString &bone1, const EulerAngles &minAngles, const EulerAngles &maxAngles, SignedAxis axis = SignedAxis::Z);

		const std::vector<PRigConfigBone> &GetBones() const { return m_bones; }
		const std::vector<PRigConfigControl> &GetControls() const { return m_controls; }
		const std::vector<PRigConfigConstraint> &GetConstraints() const { return m_constraints; }
		const std::vector<PRigConfigJoint> &GetJoints() const { return m_joints; }

		void SetRootBone(const std::string &rootBone) { m_rootBone = rootBone; }
		void ClearRootBone() { m_rootBone = {}; }
		const std::optional<std::string> &GetRootBone() const { return m_rootBone; }

		float CalcScaleFactor() const;

		bool Save(const std::string &fileName);
	  private:
		std::vector<PRigConfigBone>::iterator FindBoneIt(const pragma::GString &name);
		const std::vector<PRigConfigBone>::iterator FindBoneIt(const pragma::GString &name) const;

		std::vector<PRigConfigControl>::iterator FindControlIt(const pragma::GString &name);
		const std::vector<PRigConfigControl>::iterator FindControlIt(const pragma::GString &name) const;

		std::optional<std::string> m_rootBone {};
		std::vector<PRigConfigBone> m_bones;
		std::vector<PRigConfigControl> m_controls;
		std::vector<PRigConfigConstraint> m_constraints;
		std::vector<PRigConfigJoint> m_joints;
	};
};
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfig &config);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigBone &bone);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigControl &control);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigConstraint &constraint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigJoint &joint);

#endif
