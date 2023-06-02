/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __IK_SOLVER_COMPONENT_HPP__
#define __IK_SOLVER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/base_entity_component_member_register.hpp"
#include "pragma/game/game_coordinate_system.hpp"
#include "pragma/util/ik.hpp"
#include <mathutil/uvec.h>

namespace pragma {
	namespace ik {
		class RigConfig;
	};
	class DLLNETWORK IkSolverComponent final : public BaseEntityComponent, public DynamicMemberRegister {
	  public:
		using IkBoneId = pragma::ik::BoneId;
		static ComponentEventId EVENT_INITIALIZE_SOLVER;
		static ComponentEventId EVENT_UPDATE_IK;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static std::optional<pragma::SignedAxis> FindTwistAxis(Model &mdl, BoneId boneId);
		IkSolverComponent(BaseEntity &ent);
		virtual ~IkSolverComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;

		void SetIkRigFile(const std::string &ikRigFile);
		const std::string &GetIkRigFile() const;

		void AddSkeletalBone(BoneId boneId);
		void SetBoneLocked(BoneId boneId, bool locked);
		pragma::ik::Bone *GetBone(BoneId boneId);
		pragma::ik::IControl *GetControl(BoneId boneId);
		size_t GetBoneCount() const;
		std::optional<BoneId> GetSkeletalBoneId(IkBoneId boneId) const;
		std::optional<IkBoneId> GetIkBoneId(BoneId boneId) const;

		void AddDragControl(BoneId boneId);
		void AddStateControl(BoneId boneId);

		void AddFixedConstraint(BoneId boneId0, BoneId boneId1);
		void AddHingeConstraint(BoneId boneId0, BoneId boneId1, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation = uquat::identity(), Axis twistAxis = Axis::X);
		void AddBallSocketConstraint(BoneId boneId0, BoneId boneId1, const EulerAngles &minLimits, const EulerAngles &maxLimits, Axis twistAxis = Axis::Z);

		udm::PProperty &GetIkRig() { return m_ikRig; }
		void Solve();
		void ResetIkRig();

		const std::shared_ptr<pragma::ik::Solver> &GetIkSolver() const;
		bool AddIkSolverByRig(const ik::RigConfig &ikRig);
		bool AddIkSolverByChain(const std::string &boneName, uint32_t chainLength);
		virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;

		// Internal use only
		bool UpdateIkRig();
	  protected:
		static void UpdateGlobalSolverSettings();
		void UpdateSolverSettings();
		virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
		void ResetIkBones();
		void UpdateIkRigFile();
		void InitializeSolver();
		static std::optional<umath::ScaledTransform> GetReferenceBonePose(Model &model, BoneId boneId);
		std::optional<umath::ScaledTransform> GetReferenceBonePose(BoneId boneId) const;
		pragma::ik::Bone *AddBone(const std::string &boneName, BoneId boneId, const umath::Transform &pose, float radius, float length);
		void AddControl(BoneId boneId, bool translation, bool rotation);
		pragma::ik::Bone *GetIkBone(BoneId boneId);

		bool GetConstraintBones(BoneId boneId0, BoneId boneId1, pragma::ik::Bone **bone0, pragma::ik::Bone **bone1, umath::ScaledTransform &pose0, umath::ScaledTransform &pose1) const;

		udm::PProperty m_ikRig;
		std::string m_ikRigFile;
		std::shared_ptr<pragma::ik::Solver> m_ikSolver;
		std::unordered_map<BoneId, IkBoneId> m_boneIdToIkBoneId;
		std::unordered_map<IkBoneId, BoneId> m_ikBoneIdToBoneId;
		std::unordered_map<BoneId, std::shared_ptr<pragma::ik::IControl>> m_ikControls;
		bool m_updateRequired = false;
	};
};

#endif
