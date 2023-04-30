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
		enum class Type : uint8_t { Drag = 0, State };
		std::string bone;
		Type type = Type::Drag;
	};

	struct RigConfigConstraint {
		enum class Type : uint8_t { Fixed = 0, Hinge, BallSocket };
		std::string bone0;
		std::string bone1;
		Type type = Type::Fixed;
		Axis axis = Axis::Z;
		EulerAngles minLimits;
		EulerAngles maxLimits;
		umath::ScaledTransform offsetPose {};
	};

	using PRigConfigBone = std::shared_ptr<RigConfigBone>;
	using PRigConfigControl = std::shared_ptr<RigConfigControl>;
	using PRigConfigConstraint = std::shared_ptr<RigConfigConstraint>;
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

		PRigConfigControl AddControl(const std::string &bone, RigConfigControl::Type type);

		void RemoveConstraints(const std::string &bone);
		void RemoveConstraints(const std::string &bone0, const std::string &bone1);
		void RemoveConstraint(const RigConfigConstraint &constraint);
		void RemoveControl(const RigConfigControl &control);
		void RemoveBone(const RigConfigBone &bone);
		PRigConfigConstraint AddFixedConstraint(const std::string &bone0, const std::string &bone1);
		PRigConfigConstraint AddHingeConstraint(const std::string &bone0, const std::string &bone1, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation = uquat::identity());
		PRigConfigConstraint AddBallSocketConstraint(const std::string &bone0, const std::string &bone1, const EulerAngles &minAngles, const EulerAngles &maxAngles, Axis axis = Axis::Z);

		const std::vector<PRigConfigBone> &GetBones() const { return m_bones; }
		const std::vector<PRigConfigControl> &GetControls() const { return m_controls; }
		const std::vector<PRigConfigConstraint> &GetConstraints() const { return m_constraints; }

		bool Save(const std::string &fileName);
	  private:
		std::vector<PRigConfigBone>::iterator FindBoneIt(const std::string &name);
		const std::vector<PRigConfigBone>::iterator FindBoneIt(const std::string &name) const;

		std::vector<PRigConfigControl>::iterator FindControlIt(const std::string &name);
		const std::vector<PRigConfigControl>::iterator FindControlIt(const std::string &name) const;

		std::vector<PRigConfigBone> m_bones;
		std::vector<PRigConfigControl> m_controls;
		std::vector<PRigConfigConstraint> m_constraints;
	};
};
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfig &config);

#endif
