#include "stdafx_shared.h"
#include "pragma/util/util_bsp_tree.hpp"
#include <util_bsp.hpp>

extern DLLENGINE Engine *engine;

using namespace util;

BSPTree BSPTree::Create(bsp::File &bsp)
{
	BSPTree tree {};
	auto &nodes = bsp.GetNodes();
	if(nodes.empty())
		return tree;
	tree.m_nodes.reserve(bsp.GetLeaves().size() +bsp.GetNodes().size());
	tree.m_rootNode = tree.CreateNode(bsp,0);

	auto &clusterVisibility = bsp.GetVisibilityData();
	auto numClusters = tree.m_clusterCount = clusterVisibility.size();
	auto numCompressedClusters = umath::pow2(numClusters);
	numCompressedClusters = numCompressedClusters /8u +((numCompressedClusters %8u) > 0u ? 1u : 0u);
	tree.m_clusterVisibility.resize(numCompressedClusters,0u);
	auto offset = 0ull; // Offset in bits(!)
	for(auto &cluster : clusterVisibility)
	{
		for(auto vis : cluster)
		{
			auto &visCompressed = tree.m_clusterVisibility.at(offset /8u);
			if(vis)
				visCompressed |= 1<<(offset %8u);
			++offset;
		}
	}
	return tree;
}
BSPTree BSPTree::Create()
{
	BSPTree tree {};
	tree.m_rootNode = tree.CreateNode();
	return tree;
}
std::shared_ptr<BSPTree::Node> BSPTree::CreateNode()
{
	auto r = std::make_shared<BSPTree::Node>();
	m_nodes.push_back(r);
	return r;
}
void BSPTree::Node::Initialize(bsp::File &bsp,const std::array<int16_t,3u> &minSrc,const std::array<int16_t,3u> &maxSrc,uint16_t firstFaceSrc,uint16_t faceCountSrc)
{
	firstFace = firstFaceSrc;
	numFaces = faceCountSrc;
	min = Vector3{static_cast<float>(minSrc.at(0u)),static_cast<float>(minSrc.at(1u)),static_cast<float>(minSrc.at(2u))};
	umath::swap(min.y,min.z);
	umath::negate(min.z);

	max = Vector3{static_cast<float>(maxSrc.at(0u)),static_cast<float>(maxSrc.at(1u)),static_cast<float>(maxSrc.at(2u))};
	umath::swap(max.y,max.z);
	umath::negate(max.z);
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

std::shared_ptr<BSPTree::Node> BSPTree::CreateLeaf(bsp::File &bsp,int32_t nodeIndex)
{
	auto &leaf = bsp.GetLeaves().at(nodeIndex);
	auto pNode = std::make_shared<Node>();
	pNode->leaf = true;
	pNode->cluster = leaf.cluster;
	pNode->originalNodeIndex = nodeIndex;
	pNode->Initialize(bsp,leaf.mins,leaf.maxs,leaf.firstleafface,leaf.numleaffaces);
	m_nodes.push_back(pNode);
	return pNode;
}

std::shared_ptr<BSPTree::Node> BSPTree::CreateNode(bsp::File &bsp,int32_t nodeIndex)
{
	auto &node = bsp.GetNodes().at(nodeIndex);
	auto pNode = std::make_shared<Node>();
	pNode->leaf = false;
	pNode->originalNodeIndex = nodeIndex;
	pNode->Initialize(bsp,node.mins,node.maxs,node.firstface,node.numfaces);

	auto &plane = bsp.GetPlanes().at(node.planenum);
	auto planeNormal = plane.normal;
	umath::swap(planeNormal.y,planeNormal.z);
	umath::negate(planeNormal.z);
	pNode->plane = Plane{planeNormal,plane.dist};

	auto i = 0u;
	for(auto childIdx : node.children)
	{
		if(childIdx >= 0)
		{
			auto nodeIdx = childIdx;
			pNode->children.at(i) = CreateNode(bsp,nodeIdx);
		}
		else
		{
			auto leafIdx = -1 -childIdx;
			pNode->children.at(i) = CreateLeaf(bsp,leafIdx);
		}
		++i;
	}
	m_nodes.push_back(pNode);
	return pNode;
}
