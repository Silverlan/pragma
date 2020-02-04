#ifndef __UTIL_BSP_TREE_HPP__
#define __UTIL_BSP_TREE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/math/plane.h"
#include <array>
#include <mathutil/uvec.h>

namespace bsp {class File;};
namespace util
{
	class DLLNETWORK BSPTree
	{
	public:
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
			uint16_t cluster = std::numeric_limits<uint16_t>::max();
			Vector3 minVisible = {}; // Min bounds encompassing entire visible area of this leaf
			Vector3 maxVisible = {}; // Max bounds encompassing entire visible area of this leaf

			// Only valid if this is a non-leaf node
			Plane plane = {};

			friend BSPTree;
		private:
			void Initialize(::bsp::File &bsp,const std::array<int16_t,3u> &min,const std::array<int16_t,3u> &max,uint16_t firstFace,uint16_t faceCount);
		};
		static BSPTree Create(::bsp::File &bsp);
		static BSPTree Create();
		bool IsValid() const;
		bool IsClusterVisible(uint16_t clusterSrc,uint16_t clusterDst) const;
		const Node &GetRootNode() const;
		Node &GetRootNode();
		const std::vector<std::shared_ptr<Node>> &GetNodes() const;
		const std::vector<uint8_t> &GetClusterVisibility() const;
		std::vector<uint8_t> &GetClusterVisibility();
		uint64_t GetClusterCount() const;
		void SetClusterCount(uint64_t numClusters);
		Node *FindLeafNode(const Vector3 &pos);
		std::vector<Node*> FindLeafNodesInAABB(const Vector3 &min,const Vector3 &max);

		std::shared_ptr<Node> CreateNode();
	private:
		BSPTree()=default;
		std::shared_ptr<Node> CreateNode(::bsp::File &bsp,int32_t nodeIndex);
		std::shared_ptr<Node> CreateLeaf(::bsp::File &bsp,int32_t nodeIndex);
		std::shared_ptr<Node> m_rootNode = nullptr;
		std::vector<std::shared_ptr<Node>> m_nodes = {};
		std::vector<uint8_t> m_clusterVisibility = {};
		uint64_t m_clusterCount = 0ull;
	};
};

#endif
