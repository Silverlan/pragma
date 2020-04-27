/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/util/util_bsp_tree.hpp"

extern DLLENGINE Engine *engine;

using namespace util;

std::shared_ptr<BSPTree> BSPTree::Create()
{
	auto tree = std::shared_ptr<BSPTree>{new BSPTree{}};
	tree->m_rootNode = tree->CreateNode();
	return tree;
}
std::shared_ptr<BSPTree::Node> BSPTree::CreateNode()
{
	auto r = std::make_shared<BSPTree::Node>();
	m_nodes.push_back(r);
	return r;
}

bool BSPTree::IsValid() const {return m_rootNode != nullptr;}
bool BSPTree::IsClusterVisible(uint16_t clusterSrc,uint16_t clusterDst) const
{
	auto bit = static_cast<uint64_t>(clusterSrc) *m_clusterCount +static_cast<uint64_t>(clusterDst);
	auto offset = bit /8u;
	bit %= 8u;
	return offset < m_clusterVisibility.size() && (m_clusterVisibility.at(offset) &(1<<bit)) > 0u;
}
const BSPTree::Node &BSPTree::GetRootNode() const {return const_cast<BSPTree*>(this)->GetRootNode();}
BSPTree::Node &BSPTree::GetRootNode() {return *m_rootNode;}
const std::vector<std::shared_ptr<BSPTree::Node>> &BSPTree::GetNodes() const {return m_nodes;}
const std::vector<uint8_t> &BSPTree::GetClusterVisibility() const {return const_cast<BSPTree*>(this)->GetClusterVisibility();}
std::vector<uint8_t> &BSPTree::GetClusterVisibility() {return m_clusterVisibility;}
uint64_t BSPTree::GetClusterCount() const {return m_clusterCount;}
void BSPTree::SetClusterCount(uint64_t numClusters) {m_clusterCount = numClusters;}

static BSPTree::Node *find_leaf_node(BSPTree::Node &node,const Vector3 &point)
{
	if(node.leaf)
		return &node;
	const auto &n = node.plane.GetNormal();
	auto d = node.plane.GetDistance();
	auto v = point -n *static_cast<float>(d);
	auto dot = uvec::dot(v,n);
	if(dot >= 0.f)
		return find_leaf_node(*node.children.at(0),point);
	return find_leaf_node(*node.children.at(1),point);
}
BSPTree::Node *BSPTree::FindLeafNode(const Vector3 &pos)
{
	return find_leaf_node(GetRootNode(),pos);
}

static void find_leaf_nodes_in_aabb(BSPTree::Node &node,const std::array<Vector3,8> &aabbPoints,std::vector<BSPTree::Node*> &outNodes)
{
	if(node.leaf)
	{
		outNodes.push_back(&node);
		return;
	}
	const auto &n = node.plane.GetNormal();
	auto d = node.plane.GetDistance();
	auto checkLeft = false;
	auto checkRight = false;
	for(auto &p : aabbPoints)
	{
		auto v = p -n *static_cast<float>(d);
		auto dot = uvec::dot(v,n);
		if(dot == 0.f)
			continue;
		if(dot > 0.f)
			checkLeft = true;
		else
			checkRight = true;
		if(checkLeft && checkRight)
			break;
	}
	if(checkLeft)
		find_leaf_nodes_in_aabb(*node.children.at(0),aabbPoints,outNodes);
	if(checkRight)
		find_leaf_nodes_in_aabb(*node.children.at(1),aabbPoints,outNodes);
}
std::vector<BSPTree::Node*> BSPTree::FindLeafNodesInAABB(const Vector3 &min,const Vector3 &max)
{
	std::array<Vector3,8> aabbPoints = {
		min,
		Vector3{min.x,min.y,max.z},
		Vector3{min.x,max.y,min.z},
		Vector3{min.x,max.y,max.z},
		Vector3{max.x,min.y,min.z},
		Vector3{max.x,min.y,max.z},
		Vector3{max.x,max.y,min.z},
		max
	};
	std::vector<BSPTree::Node*> nodes {};
	find_leaf_nodes_in_aabb(GetRootNode(),aabbPoints,nodes);
	return nodes;
}
