/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_BSP_TREE_HPP__
#define __UTIL_BSP_TREE_HPP__

#include "pragma/networkdefinitions.h"
#include <array>
#include <mathutil/uvec.h>
#include <mathutil/plane.hpp>

namespace util
{
	class DLLNETWORK BSPTree
		: public std::enable_shared_from_this<BSPTree>
	{
	public:
		using ClusterIndex = uint16_t;
		struct Node
			: public std::enable_shared_from_this<Node>
		{
			std::array<std::shared_ptr<Node>,2u> children = {};
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

			friend BSPTree;
		};
		static std::shared_ptr<BSPTree> Create();
		bool IsValid() const;
		bool IsClusterVisible(ClusterIndex clusterSrc,ClusterIndex clusterDst) const;
		const Node &GetRootNode() const;
		Node &GetRootNode();
		const std::vector<std::shared_ptr<Node>> &GetNodes() const;
		const std::vector<uint8_t> &GetClusterVisibility() const;
		std::vector<uint8_t> &GetClusterVisibility();
		uint64_t GetClusterCount() const;
		void SetClusterCount(uint64_t numClusters);
		Node *FindLeafNode(const Vector3 &pos);
		std::vector<Node*> FindLeafNodesInAABB(const Vector3 &min,const Vector3 &max);
		bool IsAABBVisibleInCluster(const Vector3 &min,const Vector3 &max,ClusterIndex clusterIdx) const;

		std::shared_ptr<Node> CreateNode();
	protected:
		BSPTree()=default;
		std::shared_ptr<Node> m_rootNode = nullptr;
		std::vector<std::shared_ptr<Node>> m_nodes = {};
		std::vector<uint8_t> m_clusterVisibility = {};
		uint64_t m_clusterCount = 0ull;
	};
};

#endif
