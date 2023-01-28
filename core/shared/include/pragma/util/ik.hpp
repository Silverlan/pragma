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

namespace BEPUik
{
	class IKSolver;
	class Bone;
	class Control;
	class SingleBoneLinearMotor;
	class SingleBoneAngularPlaneConstraint;
	class IKJoint;
};

namespace pragma::ik
{
	using BoneId = uint32_t;
	class DLLNETWORK Bone
	{
	public:
		Bone(const Vector3 &pos,const Quat &rot,float radius,float length,float mass);
		~Bone();
		Vector3 GetPos() const;
		Quat GetRot() const;
		void SetPos(const Vector3 &pos) const;
		void SetRot(const Quat &rot) const;
		void SetPinned(bool pinned);
		bool IsPinned();
		BEPUik::Bone *operator*();
		BEPUik::Bone *operator->();
	private:
		std::unique_ptr<BEPUik::Bone> m_bone;
	};
	class DLLNETWORK IControl
	{
	public:
		~IControl();
		BEPUik::Control *operator*();
	protected:
		IControl();
		std::unique_ptr<BEPUik::Control> m_control;
	};
	class DLLNETWORK ILinearMotorControl
	{
	public:
		virtual BEPUik::SingleBoneLinearMotor &GetLinearMotor()=0;
		const BEPUik::SingleBoneLinearMotor &GetLinearMotor() const;
		void SetTargetPosition(const Vector3 &pos);
		Vector3 GetTargetPosition() const;
	};
	class DLLNETWORK DragControl
		: public IControl,public ILinearMotorControl
	{
	public:
		DragControl(Bone &bone);
		~DragControl();
		virtual BEPUik::SingleBoneLinearMotor &GetLinearMotor() override;
	};
	class DLLNETWORK AngularPlaneControl
		: public IControl
	{
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
	class DLLNETWORK StateControl
		: public IControl,public ILinearMotorControl
	{
	public:
		StateControl(Bone &bone);
		~StateControl();
		void SetTargetOrientation(const Quat &rot);
		Quat GetTargetOrientation() const;
		virtual BEPUik::SingleBoneLinearMotor &GetLinearMotor() override;
	};
	class DLLNETWORK IJoint
	{
	public:
		~IJoint();

		void SetRigidity(float rigidity);
		float GetRigidity();

		BEPUik::IKJoint *operator*();
	protected:
		IJoint();
		std::unique_ptr<BEPUik::IKJoint> m_joint;
	};
	class DLLNETWORK DistanceJoint
		: public IJoint
	{
	public:
		DistanceJoint(Bone &bone0,Bone &bone1);
		~DistanceJoint();
	};
	class DLLNETWORK PointOnLineJoint
		: public IJoint
	{
	public:
		PointOnLineJoint(Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,const Vector3 &anchorB);
		~PointOnLineJoint();
	};
	class DLLNETWORK BallSocketJoint
		: public IJoint
	{
	public:
		BallSocketJoint(Bone &bone0,Bone &bone1,const Vector3 &anchor);
		~BallSocketJoint();
	};
	class DLLNETWORK AngularJoint
		: public IJoint
	{
	public:
		AngularJoint(Bone &bone0,Bone &bone1);
		~AngularJoint();
	};
	class DLLNETWORK RevoluteJoint
		: public IJoint
	{
	public:
		RevoluteJoint(Bone &bone0,Bone &bone1,const Vector3 &freeAxis);
		~RevoluteJoint();
	};
	class DLLNETWORK TwistJoint
		: public IJoint
	{
	public:
		TwistJoint(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB);
		~TwistJoint();
	};
	class DLLNETWORK SwingLimit
		: public IJoint
	{
	public:
		SwingLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle);
		~SwingLimit();
	};
	class DLLNETWORK EllipseSwingLimit
		: public IJoint
	{
	public:
		EllipseSwingLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,const Vector3 &xAxis,float maxAngleX,float maxAngleY);
		~EllipseSwingLimit();
	};
	class DLLNETWORK LinearAxisLimit
		: public IJoint
	{
	public:
		LinearAxisLimit(
			Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,
			const Vector3 &anchorB,float minimumDistance,float maximumDistance
		);
		~LinearAxisLimit();
	};
	class DLLNETWORK TwistLimit
		: public IJoint
	{
	public:
		TwistLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle);
		void SetMeasurementAxisA(const Vector3 &axis);
		Vector3 GetMeasurementAxisA();
		void SetMeasurementAxisB(const Vector3 &axis);
		Vector3 GetMeasurementAxisB();
		~TwistLimit();
	};
	class DLLNETWORK SwivelHingeJoint
		: public IJoint
	{
	public:
		SwivelHingeJoint(Bone &bone0,Bone &bone1,const Vector3 &worldHingeAxis,const Vector3 &worldTwistAxis);
		~SwivelHingeJoint();
	};
	class DLLNETWORK Solver
	{
	public:
		Solver(uint32_t controlIterationCount=100,uint32_t fixerIterationCount=10);
		~Solver();
		void Solve();
		DragControl &AddDragControl(Bone &bone);
		void RemoveControl(const IControl &ctrl);
		AngularPlaneControl &AddAngularPlaneControl(Bone &bone);
		IControl *FindControl(Bone &bone);
		std::shared_ptr<IControl> FindControlPtr(Bone &bone);
		StateControl &AddStateControl(Bone &bone);
		DistanceJoint &AddDistanceJoint(Bone &bone0,Bone &bone1);
		BallSocketJoint &AddBallSocketJoint(Bone &bone0,Bone &bone1,const Vector3 &anchor);
		AngularJoint &AddAngularJoint(Bone &bone0,Bone &bone1);
		PointOnLineJoint &AddPointOnLineJoint(Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,const Vector3 &anchorB);
		RevoluteJoint &AddRevoluteJoint(Bone &bone0,Bone &bone1,const Vector3 &freeAxis);
		SwingLimit &AddSwingLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle);
		EllipseSwingLimit &AddEllipseSwingLimit(
			Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,const Vector3 &xAxis,
			float maxAngleX,float maxAngleY
		);
		LinearAxisLimit &AddLinearAxisLimit(
			Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,
			const Vector3 &anchorB,float minimumDistance,float maximumDistance
		);
		TwistJoint &AddTwistJoint(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB);
		TwistLimit &AddTwistLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle);
		SwivelHingeJoint &AddSwivelHingeJoint(Bone &bone0,Bone &bone1,const Vector3 &worldHingeAxis,const Vector3 &worldTwistAxis);
		Bone &AddBone(const Vector3 &pos,const Quat &rot,float radius,float length,BoneId *optOutBoneId=nullptr);

		size_t GetControlCount() const;
		size_t GetBoneCount() const;
		size_t GetJointCount() const;

		IControl *GetControl(size_t index);
		Bone *GetBone(BoneId index);
		IJoint *GetJoint(size_t index);

		const std::vector<std::shared_ptr<IControl>> &GetControls() const;
		const std::vector<std::shared_ptr<Bone>> &GetBones() const;
		const std::vector<std::shared_ptr<IJoint>> &GetJoints() const;
	private:
		std::unique_ptr<BEPUik::IKSolver> m_solver;
		std::vector<std::shared_ptr<IControl>> m_controls;
		std::vector<std::shared_ptr<Bone>> m_bones;
		std::vector<std::shared_ptr<IJoint>> m_joints;

		std::vector<BEPUik::Control*> m_bepuControls;
		std::vector<BEPUik::Bone*> m_bepuBones;
		std::vector<BEPUik::IKJoint*> m_bepuJoints;
	};
};

#endif
