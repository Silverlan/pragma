/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CHC_HPP__
#define __CHC_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/types.hpp>
#include <sharedutils/def_handle.h>
#include <sharedutils/functioncallback.h>
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"
#include <wgui/wibase.h>
#include <wgui/wihandle.h>
#include <queue>

#define CHC_DEBUGGING_ENABLED 0

class CHC;
class CHCNode;
DECLARE_BASE_HANDLE(DLLCLIENT, CHCNode, CHCNode);
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT CHCNode {
  public:
	friend CHC;
  protected:
	CHCNode(CHC *chc, const BaseOcclusionOctree::Node &node);
	CHCNodeHandle m_handle;
	std::weak_ptr<const BaseOcclusionOctree::Node> m_node;
	CHC *m_chc;
	bool m_bVisible = false;
	uint64_t m_lastVisited = 0;
  public:
	~CHCNode();
	CHCNodeHandle GetHandle() const;
	void Render();
	bool IsLeaf() const;
	uint8_t GetChildCount() const;
	uint64_t GetLastVisited() const;
	bool IsVisible() const;
	CHCNode *GetParent();
	const CHCNode *GetChild(uint32_t idx) const;
	const BaseOcclusionOctree::Node *GetRenderNode() const;
	uint32_t GetIndex() const;
	void GetBounds(Vector3 &min, Vector3 &max) const;
	const Vector3 &GetMin() const;
	const Vector3 &GetMax() const;
};

class DLLCLIENT CHCQuery {
  public:
	friend CHC;
  protected:
	uint32_t m_queryId = 0;
	CHCNodeHandle m_hNode;
  public:
	CHCQuery(const CHCNode &node);
	~CHCQuery();
	bool IsValid() const;
	bool IsAvailable() const;
	void Run();
	bool GetResult(uint32_t &r) const;
	uint32_t GetResult() const;
};

class ModelMesh;
class CBaseEntity;
namespace pragma {
	class CCameraComponent;
};
class DLLCLIENT CHC {
  public:
	friend CHCNode;
  private:
#ifdef CHC_DEBUGGING_ENABLED
	// Overhead
	unsigned int m_debugFrustumBuffer;
	pragma::ComponentHandle<pragma::CCameraComponent> m_debugQueryDepthOverheadCamera = {};
	//
	unsigned int m_debugMeshVertexBuffer;
	bool m_bDrawDebugTexture;
	WIHandle m_hGUIDebug;
	CallbackHandle m_debugCallback;
	void DebugRender();
	void DebugRender(CHCNode *node, bool bVisible = false);
/*
	: m_cam(cam.shared_from_this()),m_frameID(0),m_octree(octree),
	m_cbOnNodeCreated(),m_cbOnNodeDestroyed()
#ifdef CHC_DEBUGGING_ENABLED
	,m_bDrawDebugTexture(false),m_debugQueryDepthTexture(nullptr),m_debugQueryDepthFrameBuffer(nullptr),m_hGUIDebug(),
	m_debugQueryDepthScreenTexture(nullptr),m_debugQueryDepthScreenFrameBuffer(nullptr),m_debugMeshVertexBuffer(0),m_debugCallback(),
	m_debugQueryDepthOverheadFrameBuffer(nullptr),m_debugQueryDepthOverheadScreenTexture(nullptr),m_debugQueryDepthOverheadCamera(nullptr),
	m_debugFrustumBuffer(0)
#endif
*/
#endif
	uint64_t m_frameID;
	std::weak_ptr<BaseOcclusionOctree> m_octree;
	std::queue<CHCNodeHandle> m_distanceQueue;
	std::queue<std::shared_ptr<CHCQuery>> m_queryQueue;
	std::queue<CHCNodeHandle> m_visQueue;
	std::queue<CHCNodeHandle> m_invisQueue;
	std::vector<umath::Plane> m_frustumPlanes;
	std::vector<pragma::OcclusionMeshInfo> m_renderMeshes;
	std::vector<std::shared_ptr<CHCNode>> m_nodes;
	CallbackHandle m_cbOnNodeCreated;
	CallbackHandle m_cbOnNodeDestroyed;
	pragma::ComponentHandle<pragma::CCameraComponent> m_cam {};
	void TraverseNode(CHCNode *cNode);
	void PullUpVisibility(CHCNode *cNode);
	void QueryPreviouslyInvisibleNode(CHCNode *cNode);
	bool InsideViewFrustum(CHCNode *cNode);
	bool QueryReasonable(CHCNode *cNode);
	void Render(CHCNode *cNode);
	bool WasVisible(CHCNode *cNode);
	void OnRenderNodeCreated(std::reference_wrapper<const BaseOcclusionOctree::Node> node);
	void OnRenderNodeDestroyed(std::reference_wrapper<const BaseOcclusionOctree::Node> node);
	void InitializeNodes(const BaseOcclusionOctree::Node &node);

	void IssueMultiQueries();
	void HandleReturnedQuery(CHCQuery *query);
	void IssueQuery(CHCNode *node);
	void UpdateFrustum();
  protected:
	CHCNode *GetNode(uint32_t idx);
  public:
	CHC(pragma::CCameraComponent &cam, const std::shared_ptr<BaseOcclusionOctree> &octree = nullptr);
	~CHC();
	void Reset(const std::shared_ptr<BaseOcclusionOctree> &octree);
#ifdef CHC_DEBUGGING_ENABLED
	void SetDrawDebugTexture(bool b);
	bool GetDrawDebugTexture();
#endif
	std::vector<pragma::OcclusionMeshInfo> &PerformCulling();
};
#pragma warning(pop)

#endif
