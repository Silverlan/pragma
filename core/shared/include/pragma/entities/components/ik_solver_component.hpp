/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __IK_SOLVER_COMPONENT_HPP__
#define __IK_SOLVER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/base_entity_component_member_register.hpp"
#include "pragma/util/ik.hpp"
#include <mathutil/uvec.h>

namespace pragma
{
	namespace ik
	{
		class RigConfig;
	};
	class DLLNETWORK IkSolverComponent final
		: public BaseEntityComponent,
		public DynamicMemberRegister
	{
	public:
		using IkBoneId = pragma::ik::BoneId;
		static ComponentEventId EVENT_INITIALIZE_SOLVER;
		static ComponentEventId EVENT_UPDATE_IK;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);
		IkSolverComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;

		void SetIkRigFile(const std::string &ikRigFile);
		const std::string &GetIkRigFile() const;

		void AddSkeletalBone(BoneId boneId);
		void SetBoneLocked(BoneId boneId,bool locked);
		pragma::ik::Bone *GetBone(BoneId boneId);

		void AddDragControl(BoneId boneId);
		void AddStateControl(BoneId boneId);

		void AddFixedConstraint(BoneId boneId0,BoneId boneId1);
		void AddHingeConstraint(BoneId boneId0,BoneId boneId1,umath::Degree minAngle,umath::Degree maxAngle);
		void AddBallSocketConstraint(BoneId boneId0,BoneId boneId1,const EulerAngles &minLimits,const EulerAngles &maxLimits);

		udm::PProperty &GetIkRig() {return m_ikRig;}
		void Solve();
	protected:
		void ResetIkBones();
		void UpdateIkRigFile();
		void InitializeSolver();
		bool AddIkSolverByRig(const ik::RigConfig &ikRig);
		bool AddIkSolverByChain(const std::string &boneName,uint32_t chainLength);
		bool UpdateIkRig();
		std::optional<umath::ScaledTransform> GetReferenceBonePose(BoneId boneId) const;
		pragma::ik::Bone *AddBone(BoneId boneId,const umath::Transform &pose,float radius,float length);
		void AddControl(BoneId boneId,const Vector3 &translation,const Quat *rotation);
		std::optional<IkBoneId> GetIkBoneId(BoneId boneId) const;
		pragma::ik::Bone *GetIkBone(BoneId boneId);

		udm::PProperty m_ikRig;
		std::string m_ikRigFile;
		std::unique_ptr<pragma::ik::Solver> m_ikSolver;
		std::unordered_map<BoneId,IkBoneId> m_boneIdToIkBoneId;
		std::unordered_map<IkBoneId,BoneId> m_ikBoneIdToBoneId;
		std::unordered_map<BoneId,std::shared_ptr<pragma::ik::IControl>> m_ikControls;
		bool m_updateRequired = false;
	};
};

#endif
