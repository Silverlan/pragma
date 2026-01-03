// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ai;

import :entities.components;
import :game;
import :scripting.lua;
import :server_state;

using namespace pragma;

void SAIComponent::_debugSendNavInfo(SPlayerComponent &pl)
{
	if(m_navInfo.pathInfo == nullptr)
		return;
	auto &path = *m_navInfo.pathInfo->path;

	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto nodePrev = pTrComponent->GetPosition();
	NetPacket p {};
	networking::write_entity(p, &ent);
	p->Write<uint32_t>(path.pathCount);
	p->Write<uint32_t>(m_navInfo.pathInfo->pathIdx);
	for(auto i = decltype(path.pathCount) {0}; i < path.pathCount; ++i) {
		Vector3 node {};
		path.GetNode(i, nodePrev, node);
		p->Write<Vector3>(node);
		nodePrev = node;
	}
	auto *session = pl.GetClientSession();
	if(session)
		ServerState::Get()->SendPacket(networking::net_messages::client::DEBUG_AI_NAVIGATION, p, networking::Protocol::SlowReliable, *session);
}

void SAIComponent::_debugSendScheduleInfo(SPlayerComponent &pl, std::shared_ptr<debug::DebugBehaviorTreeNode> &dbgTree, std::shared_ptr<ai::Schedule> &aiSchedule, float &tLastSchedUpdate)
{
	auto sched = GetCurrentSchedule();
	if(sched == nullptr) {
		if(aiSchedule != nullptr) {
			auto tDelta = SGame::Get()->CurTime() - tLastSchedUpdate;
			if(tDelta >= 4.f) // Only actually clear schedule if schedule has been finished for at least 4 seconds
			{
				NetPacket p {};
				p->Write<uint8_t>(static_cast<uint8_t>(0));
				auto *session = pl.GetClientSession();
				if(session)
					ServerState::Get()->SendPacket(networking::net_messages::client::DEBUG_AI_SCHEDULE_TREE, p, networking::Protocol::SlowReliable, *session);
				aiSchedule = nullptr;
			}
		}
		return;
	}

	tLastSchedUpdate = SGame::Get()->CurTime();
	NetPacket p {};
	auto bFullUpdate = (sched != aiSchedule) ? true : false;
	if(bFullUpdate == false) {
		auto tCur = static_cast<float>(SGame::Get()->CurTime());
		std::function<int8_t(debug::DebugBehaviorTreeNode &, const ai::BehaviorNode &, float)> fMarkChangedNodes = nullptr;
		fMarkChangedNodes = [&fMarkChangedNodes](debug::DebugBehaviorTreeNode &dbgNode, const ai::BehaviorNode &taskNode, float t) -> int8_t {
			auto &childTaskNodes = taskNode.GetNodes();
			if(dbgNode.children.size() != childTaskNodes.size())
				return -1;
			int8_t r = 0;
			auto &dbgInfo = taskNode.GetDebugInfo();
			for(auto i = decltype(dbgNode.children.size()) {0}; i < dbgNode.children.size(); ++i) {
				auto rChild = fMarkChangedNodes(*dbgNode.children[i], *childTaskNodes[i], t);
				if(rChild == -1)
					return -1;
				if(rChild == 1)
					r = 1;
			}
			if(dbgNode.state != static_cast<debug::DebugBehaviorTreeNode::State>(dbgInfo.lastResult) || dbgNode.active != taskNode.IsActive() || dbgNode.lastStartTime != dbgInfo.lastStartTime || dbgNode.lastEndTime != dbgInfo.lastEndTime
			  || dbgNode.executionIndex != dbgInfo.executionIndex) {
				dbgNode.state = static_cast<debug::DebugBehaviorTreeNode::State>(dbgInfo.lastResult);
				dbgNode.active = taskNode.IsActive();
				dbgNode.lastStartTime = dbgInfo.lastStartTime;
				dbgNode.lastEndTime = dbgInfo.lastEndTime;
				dbgNode.executionIndex = dbgInfo.executionIndex;
				dbgNode.lastUpdate = t;
				r = 1;
			}
			else if(r == 1)
				dbgNode.lastUpdate = t;
			return r;
		};
		auto r = fMarkChangedNodes(*dbgTree, sched->GetRootNode(), tCur);
		if(r == -1)
			bFullUpdate = true;
		else {
			if(r == 0) // No update required
				return;
			p->Write<uint8_t>(2);

			std::function<void(NetPacket &, debug::DebugBehaviorTreeNode &, float)> fWriteChanges = nullptr;
			fWriteChanges = [&fWriteChanges](NetPacket &p, debug::DebugBehaviorTreeNode &dbgNode, float t) {
				if(dbgNode.lastUpdate != t)
					p->Write<debug::DebugBehaviorTreeNode::State>(debug::DebugBehaviorTreeNode::State::Invalid); // Keep old state; Skip entire branch
				else {
					p->Write<debug::DebugBehaviorTreeNode::State>(dbgNode.state);
					p->Write<bool>(dbgNode.active);
					p->Write<float>(dbgNode.lastStartTime);
					p->Write<float>(dbgNode.lastEndTime);
					p->Write<uint64_t>(dbgNode.executionIndex);
					for(auto &child : dbgNode.children)
						fWriteChanges(p, *child, t);
				}
			};
			fWriteChanges(p, *dbgTree, tCur);
		}
	}
	if(bFullUpdate == true) {
		aiSchedule = sched;

		std::function<std::shared_ptr<debug::DebugBehaviorTreeNode>(const ai::BehaviorNode &)> fAddNode = nullptr;
		fAddNode = [&fAddNode, &sched](const ai::BehaviorNode &node) {
			auto &dbgInfo = node.GetDebugInfo();
			auto dbgChildNode = pragma::util::make_shared<debug::DebugBehaviorTreeNode>();
			auto *luaTask = dynamic_cast<const AILuaBehaviorNode *>(&node);
			if(dbgInfo.debugName.empty() == false)
				dbgChildNode->name = dbgInfo.debugName;
			else {
				std::stringstream ss;
				node.Print(sched.get(), ss);
				dbgChildNode->name = ss.str();
			}
			dbgChildNode->nodeType = static_cast<debug::DebugBehaviorTreeNode::BehaviorNodeType>(node.GetType());
			dbgChildNode->selectorType = static_cast<debug::DebugBehaviorTreeNode::SelectorType>(node.GetSelectorType());
			dbgChildNode->state = static_cast<debug::DebugBehaviorTreeNode::State>(dbgInfo.lastResult);
			dbgChildNode->active = node.IsActive();
			dbgChildNode->lastStartTime = dbgInfo.lastStartTime;
			dbgChildNode->lastEndTime = dbgInfo.lastEndTime;
			dbgChildNode->executionIndex = dbgInfo.executionIndex;

			for(auto &child : node.GetNodes())
				dbgChildNode->children.push_back(fAddNode(*child));
			return dbgChildNode;
		};
		auto dbgRootNode = fAddNode(sched->GetRootNode());

		p->Write<uint8_t>(1);
		*dbgTree = *dbgRootNode;
		networking::write_entity(p, &GetEntity());
		std::function<void(NetPacket &, const debug::DebugBehaviorTreeNode &)> fWriteTree = nullptr;
		fWriteTree = [&fWriteTree](NetPacket &p, const debug::DebugBehaviorTreeNode &node) {
			p->WriteString(node.name);
			p->Write<uint32_t>(math::to_integral(node.nodeType));
			p->Write<uint32_t>(math::to_integral(node.selectorType));
			p->Write<float>(node.lastStartTime);
			p->Write<float>(node.lastEndTime);
			p->Write<uint64_t>(node.executionIndex);
			p->Write<bool>(node.active);
			p->Write<debug::DebugBehaviorTreeNode::State>(node.state);
			p->Write<uint32_t>(node.children.size());
			for(auto &child : node.children)
				fWriteTree(p, *child);
		};
		fWriteTree(p, *dbgRootNode);

		dbgTree = dbgRootNode;
	}
	auto *session = pl.GetClientSession();
	if(session)
		ServerState::Get()->SendPacket(networking::net_messages::client::DEBUG_AI_SCHEDULE_TREE, p, networking::Protocol::SlowReliable, *session);
}
