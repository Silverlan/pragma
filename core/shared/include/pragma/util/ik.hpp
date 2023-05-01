/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_UTIL_IK_HPP__
#define __PRAGMA_UTIL_IK_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>
#include <mathutil/uvec.h>
#include <mathutil/uquat.h>

namespace BEPUik {
	class IKSolver;
	class Bone;
	class Control;
	class SingleBoneLinearMotor;
	class SingleBoneAngularPlaneConstraint;
	class IKJoint;
};

namespace pragma::ik {
	using BoneId = uint32_t;
	class DLLNETWORK Bone {
	  public:
		Bone(const Vector3 &pos, const Quat &rot, float radius, float length, float mass);
		~Bone();
		bool operator==(const Bone &other) const;
		bool operator!=(const Bone &other) const { return !operator==(other); }
		Vector3 GetPos() const;
		Quat GetRot() const;
		void SetPos(const Vector3 &pos) const;
		void SetRot(const Quat &rot) const;
		void SetPinned(bool pinned);
		bool IsPinned();
		void SetName(const std::string &name);
		const std::string &GetName() const;
		float GetRadius() const;
		float GetLength() const;
		float GetMass() const;
		const umath::Transform &GetOriginalPose() const;
		BEPUik::Bone *operator*();
		BEPUik::Bone *operator->();
	  private:
		std::unique_ptr<BEPUik::Bone> m_bone;
		std::string m_name;
		umath::Transform m_origPose;
	};
	class DLLNETWORK IControl {
	  public:
		virtual ~IControl();
		BEPUik::Control *operator*();
		void SetTargetBone(Bone &bone);
		Bone *GetTargetBone();
		const Bone *GetTargetBone() const;
	  protected:
		IControl();
		std::unique_ptr<BEPUik::Control> m_control;

		Bone *m_bone = nullptr;
	};
	class DLLNETWORK ILinearMotorControl {
	  public:
		virtual BEPUik::SingleBoneLinearMotor &GetLinearMotor() = 0;
		const BEPUik::SingleBoneLinearMotor &GetLinearMotor() const;
		void SetTargetPosition(const Vector3 &pos);
		Vector3 GetTargetPosition() const;
		void SetOffset(const Vector3 &offset);
		Vector3 GetOffset() const;
	};
	class DLLNETWORK DragControl : public IControl, public ILinearMotorControl {
	  public:
		DragControl(Bone &bone);
		~DragControl();
		virtual BEPUik::SingleBoneLinearMotor &GetLinearMotor() override;
	};
	class DLLNETWORK AngularPlaneControl : public IControl {
	  public:
		AngularPlaneControl(Bone &bone);
		~AngularPlaneControl();
		void SetPlaneNormal(const Vector3 &n);
		Vector3 GetPlaneNormal() const;
		void SetBoneLocalAxis(const Vector3 &n);
		Vector3 GetBoneLocalAxis() const;

		BEPUik::SingleBoneAngularPlaneConstraint &GetAngularMotor();
		const BEPUik::SingleBoneAngularPlaneConstraint &GetAngularMotor() const;
	};
	class DLLNETWORK StateControl : public IControl, public ILinearMotorControl {
	  public:
		StateControl(Bone &bone);
		~StateControl();
		void SetTargetOrientation(const Quat &rot);
		Quat GetTargetOrientation() const;
		virtual BEPUik::SingleBoneLinearMotor &GetLinearMotor() override;
	};
	enum class JointType : uint8_t {
		DistanceJoint = 0,
		BallSocketJoint,
		AngularJoint,
		PointOnLineJoint,
		RevoluteJoint,
		SwingLimit,
		EllipseSwingLimit,
		LinearAxisLimit,
		TwistJoint,
		TwistLimit,
		SwivelHingeJoint,
		Count,

		Invalid = std::numeric_limits<uint8_t>::max()
	};
	class DLLNETWORK IJoint {
	  public:
		IJoint(JointType type);
		virtual ~IJoint();

		void SetRigidity(float rigidity);
		float GetRigidity();

		Bone &GetConnectionA();
		const Bone &GetConnectionA() const;
		Bone &GetConnectionB();
		const Bone &GetConnectionB() const;

		JointType GetJointType() const { return m_jointType; }

