/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __IK_COMPONENT_HPP__
#define __IK_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/uvec.h>

class Jacobian;
class Tree;
class Node;
namespace pragma
{
	class DLLNETWORK IKComponent final
		: public BaseEntityComponent
	{
	public:
		IKComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void SetIKControllerEnabled(uint32_t ikControllerId,bool b);
		bool IsIKControllerEnabled(uint32_t ikControllerId) const;
		void SetIKEffectorPos(uint32_t ikControllerId,uint32_t effectorIdx,const Vector3 &pos);
		const Vector3 *GetIKEffectorPos(uint32_t ikControllerId,uint32_t effectorIdx) const;
		virtual void InitializeLuaObject(lua_State *l) override;
	protected:
		struct DLLNETWORK IKTreeInfo
		{
			struct DLLNETWORK NodeInfo
			{
				std::array<std::shared_ptr<Node>,3u> ikNodes = {}; // One node for each axis
				uint32_t boneId = std::numeric_limits<uint32_t>::max();
				Quat deltaRotation = uquat::identity(); // Rotation from model reference pose (for this bone) to IK node reference pose

				std::vector<std::shared_ptr<NodeInfo>> children;
				virtual bool IsEffector() const {return false;}
			};
			struct DLLNETWORK EffectorInfo
				: public NodeInfo
			{
				Vector3 position = {};
				uint32_t effectorIndex = std::numeric_limits<uint32_t>::max();
				uint32_t rootIndex = std::numeric_limits<uint32_t>::max();
				virtual bool IsEffector() const override {return true;}
			};
			struct DLLNETWORK FootInfo
			{
				float yOffset = 0.f;
				float yIkTreshold = 0.2f; // Default threshold
				uint32_t effectorBoneId = std::numeric_limits<uint32_t>::max();
			};
			std::shared_ptr<Jacobian> jacobian = nullptr;
			std::shared_ptr<Tree> tree = nullptr;
			std::unique_ptr<FootInfo> footInfo = nullptr;
			std::vector<std::shared_ptr<NodeInfo>> rootNodes = {};
			std::vector<std::weak_ptr<EffectorInfo>> effectors = {};
			bool enabled = false;
		};
		std::unordered_map<uint32_t,std::shared_ptr<IKTreeInfo>> m_ikTrees;

		bool InitializeIKController(uint32_t ikControllerId);
		void ClearIKControllers();
		virtual void UpdateInverseKinematics(double tDelta);
	};
};

#endif
