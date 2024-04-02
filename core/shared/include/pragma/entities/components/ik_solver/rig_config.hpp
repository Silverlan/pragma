/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __IK_RIG_CONFIG_HPP__
#define __IK_RIG_CONFIG_HPP__

#include <string>
#include <cinttypes>
#include <optional>
#include <mathutil/umath.h>
#include <mathutil/eulerangles.h>
#include <udm_types.hpp>
#include "pragma/game/game_coordinate_system.hpp"

namespace pragma::ik {
	struct RigConfigBone {
		std::string name;
		bool locked = false;
	};

	struct RigConfigControl {
		enum class Type : uint8_t {
			Drag = 0,
			State,
			OrientedDrag,
			Count,
		};
		std::string bone;
		Type type = Type::Drag;
		float maxForce = -1.f;
		float rigidity = 1.f;
	};

	struct RigConfigConstraint {
		enum class Type : uint8_t {
			Fixed = 0,
			Hinge,
			BallSocket,
			Count,
		};
		std::string bone0;
		std::string bone1;
		Type type = Type::Fixed;
		SignedAxis axis = SignedAxis::Z;
		EulerAngles minLimits;
		EulerAngles maxLimits;
		umath::ScaledTransform offsetPose {};

		float rigidity = 1'000.f;
		float maxForce = -1.f;
	};

	struct RigConfigJoint {
		enum class Type : uint8_t {
			BallSocketJoint = 0,
			SwingLimit,
			TwistLimit,
			SwivelHingeJoint,
			TwistJoint,

			Count,
		};
		std::string bone0;
		std::string bone1;
		Type type = Type::BallSocketJoint;
		Vector3 axisA;
		Vector3 axisB;
		Vector3 anchorPosition {};
		umath::Degree maxAngle = 0;
		float rigidity = 1.f;
	};

	using PRigConfigBone = std::shared_ptr<RigConfigBone>;
	using PRigConfigControl = std::shared_ptr<RigConfigControl>;
	using PRigConfigConstraint = std::shared_ptr<RigConfigConstraint>;
	using PRigConfigJoint = std::shared_ptr<RigConfigJoint>;
	class RigConfig {
	  public:
		static std::optional<RigConfig> load(const std::string &fileName);
		static std::optional<RigConfig> load_from_udm_data(udm::LinkedPropertyWrapper &prop);
		static const std::vector<std::string> &get_supported_extensions();

		RigConfig();
		void DebugPrint() const;
		void ToUdmData(udm::LinkedPropertyWrapper &udmData) const;

		PRigConfigBone AddBone(const std::string &name);
		PRigConfigBone FindBone(const std::string &name);
		void RemoveBone(const std::string &name);
		bool HasBone(const std::string &name) const;
		bool IsBoneLocked(const std::string &name) const;
		void SetBoneLocked(const std::string &name, bool locked);

		void RemoveControl(const std::string &name);
		bool HasControl(const std::string &name) const;

		PRigConfigControl AddControl(const std::string &bone, RigConfigControl::Type type, float rigidity = 1.f);

		void RemoveJoints(const std::string &bone);
		void RemoveJoints(const std::string &bone0, const std::string &bone1);
		void RemoveJoint(const RigConfigJoint &joint);
		PRigConfigJoint AddBallSocketJoint(const std::string &bone0, const std::string &bone1);
		PRigConfigJoint AddSwingLimit(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Degree maxAngle);
		PRigConfigJoint AddTwistLimit(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Degree maxAngle, float rigidity);
		PRigConfigJoint AddSwivelHingeJoint(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB);
		PRigConfigJoint AddTwistJoint(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity);

		void RemoveConstraints(const std::string &bone);
		void RemoveConstraints(const std::string &bone0, const std::string &bone1);
		void RemoveConstraint(const RigConfigConstraint &constraint);
		void RemoveControl(const RigConfigControl &control);
		void RemoveBone(const RigConfigBone &bone);
		PRigConfigConstraint AddFixedConstraint(const std::string &bone0, const std::string &bone1);
		PRigConfigConstraint AddHingeConstraint(const std::string &bone0, const std::string &bone1, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation = uquat::identity());
		PRigConfigConstraint AddBallSocketConstraint(const std::string &bone0, const std::string &bone1, const EulerAngles &minAngles, const EulerAngles &maxAngles, SignedAxis axis = SignedAxis::Z);

		const std::vector<PRigConfigBone> &GetBones() const { return m_bones; }
		const std::vector<PRigConfigControl> &GetControls() const { return m_controls; }
		const std::vector<PRigConfigConstraint> &GetConstraints() const { return m_constraints; }
		const std::vector<PRigConfigJoint> &GetJoints() const { return m_joints; }

		bool Save(const std::string &fileName);
	  private:
		std::vector<PRigConfigBone>::iterator FindBoneIt(const std::string &name);
		const std::vector<PRigConfigBone>::iterator FindBoneIt(const std::string &name) const;

		std::vector<PRigConfigControl>::iterator FindControlIt(const std::string &name);
		const std::vector<PRigConfigControl>::iterator FindControlIt(const std::string &name) const;

		std::vector<PRigConfigBone> m_bones;
		std::vector<PRigConfigControl> m_controls;
		std::vector<PRigConfigConstraint> m_constraints;
		std::vector<PRigConfigJoint> m_joints;
	};
};
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfig &config);
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigBone &bone);
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigControl &control);
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigConstraint &constraint);
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigJoint &joint);

#endif