		BEPUik::IKJoint *operator*();
		const BEPUik::IKJoint *operator*() const { return const_cast<IJoint *>(this)->operator*(); }
		BEPUik::IKJoint *operator->();
		const BEPUik::IKJoint *operator->() const { return const_cast<IJoint *>(this)->operator->(); }
	  protected:
		IJoint();
		void SetJoint(std::unique_ptr<BEPUik::IKJoint> joint, Bone &connectionA, Bone &connectionB);
		std::unique_ptr<BEPUik::IKJoint> m_joint;
		Bone *m_connectionA = nullptr;
		Bone *m_connectionB = nullptr;
		JointType m_jointType = JointType::Invalid;
	};
	class DLLNETWORK DistanceJoint : public IJoint {
	  public:
		DistanceJoint(Bone &bone0, Bone &bone1);
		~DistanceJoint();
	};
	class DLLNETWORK PointOnLineJoint : public IJoint {
	  public:
		PointOnLineJoint(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB);
		~PointOnLineJoint();
		const Vector3 &GetLineAnchor() const;
		const Vector3 &GetLineDirection() const;
		const Vector3 &GetAnchorB() const;
	  private:
		Vector3 m_lineAnchor;
		Vector3 m_lineDirection;
		Vector3 m_anchorB;
	};
	class DLLNETWORK BallSocketJoint : public IJoint {
	  public:
		BallSocketJoint(Bone &bone0, Bone &bone1, const Vector3 &anchor);
		~BallSocketJoint();

		const Vector3 &GetAnchor() const;
		Vector3 GetOffsetA() const;
		Vector3 GetOffsetB() const;
	  private:
		Vector3 m_anchor;
	};
	class DLLNETWORK AngularJoint : public IJoint {
	  public:
		AngularJoint(Bone &bone0, Bone &bone1);
		~AngularJoint();
	};
	class DLLNETWORK RevoluteJoint : public IJoint {
	  public:
		RevoluteJoint(Bone &bone0, Bone &bone1, const Vector3 &freeAxis);
		~RevoluteJoint();
		const Vector3 &GetFreeAxis() const;
	  private:
		Vector3 m_freeAxis;
	};
	class DLLNETWORK TwistJoint : public IJoint {
	  public:
		TwistJoint(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB);
		~TwistJoint();
		const Vector3 &GetAxisA() const;
		const Vector3 &GetAxisB() const;

		void SetAxisA(const Vector3 &axisA);
		void SetAxisB(const Vector3 &axisB);
	  private:
		Vector3 m_axisA;
		Vector3 m_axisB;
	};
	class DLLNETWORK SwingLimit : public IJoint {
	  public:
		SwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Radian maxAngle);
		~SwingLimit();

		Vector3 GetAxisA() const;
		Vector3 GetAxisB() const;
		umath::Radian GetMaxAngle() const;

