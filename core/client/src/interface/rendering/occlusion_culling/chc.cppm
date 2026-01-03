// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.occlusion_culling.chc;

export import :entities.base_entity;
export import :model.mesh;
import pragma.gui;
export import :rendering.occlusion_culling.octree;

export {
	class CHC;
	class CHCNode;
	using CHCNodeHandle = pragma::util::TSharedHandle<CHCNode>;
};

struct DLLCLIENT CHCMeshInfo {
	CHCMeshInfo(pragma::ecs::CBaseEntity &ent, pragma::geometry::CModelMesh &mesh);
	pragma::geometry::CModelMesh *mesh;
	EntityHandle hEntity;
};

#pragma warning(push)
#pragma warning(disable : 4251)
export class DLLCLIENT CHCNode : public std::enable_shared_from_this<CHCNode> {
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

export class DLLCLIENT CHCQuery {
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

export namespace pragma {
	class CCameraComponent;
};
export class DLLCLIENT CHC {
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
	pragma::gui::WIHandle m_hGUIDebug;
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
	std::vector<pragma::math::Plane> m_frustumPlanes;
	std::vector<CHCMeshInfo> m_renderMeshes;
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
	std::vector<CHCMeshInfo> &PerformCulling();
};
#pragma warning(pop)
