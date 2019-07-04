#ifndef __RENDERHIERARCHY_H__
#define __RENDERHIERARCHY_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include "pragma/math/boundingnode.h"
#include "pragma/math/octtreenode.h"
#include <vector>
#include <sharedutils/functioncallback.h>
#include <unordered_map>

#undef min
#undef max

#define ROT_DEBUGGING_ENABLED 1

class EntityHandle;
class BaseEntity;
class CBaseEntity;
class ModelMesh;
class RenderOctTree;
class RenderNode;
class DLLCLIENT NodeMeshInfo
{
public:
	friend RenderNode;
protected:
	NodeMeshInfo(RenderNode *node,CBaseEntity *ent,ModelMesh *mesh);
	~NodeMeshInfo();
	EntityHandle m_hEnt = {};
	ModelMesh *m_mesh = nullptr;
	RenderNode *m_node = nullptr;
	double m_tLastUpdate = 0.0;
	bool IsValid();
public:
	ModelMesh *GetMesh();
	CBaseEntity *GetEntity();
	void GetBounds(Vector3 *min,Vector3 *max);
	void Remove();
};

struct DLLCLIENT Octant
{
	Octant(const Vector3 &pmin,const Vector3 &pmax)
		: min(pmin),max(pmax)
	{}
	Octant()
		: Octant(Vector3(0.f,0.f,0.f),Vector3(0.f,0.f,0.f))
	{}
	Vector3 min = {};
	Vector3 max = {};
};

class DLLCLIENT RenderNode;
struct DLLCLIENT DebugRenderNodeInfo
{
public:
	friend RenderOctTree;
private:
	DebugRenderNodeInfo(RenderNode *node);
	~DebugRenderNodeInfo();
	unsigned int vertexCount = 0u;
public:
	RenderNode *node = nullptr;
	unsigned int vertexBuffer = 0u;
};

class ModelMesh;
class DLLCLIENT RenderNode
	: public BoundingNode,public OctTreeNode<8>
{
public:
	friend NodeMeshInfo;
	friend RenderOctTree;
private:
	RenderNode(RenderOctTree *octTree,unsigned int idx,const Vector3 &min,const Vector3 &max,RenderNode *parent=NULL);
protected:
	RenderNode(RenderOctTree *octTree,unsigned int idx);
	~RenderNode();
	RenderOctTree *m_octTree = nullptr;
	std::vector<NodeMeshInfo*> m_queue;
	std::vector<NodeMeshInfo*> m_meshes;
	unsigned int m_index = 0u;
	int m_life = 0;
	int m_maxLife = 0;
	bool m_bReady = false;
	bool m_bBuilt = false;
	void BuildTree();
	bool Insert(CBaseEntity *ent,ModelMesh *mesh);
	void AddMeshes(CBaseEntity *ent);
	void AddMesh(CBaseEntity *ent,ModelMesh *mesh);
	void FillOctants(Vector3 &center,Octant *octants);
	void AddToQueue(NodeMeshInfo *info);
	void AddToQueue(std::vector<NodeMeshInfo*> &infos);
	void AddToQueue(CBaseEntity *ent,ModelMesh *mesh);
	void AddToQueue(CBaseEntity *ent,std::vector<ModelMesh*> &meshes);
	void UpdateTree();
	void FindEnclosingCube();
	void FindChildEnclosingCube();
	void Clear(bool bKeepQueue=false);
	void RemoveMesh(NodeMeshInfo *info);
	void OnChildResized(RenderNode *child);
	void SetMinSize(const Vector3 &min,const Vector3 &max);
	void Initialize();

    // OpenGL
    unsigned int m_buffer = 0u;
    void GenerateBuffer();
	void Render();
public:
	bool IsLeaf();
	void AddToQueue(CBaseEntity *ent);
	void AddToQueue(std::vector<CBaseEntity*> &ents);
	void Update(double &t);
	RenderNode *GetParent();
	RenderNode *GetChild(unsigned char idx);
	void GetMeshes(std::vector<ModelMesh*> &meshes);
	std::vector<NodeMeshInfo*> &GetMeshInfo();
	unsigned int GetIndex();
	RenderOctTree *GetOctTree();
    // OpenGL
    unsigned int GetBuffer();
};

class WIHandle;
class DLLCLIENT RenderOctTree
{
public:
	friend RenderNode;
	friend NodeMeshInfo;
private:
	RenderNode *m_root = nullptr;
	std::queue<unsigned int> m_freeIndices;
	unsigned int m_nextIndex = 0u;
	bool m_bShowOctTree = false;
	WIHandle m_hGUIDebug = {};
	unsigned int m_debugFrustumBuffer = 0u;
    unsigned int m_vertElementBuffer = 0u;
	util::WeakHandle<pragma::CCameraComponent> m_debugCamera = {};
	CallbackHandle m_debugCallback = {};
	std::vector<DebugRenderNodeInfo*> m_debugInfo;
	std::unordered_map<CBaseEntity*,std::vector<NodeMeshInfo*>> m_entNodes;
	std::vector<CallbackHandle> m_cbOnNodeCreated;
	std::vector<CallbackHandle> m_cbOnNodeDestroyed;
	std::vector<CallbackHandle> m_debugRenderCallbacks;
	void OnNodeCreated(RenderNode *node);
	void OnNodeDestroyed(RenderNode *node);
	void GetNodeDebugInfo(RenderNode *node);
#if ROT_DEBUGGING_ENABLED != 0
	void DebugPrint(RenderNode *node,std::vector<BaseEntity*> &ents,unsigned int &meshCount,unsigned int &numInvalid,unsigned int &numInvalidMeshes,unsigned int indent=0);
#endif
	RenderNode *CreateRenderNode(const Vector3 &min,const Vector3 &max,RenderNode *parent=NULL);
	RenderNode *CreateRenderNode();
	void DestroyRenderNode(RenderNode *node);
	void GetVisibleNodes(const Vector3 &origin,float radius,std::vector<RenderNode*> &nodes,std::vector<Plane> &planes,RenderNode *node);
protected:
	void AddNodeMesh(NodeMeshInfo *info);
	void RemoveNodeMesh(NodeMeshInfo *info);
	void DebugRender();
public:
	RenderOctTree();
	~RenderOctTree();
	void AddDebugRenderCallback(const CallbackHandle &hCb);
	pragma::CCameraComponent *GetDebugCamera();
	unsigned int GetVertexIndexBuffer();
	unsigned int GetDebugFrameBuffer();
	RenderNode *GetRoot();
	void GetVisibleNodes(const Vector3 &origin,float radius,std::vector<RenderNode*> &nodes,std::vector<Plane> &planes);
	void CallOnNodeCreated(const CallbackHandle &hCb);
	void CallOnNodeDestroyed(const CallbackHandle &hCb);
	void Generate(std::vector<CBaseEntity*> &ents);
	void Generate();
	void AddToQueue(CBaseEntity *ent);
	void RemoveFromTree(CBaseEntity *ent);
	void Clear();
	void ShowOctTree(bool b);
	void Update(double &t);
	unsigned int GetMaxNodeCount();
#if ROT_DEBUGGING_ENABLED != 0
	void DebugPrint();
#endif
};

#endif