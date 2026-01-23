// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "buss_ik/Tree.h"
#include "buss_ik/Jacobian.h"
#include <cassert>

module pragma.shared;

import :entities.components.ik;

using namespace pragma;

IKComponent::IKComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void IKComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("animated");
	BindEventUnhandled(baseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateInverseKinematics(GetEntity().GetNetworkState()->GetGameState()->DeltaTime()); });
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		m_ikTrees.clear();
		ClearIKControllers();
	});
}
void IKComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
bool IKComponent::InitializeIKController(uint32_t ikControllerId)
{
	auto it = m_ikTrees.find(ikControllerId);
	if(it != m_ikTrees.end())
		return true;
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return false;
	auto *ikController = hMdl->GetIKController(ikControllerId);
	if(ikController == nullptr)
		return false;
	auto chainLen = ikController->GetChainLength();
	if(chainLen <= 1) {
		Con::CWAR << "Unable to initialize ik controller for " << ikController->GetEffectorName() << ": Chain length has to be at least 1!" << Con::endl;
		return false;
	}
	auto &skeleton = hMdl->GetSkeleton();
	auto &boneName = ikController->GetEffectorName();
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId < 0) {
		Con::CWAR << "Unable to initialize ik controller for " << ikController->GetEffectorName() << ": Invalid bone '" << boneName << "'!" << Con::endl;
		return false;
	}
	auto wpBone = skeleton.GetBone(boneId);
	if(wpBone.expired()) {
		Con::CWAR << "Unable to initialize ik controller for " << ikController->GetEffectorName() << ": Invalid bone '" << boneName << "'!" << Con::endl;
		return false;
	}
	auto &reference = hMdl->GetReference();
	struct IKJointInfo {
		IKJointInfo(uint32_t boneId) : boneId(boneId) {}
		uint32_t boneId = std::numeric_limits<uint32_t>::max();
		uint32_t jointId = std::numeric_limits<uint32_t>::max();
		math::OrientedPoint referenceTransform = {};

		std::array<std::shared_ptr<Node>, 3> nodes = {};
	};

	auto ikTreeInfo = pragma::util::make_shared<IKTreeInfo>();
	std::vector<IKJointInfo> ikJoints;
	ikJoints.reserve(chainLen);
	ikJoints.push_back({static_cast<uint32_t>(boneId)});

	auto bone = wpBone.lock();
	for(auto i = decltype(chainLen) {0}; i < (chainLen - 1); ++i) {
		auto parent = bone->parent;
		if(parent.expired()) {
			Con::CWAR << "Unable to initialize ik controller for " << ikController->GetEffectorName() << ": Total chain length exceeds bone hierarchy!" << Con::endl;
			return false;
		}
		bone = parent.lock();
		ikJoints.push_back({bone->ID});
	}

	for(auto &ikJoint : ikJoints) {
		auto *ppos = reference.GetBonePosition(ikJoint.boneId);
		auto *prot = reference.GetBoneOrientation(ikJoint.boneId);
		auto pos = ppos ? *ppos : Vector3 {};
		auto rot = prot ? *prot : uquat::identity();
		ikJoint.referenceTransform = {pos, rot};
	}

	// Find joints associated with ik chain bones
	auto &joints = hMdl->GetJoints();
	for(auto &ikJoint : ikJoints) {
		auto boneId = ikJoint.boneId;
		auto itJoint = std::find_if(joints.begin(), joints.end(), [boneId](const physics::JointInfo &joint) { return joint.child == boneId && (joint.type == physics::JointType::DOF || joint.type == physics::JointType::ConeTwist); });
		if(itJoint == joints.end()) {
			Con::CWAR << "Unable to initialize ik controller for " << ikController->GetEffectorName() << ": Joint for bone " << ikJoint.boneId << " in chain does not have joint assigned to it!" << Con::endl;
			return false; // All bones in chain need to have a valid joint assigned to them
		}
		ikJoint.jointId = itJoint - joints.begin();
	}

	// Effector is always bottom-most element in tree (= first element in ikJoints)
	auto &ikJointEffector = ikJoints.front();
	auto &effectorPos = ikJointEffector.referenceTransform.pos;
	ikJointEffector.nodes.at(0) = pragma::util::make_shared<Node>(VectorR3(effectorPos.x, effectorPos.y, effectorPos.z), VectorR3(0.f, 0.f, 0.f), 0.0, EFFECTOR);

	for(auto it = ikJoints.begin() + 1; it < ikJoints.end(); ++it) {
		auto &ikJoint = *it;
		auto &pos = ikJoint.referenceTransform.pos;

		auto &joint = joints.at(ikJoint.jointId);
		auto min = EulerAngles {};
		auto max = EulerAngles {};
		switch(joint.type) {
		case physics::JointType::DOF:
			{
				auto itMin = joint.args.find("ang_limit_l");
				if(itMin != joint.args.end())
					min = EulerAngles(itMin->second);
				auto itMax = joint.args.find("ang_limit_u");
				if(itMax != joint.args.end())
					max = EulerAngles(itMax->second);
				break;
			}
		case physics::JointType::ConeTwist:
			{
				auto itSp1l = joint.args.find("sp1l");
				if(itSp1l != joint.args.end())
					min.p = util::to_float(itSp1l->second);

				auto itSp1u = joint.args.find("sp1u");
				if(itSp1u != joint.args.end())
					max.p = util::to_float(itSp1u->second);

				auto itSp2l = joint.args.find("sp2l");
				if(itSp2l != joint.args.end())
					min.y = util::to_float(itSp2l->second);

				auto itSp2u = joint.args.find("sp2u");
				if(itSp2u != joint.args.end())
					max.y = util::to_float(itSp2u->second);

				auto itTsl = joint.args.find("tsl");
				if(itTsl != joint.args.end())
					min.r = util::to_float(itTsl->second);

				auto itTsu = joint.args.find("tsu");
				if(itTsu != joint.args.end())
					max.r = util::to_float(itTsu->second);
				break;
			}
		}

		// Twist is disabled for now; Does not contribute to ik in meaningful way
		// max.p = 0.f;
		// min.p = 0.f;

		auto &rot = ikJoint.referenceTransform.rot;
		auto rotAxis = uquat::up(rot);
		ikJoint.nodes.at(2) = pragma::util::make_shared<Node>(VectorR3(pos.x, pos.y, pos.z), VectorR3(rotAxis.x, rotAxis.y, rotAxis.z), 0.0, JOINT, math::deg_to_rad(min.y), math::deg_to_rad(max.y), math::deg_to_rad(0.0));

		rotAxis = uquat::forward(rot); // TODO: Does this axis have to be negated?
		ikJoint.nodes.at(1) = pragma::util::make_shared<Node>(VectorR3(pos.x, pos.y, pos.z), VectorR3(rotAxis.x, rotAxis.y, rotAxis.z), 0.0, JOINT, math::deg_to_rad(min.r), math::deg_to_rad(max.r), math::deg_to_rad(0.0));

		rotAxis = -uquat::right(rot);
		ikJoint.nodes.at(0) = pragma::util::make_shared<Node>(VectorR3(pos.x, pos.y, pos.z), VectorR3(rotAxis.x, rotAxis.y, rotAxis.z), 0.0, JOINT, math::deg_to_rad(min.p), math::deg_to_rad(max.p), math::deg_to_rad(0.0));
	}

	// Initialize IK Tree
	auto ikTree = pragma::util::make_shared<Tree>();
	ikTree->InsertRoot(ikJoints.back().nodes.at(0).get());

	for(auto it = ikJoints.rbegin(); it != (ikJoints.rend() - 1); ++it) {
		auto &ikJoint = *it;
		ikTree->InsertLeftChild(ikJoint.nodes.at(0).get(), ikJoint.nodes.at(1).get());
		ikTree->InsertLeftChild(ikJoint.nodes.at(1).get(), ikJoint.nodes.at(2).get());

		auto &ikJointNext = *(it + 1);
		ikTree->InsertLeftChild(ikJoint.nodes.at(2).get(), ikJointNext.nodes.at(0).get());
	}

	ikTreeInfo->jacobian = pragma::util::make_shared<Jacobian>(ikTree.get());

	if(string::compare<std::string>(ikController->GetType(), "foot", false) == true) {
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		if(mdlComponent) {
			auto boneId = mdlComponent->LookupBone(ikController->GetEffectorName());
			if(boneId != -1) {
				ikTreeInfo->footInfo = std::make_unique<IKTreeInfo::FootInfo>();
				ikTreeInfo->footInfo->effectorBoneId = boneId;
				auto &keyValues = ikController->GetKeyValues();
				auto itOffset = keyValues.find("foot_height_offset");
				if(itOffset != keyValues.end())
					ikTreeInfo->footInfo->yOffset = util::to_float(itOffset->second);

				auto itThreshold = keyValues.find("foot_height_threshold");
				if(itThreshold != keyValues.end())
					ikTreeInfo->footInfo->yIkTreshold = util::to_float(itThreshold->second);
			}
		}
	}

	ikTreeInfo->tree = ikTree;
	auto *childNodes = &ikTreeInfo->rootNodes;
	auto effectorIdx = 0u;
	for(auto it = ikJoints.rbegin(); it != ikJoints.rend(); ++it) {
		auto &ikJoint = *it;
		auto bEffector = (it == ikJoints.rend() - 1) ? true : false;
		if(bEffector == false)
			childNodes->push_back(pragma::util::make_shared<IKTreeInfo::NodeInfo>());
		else
			childNodes->push_back(pragma::util::make_shared<IKTreeInfo::EffectorInfo>());
		auto &nodeInfo = childNodes->back();
		nodeInfo->ikNodes = ikJoint.nodes;
		nodeInfo->boneId = ikJoint.boneId;

		auto *pos = reference.GetBonePosition(ikJoint.boneId);
		auto *rot = reference.GetBoneOrientation(ikJoint.boneId);
		if(pos != nullptr && rot != nullptr) {
			auto &node = ikJoint.nodes.at(0);
			auto rotNode = util::ik::get_rotation(*node);
			uquat::inverse(rotNode);
			nodeInfo->deltaRotation = *rot * rotNode;
		}
		if(nodeInfo->IsEffector()) {
			auto *effectorInfo = static_cast<IKTreeInfo::EffectorInfo *>(nodeInfo.get());
			effectorInfo->effectorIndex = effectorIdx++;
			effectorInfo->rootIndex = 0u;
			effectorInfo->position = effectorPos;
			ikTreeInfo->effectors.push_back(std::static_pointer_cast<IKTreeInfo::EffectorInfo>(nodeInfo));
		}
		childNodes = &nodeInfo->children;
	}

	ikTree->Init();
	ikTree->Compute();
	ikTreeInfo->jacobian->Reset();

	m_ikTrees.insert(std::make_pair(ikControllerId, ikTreeInfo));
	return true;
}
void IKComponent::SetIKControllerEnabled(uint32_t ikControllerId, bool b)
{
	auto it = m_ikTrees.find(ikControllerId);
	if(it != m_ikTrees.end()) {
		it->second->enabled = b;
		return;
	}
	if(b == false || InitializeIKController(ikControllerId) == false)
		return;
	it = m_ikTrees.find(ikControllerId);
	if(it == m_ikTrees.end())
		return;
	it->second->enabled = b;
}
bool IKComponent::IsIKControllerEnabled(uint32_t ikControllerId) const
{
	auto it = m_ikTrees.find(ikControllerId);
	if(it == m_ikTrees.end())
		return false;
	return it->second->enabled;
}
void IKComponent::ClearIKControllers() { m_ikTrees.clear(); }

