/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CHC_NEW_HPP__
#define __CHC_NEW_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"
#include <sharedutils/util_weak_handle.hpp>
#include <queue>
#include <vector>
#include <memory>

#define CHC_DEBUGGING_ENABLED 0

namespace pragma {
#pragma warning(push)
#pragma warning(disable : 4251)
	class CHC;
	class CHCNode;
	using WPCHCNode = util::WeakHandle<CHCNode>;
	using PCHCNode = std::shared_ptr<CHCNode>;
	class DLLCLIENT CHCNode : public std::enable_shared_from_this<CHCNode> {
	  public:
		friend CHC;
		bool IsVisible() const;
		bool IsLeaf() const;
		const std::vector<PCHCNode> &GetChildren() const;
	  protected:
		void SetVisible(bool bVisible);
		CHCNode *GetParent() const;
	  private:
		bool m_bVisible = false;
		mutable WPCHCNode m_parent = {};
	};

	class DLLCLIENT CHCQuery {
	  public:
		uint32_t GetVisiblePixels() const;
		CHCNode *GetNode() const;
		uint32_t GetQueryCount() const;
	};
	using PCHCQuery = std::shared_ptr<CHCQuery>;

	class DLLCLIENT CHC {
	  public:
		CHC(CCameraComponent &cam, const std::shared_ptr<BaseOcclusionOctree> &octree = nullptr);
		~CHC();
		void Reset(const std::shared_ptr<BaseOcclusionOctree> &octree);
#ifdef CHC_DEBUGGING_ENABLED
		void SetDrawDebugTexture(bool b);
		bool GetDrawDebugTexture();
#endif
		std::vector<pragma::OcclusionMeshInfo> &PerformCulling();
	  protected:
		CHCNode *GetNode(uint32_t idx);
	  private:
		PCHCNode m_rootNode = nullptr;
		std::queue<WPCHCNode> m_distanceQueue;
		std::queue<PCHCQuery> m_queryQueue;
		std::queue<WPCHCNode> m_visQueue;
		std::queue<WPCHCNode> m_invisQueue;
		std::vector<umath::Plane> m_frustumPlanes;
		std::vector<PCHCNode> m_nodes;
		util::WeakHandle<pragma::CCameraComponent> m_cam = {};
		void TraverseNode(CHCNode &cNode);
		void PullUpVisibility(CHCNode &cNode);
		void QueryPreviouslyInvisibleNode(CHCNode &cNode);
		bool InsideViewFrustum(CHCNode &cNode);
		bool QueryReasonable(CHCNode &cNode);
		void Render(CHCNode &cNode);
		bool WasVisible(CHCNode &cNode);
		void OnRenderNodeCreated(std::reference_wrapper<const BaseOcclusionOctree::Node> node);
		void OnRenderNodeDestroyed(std::reference_wrapper<const BaseOcclusionOctree::Node> node);
		void InitializeNodes(const BaseOcclusionOctree::Node &node);

		void IssueMultiQueries();
		void HandleReturnedQuery(CHCQuery &query);
		void IssueQuery(CHCNode &node);
		void UpdateFrustum();
	};
#pragma warning(pop)
};

#endif
