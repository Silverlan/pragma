// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :util.bsp_tree;

using namespace pragma::util;

std::shared_ptr<BSPTree> BSPTree::Create()
{
	auto tree = std::shared_ptr<BSPTree> {new BSPTree {}};
	tree->m_rootNode = tree->CreateNode().index;
	return tree;
}
BSPTree::Node &BSPTree::CreateNode()
{
	m_nodes.push_back({});
	auto &node = m_nodes.back();
	node.index = m_nodes.size() - 1;
	return node;
}

const BSPTree::Node *BSPTree::Node::GetChild(BSPTree &tree, uint8_t idx) { return &tree.m_nodes[children[idx]]; }

static void preprocess_bsp_data(BSPTree &bspTree, std::vector<std::vector<size_t>> &outClusterNodes, std::vector<std::vector<uint16_t>> &outClusterToClusterVisibility)
{
	auto numClusters = bspTree.GetClusterCount();
	auto &bspNodes = bspTree.GetNodes();

	// Pre-processing to speed up some calculations
	{
		// Nodes per cluster
		outClusterNodes.resize(numClusters);
		for(auto i = decltype(bspNodes.size()) {0u}; i < bspNodes.size(); ++i) {
			auto &bspNode = bspNodes.at(i);
			if(bspNode.cluster >= outClusterNodes.size())
				continue;
			auto &nodeList = outClusterNodes.at(bspNode.cluster);
			if(nodeList.size() == nodeList.capacity())
				nodeList.reserve(nodeList.size() * 1.5f + 100);
			nodeList.push_back(i);
		}

		// List of clusters visible from every other cluster
		outClusterToClusterVisibility.resize(numClusters);
		for(auto cluster0 = decltype(numClusters) {0u}; cluster0 < numClusters; ++cluster0) {
			auto &visibleClusters = outClusterToClusterVisibility.at(cluster0);
			for(auto cluster1 = decltype(numClusters) {0u}; cluster1 < numClusters; ++cluster1) {
				if(bspTree.IsClusterVisible(cluster0, cluster1) == false)
					continue;
				if(visibleClusters.size() == visibleClusters.capacity())
					visibleClusters.reserve(visibleClusters.size() * 1.5f + 50);
				visibleClusters.push_back(cluster1);
			}
		}
	}
	//
}

std::shared_ptr<BSPTree> BSPTree::Load(const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PBSP_IDENTIFIER) {
		outErr = "Incorrect format!";
		return nullptr;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return nullptr;
	}

	auto bspTree = Create();
	udm["nodes"].GetBlobData(bspTree->m_nodes);
	udm["rootNode"](bspTree->m_rootNode);
	udm["numClusters"](bspTree->m_clusterCount);

	auto &clusterVisibility = bspTree->GetClusterVisibility();
	udm["clusterVisibility"].GetBlobData(clusterVisibility);
	return bspTree;
}
bool BSPTree::Save(udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PBSP_IDENTIFIER);
	outData.SetAssetVersion(PBSP_VERSION);
	auto udm = *outData;
	udm["numClusters"] = m_clusterCount;
	udm["numNodes"] = m_nodes.size();
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [this](const Node &node) { return node.index == m_rootNode; });
	assert(it != m_nodes.end());
	if(it == m_nodes.end()) {
		outErr = "Root node missing from node list!";
		return false;
	}
	udm["rootNode"] = static_cast<uint32_t>(it - m_nodes.begin());
	udm["nodes"] = udm::compress_lz4_blob(m_nodes);

	auto &clusterVisibility = GetClusterVisibility();
	udm["clusterVisibility"] = udm::compress_lz4_blob(clusterVisibility);
	return true;
}