void IKComponent::SetIKEffectorPos(uint32_t ikControllerId, uint32_t effectorIdx, const Vector3 &pos)
{
	auto it = m_ikTrees.find(ikControllerId);
	if(it == m_ikTrees.end())
		return;
	auto &treeInfo = *it->second;
	auto &effectors = treeInfo.effectors;
	if(effectorIdx >= effectors.size())
		return;
	auto &effector = effectors.at(effectorIdx);
	if(effector.expired())
		return;
	effector.lock()->position = pos;
}
const Vector3 *IKComponent::GetIKEffectorPos(uint32_t ikControllerId, uint32_t effectorIdx) const
{
	auto it = m_ikTrees.find(ikControllerId);
	if(it == m_ikTrees.end())
		return nullptr;
	auto &treeInfo = *it->second;
	auto &effectors = treeInfo.effectors;
	if(effectorIdx >= effectors.size())
		return nullptr;
	auto &effector = effectors.at(effectorIdx);
	if(effector.expired())
		return nullptr;
	return &effector.lock()->position;
}

void IKComponent::UpdateInverseKinematics(double tDelta)
{
	if(m_ikTrees.empty())
		return;
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto animComponent = ent.GetAnimatedComponent();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr || animComponent.expired())
		return;

	// Update feet effector positions
	struct FootData {
		uint32_t boneId = std::numeric_limits<uint32_t>::max();
		Vector3 upNormal = {};
		Quat rotation = uquat::identity();
		bool enabled = true;
	};
	auto pTrComponent = ent.GetTransformComponent();
	auto pPhysComponent = ent.GetPhysicsComponent();
	const auto up = pTrComponent ? pTrComponent->GetUp() : uvec::PRM_UP;
	auto yExtent = pPhysComponent ? pPhysComponent->GetCollisionExtents().y : 0.f;
	std::unordered_map<uint32_t, FootData> feetData {};
	auto &reference = hMdl->GetReference();
	for(auto &pair : m_ikTrees) {
		if(pair.second->enabled == false || pair.second->footInfo == nullptr)
			continue;
		auto *ikController = hMdl->GetIKController(pair.first);
		if(ikController == nullptr)
			continue;
		auto &footInfo = *pair.second->footInfo;
		auto boneId = footInfo.effectorBoneId;
		Vector3 pos {};
		auto rot = uquat::identity();
		animComponent->GetBonePose(boneId, &pos, &rot, nullptr, math::CoordinateSpace::World);

		auto srcPos = pos + up * yExtent;
		auto dstPos = pos - up * yExtent;
		if(pTrComponent)
			pTrComponent->WorldToLocal(&pos, &rot);

		auto &footData = feetData.insert(std::make_pair(pair.first, FootData {})).first->second;
		auto *refPos = reference.GetBonePosition(boneId);
		if(refPos != nullptr) {
			auto footHeight = pos.y - refPos->y;   // Foot is raised (above foot pose in reference)
			if(footHeight >= footInfo.yIkTreshold) // Disable IK if foot is raised above threshold
			{
				footData.enabled = false;
				SetIKControllerEnabled(pair.first, false);
				continue;
			}
		}

		auto bIkPlaced = false;
		if(pTrComponent) {
			auto traceData = util::get_entity_trace_data(*pTrComponent);
			traceData.SetSource(srcPos);
			traceData.SetTarget(dstPos);
			auto *game = ent.GetNetworkState()->GetGameState();
			auto rayResult = game->RayCast(traceData);

			if(rayResult.hitType != physics::RayCastHitType::None) {
				auto posRay = rayResult.position + rayResult.normal * footInfo.yOffset;
				if(pTrComponent)
					pTrComponent->WorldToLocal(&posRay);
				SetIKEffectorPos(pair.first, 0u, posRay);
				footData.upNormal = rayResult.normal;
				footData.boneId = boneId;
				footData.rotation = rot;
				uvec::rotate(&footData.upNormal, pTrComponent ? uquat::get_inverse(pTrComponent->GetRotation()) : uquat::identity());
				bIkPlaced = true;
			}
		}
		if(bIkPlaced == false) {
			footData.enabled = false;
			SetIKControllerEnabled(pair.first, false); // Temporarily disable (Controller is re-enabled after IK processing)
		}
	}

	for(auto &pair : m_ikTrees) {
		if(pair.second->enabled == false)
			continue;
		auto *ikController = hMdl->GetIKController(pair.first);
		if(ikController == nullptr)
			continue;

		auto &treeInfo = *pair.second;
		std::vector<math::Transform> rootDeltaTransforms {};
		rootDeltaTransforms.reserve(treeInfo.rootNodes.size());
		for(auto &rootNodeInfo : treeInfo.rootNodes) {
			rootDeltaTransforms.push_back({});
			auto &t = rootDeltaTransforms.back();
			Vector3 pos {};
			auto rot = uquat::identity();
			if(animComponent->GetBonePose(rootNodeInfo->boneId, &pos, &rot, nullptr, math::CoordinateSpace::Object) == true) {
				auto *posRef = reference.GetBonePosition(rootNodeInfo->boneId);
				auto *rotRef = reference.GetBoneOrientation(rootNodeInfo->boneId);
				if(posRef != nullptr && rotRef != nullptr) {
					auto posDelta = Vector3 {};
					auto rotDelta = uquat::identity();
					uvec::world_to_local(*posRef, *rotRef, posDelta, rotDelta);
					uvec::local_to_world(pos, rot, posDelta, rotDelta);
					t.SetOrigin(posDelta);
					t.SetRotation(rotDelta);
				}
			}
		}

		std::vector<VectorR3> ikEffectorPositions {};
		ikEffectorPositions.reserve(treeInfo.effectors.size());
		for(auto &wpEffector : treeInfo.effectors) {
			if(wpEffector.expired()) {
				ikEffectorPositions.push_back({});
				continue;
			}
			auto effector = wpEffector.lock();
			if(effector->rootIndex >= rootDeltaTransforms.size()) {
				ikEffectorPositions.push_back({});
				continue;
			}
			auto &t = rootDeltaTransforms.at(effector->rootIndex);
			auto posEffector = effector->position;
			posEffector = t.GetInverse() * posEffector;
			ikEffectorPositions.push_back(VectorR3(posEffector.x, posEffector.y, posEffector.z));
		}

		auto &jacobian = *treeInfo.jacobian;
		jacobian.SetJtargetActive();
		jacobian.ComputeJacobian(ikEffectorPositions.data());
		switch(ikController->GetMethod()) {
		case physics::ik::Method::SelectivelyDampedLeastSquare:
			jacobian.CalcDeltaThetasSDLS();
			break;
		case physics::ik::Method::DampedLeastSquares:
			jacobian.CalcDeltaThetasDLS();
			break;
		case physics::ik::Method::DampedLeastSquaresWithSingularValueDecomposition:
			jacobian.CalcDeltaThetasDLSwithSVD();
			break;
		case physics::ik::Method::Pseudoinverse:
			jacobian.CalcDeltaThetasPseudoinverse();
			break;
		case physics::ik::Method::JacobianTranspose:
			jacobian.CalcDeltaThetasTranspose();
			break;
		default:
			jacobian.ZeroDeltaThetas();
			break;
		}
		jacobian.UpdateThetas();
		jacobian.UpdatedSClampValue(ikEffectorPositions.data());

		static auto debugPrint = false;
		if(debugPrint) {
			auto *game = GetEntity().GetNetworkState()->GetGameState();
			auto fGetLocalTransform = [](const Node *node, math::Transform &act) {
				auto axis = Vector3(node->v.x, node->v.y, node->v.z);
				auto rot = uquat::identity();
				if(axis.length())
					rot = Quat(node->GetTheta(), axis);
				act.SetIdentity();
				act.SetRotation(rot);
				act.SetOrigin(Vector3(node->r.x, node->r.y, node->r.z));
			};
			std::function<void(Node *, const math::Transform &)> fDrawTree = nullptr;
			fDrawTree = [&fGetLocalTransform, &fDrawTree, game](Node *node, const math::Transform &tr) {
				Vector3 lineColor = Vector3(0, 0, 0);
				int lineWidth = 2;
				auto fUpdateLine = [game](int32_t tIdx, const Vector3 &start, const Vector3 &end, const Color &col) {
					/*auto it = m_dbgObjects.find(tIdx);
					if(it == m_dbgObjects.end())
						it = m_dbgObjects.insert(std::make_pair(tIdx,DebugRenderer::DrawLine(start,end,col))).first;
					auto &wo = static_cast<DebugRenderer::WorldObject&>(*m_dbgObjects.at(tIdx));
					wo.GetVertices().at(0) = start;
					wo.GetVertices().at(1) = end;
					wo.UpdateVertexBuffer();*/
					game->DrawLine(start, end, col, 0.05f);
					//DebugRenderer::DrawLine(start,end,col,0.05f);
				};
				if(node != 0) {
					//	glPushMatrix();
					Vector3 pos = Vector3(tr.GetOrigin().x, tr.GetOrigin().y, tr.GetOrigin().z);
					Vector3 color = Vector3(0, 1, 0);
					int pointSize = 10;
					auto enPos = uvec::create(pos);
					//auto it = m_dbgObjects.find(0u);
					//if(it == m_dbgObjects.end())
					//	it = m_dbgObjects.insert(std::make_pair(0u,DebugRenderer::DrawPoint(enPos,colors::Lime))).first;
					//it->second->SetPos(enPos);

					auto enForward = uquat::forward(tr.GetRotation());
					auto enRight = uquat::right(tr.GetRotation());
					auto enUp = uquat::up(tr.GetRotation());
					auto worldScale = game->GetPhysicsEnvironment()->GetWorldScale();
					fUpdateLine(1, enPos, enPos + enForward * static_cast<float>(0.05f / worldScale), colors::Red);
					fUpdateLine(2, enPos, enPos + enRight * static_cast<float>(0.05f / worldScale), colors::Lime);
					fUpdateLine(3, enPos, enPos + enUp * static_cast<float>(0.05f / worldScale), colors::Aqua);

					Vector3 axisLocal = Vector3(node->v.x, node->v.y, node->v.z);
					Vector3 axisWorld = tr * axisLocal;

					fUpdateLine(4, enPos, enPos + 0.1f * uvec::create(axisWorld), colors::Yellow);

					//node->DrawNode(node == root);	// Recursively draw node and update ModelView matrix
					if(node->left) {
						math::Transform act;
						fGetLocalTransform(node->left, act);

						math::Transform trl = tr * act;
						auto trOrigin = tr.GetOrigin();
						auto trlOrigin = trl.GetOrigin();
						fUpdateLine(5, trOrigin, trlOrigin, colors::Maroon);
						fDrawTree(node->left, trl); // Draw tree of children recursively
					}
					//	glPopMatrix();
					if(node->right) {
						math::Transform act;
						fGetLocalTransform(node->right, act);
						math::Transform trr = tr * act;
						auto trOrigin = tr.GetOrigin();
						auto trrOrigin = trr.GetOrigin();
						fUpdateLine(6, trOrigin, trrOrigin, colors::Silver);
						fDrawTree(node->right, trr); // Draw right siblings recursively
					}
				}
			};
			auto fRenderScene = [&fGetLocalTransform, &fDrawTree, &rootDeltaTransforms](Tree &tree) {
				auto &tRoot = rootDeltaTransforms.front();
				math::Transform act {};
				fGetLocalTransform(tree.GetRoot(), act);
				act = tRoot * act;

				fDrawTree(tree.GetRoot(), act);
			};
			fRenderScene(*pair.second->tree);
		}

		// Apply IK transforms to entity skeleton
		std::function<void(const std::vector<std::shared_ptr<IKTreeInfo::NodeInfo>> &, math::Transform &, math::Transform *, bool)> fIterateIkTree = nullptr;
		fIterateIkTree = [this, &fIterateIkTree, &rootDeltaTransforms, &animComponent](const std::vector<std::shared_ptr<IKTreeInfo::NodeInfo>> &nodes, math::Transform &tParent, math::Transform *rootDeltaTransform, bool root) {
			auto nodeIdx = 0u;
			for(auto &nodeInfo : nodes) {
				if(root == true) {
					assert(nodeIdx < rootDeltaTransforms.size());
					if(nodeIdx >= rootDeltaTransforms.size())
						continue;
					rootDeltaTransform = &rootDeltaTransforms.at(nodeIdx);
				}
				math::Transform tNode {};
				util::ik::get_local_transform(*nodeInfo->ikNodes.at(0u), tNode);
				tNode = tParent * tNode;

				for(auto i = decltype(nodeInfo->ikNodes.size()) {1u}; i < nodeInfo->ikNodes.size(); ++i) {
					math::Transform tNodeOther {};
					auto &nodeOther = nodeInfo->ikNodes.at(i);
					if(nodeOther != nullptr) {
						util::ik::get_local_transform(*nodeOther, tNodeOther);
						tNode *= tNodeOther;
					}
				}

				auto tLocal = *rootDeltaTransform * tNode;
				auto pos = tLocal.GetOrigin();
				auto rot = tLocal.GetRotation() * nodeInfo->deltaRotation;
				animComponent->SetBonePose(nodeInfo->boneId, &pos, &rot, nullptr, math::CoordinateSpace::Object);

				fIterateIkTree(nodeInfo->children, tNode, rootDeltaTransform, false);
				++nodeIdx;
			}
		};
		math::Transform t {};
		fIterateIkTree(treeInfo.rootNodes, t, nullptr, true);
	}

	// Update feet rotations (Has to be done AFTER inverse kinematics have been applied)
	const auto forward = pTrComponent ? pTrComponent->GetForward() : uvec::PRM_FORWARD;
	const auto right = pTrComponent ? pTrComponent->GetRight() : uvec::PRM_RIGHT;
	const auto rot = uquat::create(forward, right, up);
	for(auto &pair : feetData) {
		auto &footData = pair.second;
		if(footData.enabled == false) {
			SetIKControllerEnabled(pair.first, true);
			continue;
		}
		auto *ikController = hMdl->GetIKController(pair.first);
		auto &newUp = footData.upNormal;

		auto newForward = forward - uvec::project(forward, newUp);
		uvec::normalize(&newForward);
		auto newRight = right - uvec::project(right, newUp);
		newRight -= uvec::project(newRight, newForward);
		uvec::normalize(&newRight);

		auto rotNew = uquat::create(newForward, newRight, newUp);
		auto rotDelta = rotNew * uquat::get_inverse(rot);

		auto posBone = Vector3 {};
		auto rotBone = uquat::identity();
		animComponent->GetBonePose(footData.boneId, &posBone, &rotBone, nullptr, math::CoordinateSpace::Object);

		rotBone = rotDelta * footData.rotation;
		animComponent->GetBonePose(footData.boneId, &posBone, &rotBone, nullptr, math::CoordinateSpace::Object);
	}
}
