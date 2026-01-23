// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:game.rig_config;

export import :game.coordinate_system;
export import :util.global_string_table;
export import pragma.udm;

export {
	namespace pragma::ik {
		// We'll assume 73 units to be roughly the size of a human and use that as a reference, i.e. a scale of 1.0 represents 73 game units
		constexpr double REFERENCE_HUMAN_UNIT_SIZE = 73.0;
		// This is the size of the ik rig of the reference model (test/ik_reference) divided by REFERENCE_HUMAN_UNIT_SIZE.
		// The default radius/length of the standard meta rig bones were determined using this model, so this scaling factor can be used to
		// apply them to rigs of different sizes.
		constexpr double REFERENCE_META_RIG_SCALE = 0.0782465711;

		struct DLLNETWORK RigConfigBone {
			GString name;
			bool locked = false;
			// TODO: What units are these? Meters?
			float length = 1.f;
			float radius = 0.25f;

			// If not set, bone pose will be used instead
			std::optional<math::Transform> ikPose {};
		};

		struct DLLNETWORK RigConfigControl {
			enum class Type : uint8_t {
				Drag = 0,
				AngularPlane,
				State,
				OrientedDrag,
				Revolute,
				Axis,
				PoleTarget,
				Count,
			};
			GString bone;
			Type type = Type::Drag;
			float maxForce = -1.f;
			float rigidity = 1.f;
			std::optional<Vector3> initialPos {};
			std::optional<Quat> initialRot {};

			GString poleTargetBaseBone;
			GString poleTargetEffectorBone;
			math::Degree poleAngle = 0.f;
		};

		struct DLLNETWORK RigConfigConstraint {
			enum class Type : uint8_t {
				Fixed = 0,
				Hinge,
				BallSocket,
				Count,
			};
			GString bone0;
			GString bone1;
			Type type = Type::Fixed;
			SignedAxis axis = SignedAxis::Z;
			EulerAngles minLimits;
			EulerAngles maxLimits;
			math::ScaledTransform offsetPose {};

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
			GString bone0;
			GString bone1;
			Type type = Type::BallSocketJoint;
			std::optional<Vector3> axisA {};
			std::optional<Vector3> axisB {};
			std::optional<math::Degree> maxAngle {};
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

			PRigConfigBone AddBone(const GString &name);
			PRigConfigBone FindBone(const GString &name);
			void RemoveBone(const GString &name);
			bool HasBone(const GString &name) const;
			bool IsBoneLocked(const GString &name) const;
			void SetBoneLocked(const GString &name, bool locked);

			void RemoveControl(const GString &name);
			bool HasControl(const GString &name) const;

			PRigConfigControl AddControl(const GString &bone, RigConfigControl::Type type, float rigidity = 1.f);

			void RemoveJoints(const GString &bone);
			void RemoveJoints(const GString &bone0, const GString &bone1);
			void RemoveJoint(const RigConfigJoint &joint);
			PRigConfigJoint AddBallSocketJoint(const GString &bone0, const GString &bone1, const Vector3 &anchorPosition, float rigidity = 1.f);
			PRigConfigJoint AddSwingLimit(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, math::Degree maxAngle, float rigidity = 1.f);
			PRigConfigJoint AddTwistLimit(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, math::Degree maxAngle, float rigidity = 1.f, const std::optional<Vector3> &measurementAxisA = {});
			PRigConfigJoint AddSwivelHingeJoint(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity = 1.f);
			PRigConfigJoint AddTwistJoint(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity);
			PRigConfigJoint AddDistanceJoint(const GString &bone0, const GString &bone1, float rigidity = 1.f);
			PRigConfigJoint AddAngularJoint(const GString &bone0, const GString &bone1, float rigidity = 1.f);
			PRigConfigJoint AddParentJoint(const GString &bone0, const GString &bone1);

			void RemoveConstraints(const GString &bone);
			void RemoveConstraints(const GString &bone0, const GString &bone1);
			void RemoveConstraint(const RigConfigConstraint &constraint);
			void RemoveControl(const RigConfigControl &control);
			void RemoveBone(const RigConfigBone &bone);
			PRigConfigConstraint AddFixedConstraint(const GString &bone0, const GString &bone1);
			PRigConfigConstraint AddHingeConstraint(const GString &bone0, const GString &bone1, math::Degree minAngle, math::Degree maxAngle, const Quat &offsetRotation = uquat::identity());
			PRigConfigConstraint AddBallSocketConstraint(const GString &bone0, const GString &bone1, const EulerAngles &minAngles, const EulerAngles &maxAngles, SignedAxis axis = SignedAxis::Z);

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
			std::vector<PRigConfigBone>::iterator FindBoneIt(const GString &name);
			const std::vector<PRigConfigBone>::iterator FindBoneIt(const GString &name) const;

			std::vector<PRigConfigControl>::iterator FindControlIt(const GString &name);
			const std::vector<PRigConfigControl>::iterator FindControlIt(const GString &name) const;

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
};