bool BSPTree::IsValid() const { return m_rootNode < m_nodes.size(); }
bool BSPTree::IsClusterVisible(uint16_t clusterSrc, uint16_t clusterDst) const
{
	auto bit = static_cast<uint64_t>(clusterSrc) * m_clusterCount + static_cast<uint64_t>(clusterDst);
	auto offset = bit / 8u;
	bit %= 8u;
	return offset < m_clusterVisibility.size() && (m_clusterVisibility.at(offset) & (1 << bit)) > 0u;
}
void BSPTree::UpdateVisibilityBounds()
{
	for(auto &node : m_nodes) {
		if(node.leaf == false)
			continue;
		UpdateVisibilityBounds(node);
	}
}
void BSPTree::UpdateVisibilityBounds(Node &node)
{
	auto minInit = Vector3 {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	auto maxInit = Vector3 {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
	node.minVisible = minInit;
	node.maxVisible = maxInit;
	for(auto &nodeOther : m_nodes) {
		if(!nodeOther.leaf || &nodeOther == &node || !IsClusterVisible(node.cluster, nodeOther.cluster))
			continue;
		uvec::to_min_max(node.minVisible, node.maxVisible, nodeOther.min, nodeOther.max);
	}
	if(node.minVisible == minInit)
		node.minVisible = {};
	if(node.maxVisible == maxInit)
		node.maxVisible = {};
}
const BSPTree::Node &BSPTree::GetRootNode() const { return const_cast<BSPTree *>(this)->GetRootNode(); }
BSPTree::Node &BSPTree::GetRootNode() { return m_nodes[m_rootNode]; }
const std::vector<BSPTree::Node> &BSPTree::GetNodes() const { return const_cast<BSPTree *>(this)->GetNodes(); }
std::vector<BSPTree::Node> &BSPTree::GetNodes() { return m_nodes; }
const std::vector<uint8_t> &BSPTree::GetClusterVisibility() const { return const_cast<BSPTree *>(this)->GetClusterVisibility(); }
std::vector<uint8_t> &BSPTree::GetClusterVisibility() { return m_clusterVisibility; }
uint64_t BSPTree::GetClusterCount() const { return m_clusterCount; }
void BSPTree::SetClusterCount(uint64_t numClusters) { m_clusterCount = numClusters; }

BSPTree::Node *BSPTree::FindLeafNode(Node &node, const Vector3 &point)
{
	if(node.leaf)
		return &node;
	const auto &n = node.plane.GetNormal();
	auto d = node.plane.GetDistance();
	auto v = point - n * static_cast<float>(d);
	auto dot = uvec::dot(v, n);
	if(dot >= 0.f)
		return FindLeafNode(m_nodes[node.children.at(0)], point);
	return FindLeafNode(m_nodes[node.children.at(1)], point);
}
BSPTree::Node *BSPTree::FindLeafNode(const Vector3 &pos) { return FindLeafNode(GetRootNode(), pos); }

void BSPTree::FindLeafNodesInAabb(Node &node, const std::array<Vector3, 8> &aabbPoints, std::vector<Node *> &outNodes)
{
	if(node.leaf) {
		outNodes.push_back(&node);
		return;
	}
	const auto &n = node.plane.GetNormal();
	auto d = node.plane.GetDistance();
	auto checkLeft = false;
	auto checkRight = false;
	for(auto &p : aabbPoints) {
		auto v = p - n * static_cast<float>(d);
		auto dot = uvec::dot(v, n);
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
		FindLeafNodesInAabb(m_nodes[node.children.at(0)], aabbPoints, outNodes);
	if(checkRight)
		FindLeafNodesInAabb(m_nodes[node.children.at(1)], aabbPoints, outNodes);
}
std::vector<BSPTree::Node *> BSPTree::FindLeafNodesInAabb(const Vector3 &min, const Vector3 &max)
{
	std::array<Vector3, 8> aabbPoints = {min, Vector3 {min.x, min.y, max.z}, Vector3 {min.x, max.y, min.z}, Vector3 {min.x, max.y, max.z}, Vector3 {max.x, min.y, min.z}, Vector3 {max.x, min.y, max.z}, Vector3 {max.x, max.y, min.z}, max};
	std::vector<Node *> nodes {};
	FindLeafNodesInAabb(GetRootNode(), aabbPoints, nodes);
	return nodes;
}

bool BSPTree::IsAabbVisibleInCluster(const Node &node, const std::array<Vector3, 8> &aabbPoints, ClusterIndex clusterIdx) const
{
	if(node.leaf)
		return IsClusterVisible(clusterIdx, node.cluster);
	const auto &n = node.plane.GetNormal();
	auto d = node.plane.GetDistance();
	auto checkLeft = false;
	auto checkRight = false;
	for(auto &p : aabbPoints) {
		auto v = p - n * static_cast<float>(d);
		auto dot = uvec::dot(v, n);
		if(dot == 0.f)
			continue;
		if(dot > 0.f)
			checkLeft = true;
		else
			checkRight = true;
		if(checkLeft && checkRight)
			break;
	}
	return ((checkLeft && IsAabbVisibleInCluster(m_nodes[node.children.at(0)], aabbPoints, clusterIdx)) || (checkRight && IsAabbVisibleInCluster(m_nodes[node.children.at(1)], aabbPoints, clusterIdx)));
}
bool BSPTree::IsAabbVisibleInCluster(const Vector3 &min, const Vector3 &max, ClusterIndex clusterIdx) const
{
	std::array<Vector3, 8> aabbPoints = {min, Vector3 {min.x, min.y, max.z}, Vector3 {min.x, max.y, min.z}, Vector3 {min.x, max.y, max.z}, Vector3 {max.x, min.y, min.z}, Vector3 {max.x, min.y, max.z}, Vector3 {max.x, max.y, min.z}, max};
	return IsAabbVisibleInCluster(const_cast<BSPTree *>(this)->GetRootNode(), aabbPoints, clusterIdx);
}