		void SetAxisA(const Vector3 &axisA);
		void SetAxisB(const Vector3 &axisB);
		void SetMaxAngle(umath::Radian maxAngle);
	  private:
		umath::Radian m_maxAngle = 0.f;
	};
	class DLLNETWORK EllipseSwingLimit : public IJoint {
	  public:
		EllipseSwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, const Vector3 &xAxis, umath::Radian maxAngleX, umath::Radian maxAngleY);
		~EllipseSwingLimit();
		const Vector3 &GetAxisA() const;
		const Vector3 &GetAxisB() const;
		const Vector3 &GetXAxis() const;
		umath::Radian GetMaxAngleX() const;
		umath::Radian GetMaxAngleY() const;
	  private:
		Vector3 m_axisA;
		Vector3 m_axisB;
		Vector3 m_xAxis;
		umath::Radian m_maxAngleX = 0.f;
		umath::Radian m_maxAngleY = 0.f;
	};
	class DLLNETWORK LinearAxisLimit : public IJoint {
	  public:
		LinearAxisLimit(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB, float minimumDistance, float maximumDistance);
		~LinearAxisLimit();
		const Vector3 &GetLineAnchor() const;
		const Vector3 &GetLineDirection() const;
		const Vector3 &GetAnchorB() const;
		float GetMinimumDistance() const;
		float GetMaximumDistance() const;
	  private:
		Vector3 m_lineAnchor;
		Vector3 m_lineDirection;
		Vector3 m_anchorB;
		float m_minimumDistance = 0.f;
		float m_maximumDistance = 0.f;
	};
	class DLLNETWORK TwistLimit : public IJoint {
	  public:
		TwistLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Radian maxAngle);
		void SetMeasurementAxisA(const Vector3 &axis);
		Vector3 GetMeasurementAxisA();
		void SetMeasurementAxisB(const Vector3 &axis);
		Vector3 GetMeasurementAxisB();
		umath::Radian GetMaxAngle() const;
		const Vector3 &GetAxisA() const;
		const Vector3 &GetAxisB() const;
		~TwistLimit();
	  private:
		Vector3 m_axisA;
		Vector3 m_axisB;
		umath::Radian m_maxAngle = 0.f;
	};
	class DLLNETWORK SwivelHingeJoint : public IJoint {
	  public:
		SwivelHingeJoint(Bone &bone0, Bone &bone1, const Vector3 &worldHingeAxis, const Vector3 &worldTwistAxis);
		~SwivelHingeJoint();
		const Vector3 &GetWorldHingeAxis() const;
		const Vector3 &GetWorldTwistAxis() const;
	  private:
		Vector3 m_worldHingeAxis;
		Vector3 m_worldTwistAxis;
	};
	class DLLNETWORK Solver {
	  public:
		Solver(uint32_t controlIterationCount = 100, uint32_t fixerIterationCount = 10);
		~Solver();
		void Solve();
		DragControl &AddDragControl(Bone &bone);
		void RemoveControl(const IControl &ctrl);
		AngularPlaneControl &AddAngularPlaneControl(Bone &bone);
		IControl *FindControl(Bone &bone);
		std::shared_ptr<IControl> FindControlPtr(Bone &bone);
		StateControl &AddStateControl(Bone &bone);
		DistanceJoint &AddDistanceJoint(Bone &bone0, Bone &bone1);
		BallSocketJoint &AddBallSocketJoint(Bone &bone0, Bone &bone1, const Vector3 &anchor);
		AngularJoint &AddAngularJoint(Bone &bone0, Bone &bone1);
		PointOnLineJoint &AddPointOnLineJoint(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB);
		RevoluteJoint &AddRevoluteJoint(Bone &bone0, Bone &bone1, const Vector3 &freeAxis);
		SwingLimit &AddSwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, float maxAngle);
		EllipseSwingLimit &AddEllipseSwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, const Vector3 &xAxis, float maxAngleX, float maxAngleY);
		LinearAxisLimit &AddLinearAxisLimit(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB, float minimumDistance, float maximumDistance);
		TwistJoint &AddTwistJoint(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB);
		TwistLimit &AddTwistLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, float maxAngle);
		SwivelHingeJoint &AddSwivelHingeJoint(Bone &bone0, Bone &bone1, const Vector3 &worldHingeAxis, const Vector3 &worldTwistAxis);
		Bone &AddBone(const Vector3 &pos, const Quat &rot, float radius, float length, BoneId *optOutBoneId = nullptr);

		size_t GetControlCount() const;
		size_t GetBoneCount() const;
		size_t GetJointCount() const;

		IControl *GetControl(size_t index);
		const IControl *GetControl(size_t index) const;
		Bone *GetBone(BoneId index);
		const Bone *GetBone(BoneId index) const;
		IJoint *GetJoint(size_t index);
		const IJoint *GetJoint(size_t index) const;

		const std::vector<std::shared_ptr<IControl>> &GetControls() const;
		const std::vector<std::shared_ptr<Bone>> &GetBones() const;
		const std::vector<std::shared_ptr<IJoint>> &GetJoints() const;
	  private:
		std::unique_ptr<BEPUik::IKSolver> m_solver;
		std::vector<std::shared_ptr<IControl>> m_controls;
		std::vector<std::shared_ptr<Bone>> m_bones;
		std::vector<std::shared_ptr<IJoint>> m_joints;

		std::vector<BEPUik::Control *> m_bepuControls;
		std::vector<BEPUik::Bone *> m_bepuBones;
		std::vector<BEPUik::IKJoint *> m_bepuJoints;
	};

    DLLNETWORK void debug_print(const pragma::ik::Solver &solver);
};

DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::Bone &bone);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::IJoint &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::BallSocketJoint &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::AngularJoint &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::PointOnLineJoint &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::RevoluteJoint &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::SwingLimit &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::EllipseSwingLimit &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::LinearAxisLimit &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::TwistJoint &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::TwistLimit &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::SwivelHingeJoint &joint);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::IControl &control);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::ILinearMotorControl &control);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::DragControl &control);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::AngularPlaneControl &control);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::StateControl &control);
DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::ik::Solver &solver);

#endif