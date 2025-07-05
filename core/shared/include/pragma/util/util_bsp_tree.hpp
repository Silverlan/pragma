// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_BSP_TREE_HPP__
#define __UTIL_BSP_TREE_HPP__

#include "pragma/networkdefinitions.h"
#include <array>
#include <mathutil/uvec.h>
#include <mathutil/plane.hpp>

namespace udm {
	struct AssetData;
	using AssetDataArg = const AssetData &;
};
namespace util {
#pragma pack(push, 1)
	class DLLNETWORK BSPTree : public std::enable_shared_from_this<BSPTree> {
	  public:
		static constexpr uint32_t PBSP_VERSION = 1;
		static constexpr auto PBSP_IDENTIFIER = "PBSP";
		using ClusterIndex = uint16_t;
		using ChildIndex = uint32_t;
		struct DLLNETWORK Node {
			const Node *GetChild(BSPTree &tree, uint8_t idx);

			ChildIndex index = 0;
			std::array<ChildIndex, 2u> children = {};
			bool leaf = true;
			Vector3 min = {};
			Vector3 max = {};
			// The index (node index OR leaf index!) as originally in the BSP,
			// this does NOT correspond to the m_nodes array of the tree!
			int32_t originalNodeIndex = -1;

			// Note: These have a different meaning depending on whether this is a leaf node or not.
			// If it is a leaf node, these correspond to the leaf face index array, otherwise they correspond to
			// the global face array.
			int32_t firstFace = 0u;
			int32_t numFaces = 0u;

			// Only valid if this is a leaf node
			ClusterIndex cluster = std::numeric_limits<ClusterIndex>::max();
			Vector3 minVisible = {}; // Min bounds encompassing entire visible area of this leaf
			Vector3 maxVisible = {}; // Max bounds encompassing entire visible area of this leaf

			// Only valid if this is a non-leaf node
			umath::Plane plane = {};
		};
		static std::shared_ptr<BSPTree> Create();
		static std::shared_ptr<BSPTree> Load(const udm::AssetData &data, std::string &outErr);

		bool IsValid() const;
		bool IsClusterVisible(ClusterIndex clusterSrc, ClusterIndex clusterDst) const;
		const Node &GetRootNode() const;
		Node &GetRootNode();
		const std::vector<Node> &GetNodes() const;
		std::vector<Node> &GetNodes();
		const std::vector<uint8_t> &GetClusterVisibility() const;
		std::vector<uint8_t> &GetClusterVisibility();
		uint64_t GetClusterCount() const;
		void SetClusterCount(uint64_t numClusters);
		Node *FindLeafNode(const Vector3 &pos);
		std::vector<Node *> FindLeafNodesInAabb(const Vector3 &min, const Vector3 &max);
		bool IsAabbVisibleInCluster(const Vector3 &min, const Vector3 &max, ClusterIndex clusterIdx) const;
		void UpdateVisibilityBounds();

		bool Save(udm::AssetDataArg outData, std::string &outErr);
		Node &CreateNode();
	  protected:
		BSPTree() = default;
		void UpdateVisibilityBounds(BSPTree::Node &node);
		BSPTree::Node *FindLeafNode(BSPTree::Node &node, const Vector3 &point);
		void FindLeafNodesInAabb(BSPTree::Node &node, const std::array<Vector3, 8> &aabbPoints, std::vector<BSPTree::Node *> &outNodes);
		bool IsAabbVisibleInCluster(const BSPTree::Node &node, const std::array<Vector3, 8> &aabbPoints, BSPTree::ClusterIndex clusterIdx) const;
		ChildIndex m_rootNode = std::numeric_limits<ChildIndex>::max();
		std::vector<Node> m_nodes = {};
		std::vector<uint8_t> m_clusterVisibility = {};
		uint64_t m_clusterCount = 0ull;
		friend Node;
	};
#pragma pack(pop)
};

#endif
