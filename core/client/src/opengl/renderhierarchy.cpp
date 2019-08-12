#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/opengl/renderhierarchy.h"
#include <pragma/model/model.h>
#include "pragma/model/c_modelmesh.h"
#include <pragma/console/convars.h>
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.h"
#include <wgui/types/wirect.h>
#include "pragma/rendering/scene/scene.h"
#include "pragma/game/c_game_createguielement.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

RenderOctTree::RenderOctTree()
	: m_bShowOctTree(false),m_hGUIDebug(),
	m_debugFrustumBuffer(0),
	m_vertElementBuffer(0),m_debugCamera(NULL),m_nextIndex(0)
{
	m_root = CreateRenderNode();
}

RenderOctTree::~RenderOctTree()
{
	Clear();
}

unsigned int RenderOctTree::GetVertexIndexBuffer() {return m_vertElementBuffer;}

RenderNode *RenderOctTree::CreateRenderNode(const Vector3 &min,const Vector3 &max,RenderNode *parent)
{
	unsigned int idx;
	if(m_freeIndices.empty())
		idx = m_nextIndex++;
	else
	{
		idx = m_freeIndices.front();
		m_freeIndices.pop();
	}
	RenderNode *node = new RenderNode(this,idx,min,max,parent);
	auto numCallbacks = m_cbOnNodeCreated.size();
	for(size_t i=0;i<numCallbacks;i++)
	{
		CallbackHandle &hCb = m_cbOnNodeCreated[i];
		if(hCb.IsValid())
			hCb(node);
		else
		{
			m_cbOnNodeCreated.erase(m_cbOnNodeCreated.begin() +i);
			numCallbacks--;
			i--;
		}
	}
	return node;
}

RenderNode *RenderOctTree::CreateRenderNode() {return CreateRenderNode(Vector3(0.f,0.f,0.f),Vector3(0.f,0.f,0.f),NULL);}

void RenderOctTree::DestroyRenderNode(RenderNode *node)
{
	OnNodeDestroyed(node);
	m_freeIndices.push(node->GetIndex());
	auto numCallbacks = m_cbOnNodeDestroyed.size();
	for(size_t i=0;i<numCallbacks;i++)
	{
		CallbackHandle &hCb = m_cbOnNodeDestroyed[i];
		if(hCb.IsValid())
			hCb(node);
		else
		{
			m_cbOnNodeDestroyed.erase(m_cbOnNodeDestroyed.begin() +i);
			numCallbacks--;
			i--;
		}
	}
	delete node;
}

RenderNode *RenderOctTree::GetRoot() {return m_root;}
void RenderOctTree::Generate()
{
	std::vector<CBaseEntity*> ents;
	Generate(ents);
}
void RenderOctTree::AddToQueue(CBaseEntity *ent)
{
	RemoveFromTree(ent);
	m_root->AddToQueue(ent);
}
void RenderOctTree::RemoveFromTree(CBaseEntity *ent)
{
	std::unordered_map<CBaseEntity*,std::vector<NodeMeshInfo*>>::iterator it = m_entNodes.find(ent);
	if(it == m_entNodes.end())
		return;
	std::vector<NodeMeshInfo*> &nodeMeshes = it->second;
	auto numNodes = nodeMeshes.size();
	for(size_t i=0;i<numNodes;i++)
	{
		NodeMeshInfo *info = nodeMeshes[i];
		info->Remove();
	}
}
void RenderOctTree::AddNodeMesh(NodeMeshInfo *info)
{
	CBaseEntity *ent = info->GetEntity();
	std::unordered_map<CBaseEntity*,std::vector<NodeMeshInfo*>>::iterator it = m_entNodes.find(ent);
	if(it == m_entNodes.end())
		it = m_entNodes.insert(std::unordered_map<CBaseEntity*,std::vector<NodeMeshInfo*>>::value_type(ent,std::vector<NodeMeshInfo*>())).first;
	it->second.push_back(info);
}
void RenderOctTree::RemoveNodeMesh(NodeMeshInfo *info)
{
	CBaseEntity *ent = info->GetEntity();
	std::unordered_map<CBaseEntity*,std::vector<NodeMeshInfo*>>::iterator it = m_entNodes.find(ent);
	if(it == m_entNodes.end())
		return;
	std::vector<NodeMeshInfo*> &nodeMeshes = it->second;
	for(unsigned int i=0;i<nodeMeshes.size();i++)
	{
		if(nodeMeshes[i] == info)
		{
			nodeMeshes.erase(nodeMeshes.begin() +i);
			break;
		}
	}
	if(nodeMeshes.empty())
		m_entNodes.erase(it);
}
unsigned int RenderOctTree::GetMaxNodeCount() {return m_nextIndex;}
void RenderOctTree::Generate(std::vector<CBaseEntity*> &ents)
{
#if 0
	m_root->Clear(true);
	if(m_vertElementBuffer == 0)
		glGenBuffers(1,&m_vertElementBuffer);
	unsigned char elements[36] = {
		// front
		0,1,2,
		2,3,0,
		// top
		3,2,6,
		6,7,3,
		// back
		7,6,5,
		5,4,7,
		// bottom
		4,5,1,
		1,0,4,
		// left
		4,0,3,
		3,7,4,
		// right
		1,5,6,
		6,2,1
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_vertElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(unsigned char) *36,&elements[0],GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

	m_root->AddToQueue(ents);
	m_root->UpdateTree();
	m_root->Initialize();
	//m_root->Update(c_game->CurTime());
	//m_root->BuildTree();
	//Model *mdl = entWorld->GetModel();
	//if(mdl == NULL)
	//	return;
	// TODO: Re-add meshes if LOD changes?
	//Vector3 min;
	//Vector3 max;
	//entWorld->GetRenderBounds(&min,&max);
	//m_root = new RenderNode(min,max,entWorld);
#endif
}
void RenderOctTree::Clear()
{
#if 0
	ShowOctTree(false);
	if(m_root != NULL)
	{
		DestroyRenderNode(m_root);
		m_root = NULL;
	}
	if(m_vertElementBuffer != 0)
		glDeleteBuffers(1,&m_vertElementBuffer);
	m_entNodes.clear();
#endif
}

void RenderOctTree::CallOnNodeCreated(const CallbackHandle &hCb) {m_cbOnNodeCreated.push_back(hCb);}
void RenderOctTree::CallOnNodeDestroyed(const CallbackHandle &hCb) {m_cbOnNodeDestroyed.push_back(hCb);}

void RenderOctTree::OnNodeCreated(RenderNode *node)
{
	if(m_bShowOctTree == false)
		return;
	DebugRenderNodeInfo *info = new DebugRenderNodeInfo(node);
	m_debugInfo.push_back(info);
}
void RenderOctTree::OnNodeDestroyed(RenderNode *node)
{
	for(unsigned int i=0;i<m_debugInfo.size();i++)
	{
		if(m_debugInfo[i]->node == node)
		{
			delete m_debugInfo[i];
			m_debugInfo.erase(m_debugInfo.begin() +i);
			break;
		}
	}
}

void RenderOctTree::Update(double &t)
{
	if(m_root != NULL)
		m_root->Update(t);
}

void RenderOctTree::GetVisibleNodes(const Vector3 &origin,float radius,std::vector<RenderNode*> &nodes,std::vector<Plane> &planes,RenderNode *node)
{
	auto &max = m_root->GetMax();
	auto &min = m_root->GetMin();
	auto bIntersection = Intersection::AABBSphere(min,max,origin,radius);
	if(bIntersection == false)
		return;
	auto r = Intersection::SphereInPlaneMesh(origin,radius,planes);
	if(r == INTERSECT_OUTSIDE)
		return;
	nodes.push_back(node);
	for(unsigned char i=0;i<8;i++)
	{
		auto *child = node->GetChild(i);
		if(child != nullptr)
			GetVisibleNodes(origin,radius,nodes,planes,child);
	}
}

void RenderOctTree::GetVisibleNodes(const Vector3 &origin,float radius,std::vector<RenderNode*> &nodes,std::vector<Plane> &planes)
{
	if(m_root == nullptr)
		return;
	GetVisibleNodes(origin,radius,nodes,planes,m_root);
}

#if ROT_DEBUGGING_ENABLED != 0
void RenderOctTree::DebugPrint(RenderNode *node,std::vector<BaseEntity*> &ents,unsigned int &meshCount,unsigned int &numInvalid,unsigned int &numInvalidMeshes,unsigned int indent)
{
	Vector3 &min = node->GetMin();
	Vector3 &max = node->GetMax();
	std::string strIndent = "";
	for(unsigned int i=0;i<indent;i++)
		strIndent += "\t";
	Con::cout<<strIndent<<"Min: ("<<min.x<<","<<min.y<<","<<min.z<<")"<<Con::endl;
	Con::cout<<strIndent<<"Max: ("<<max.x<<","<<max.y<<","<<max.z<<")"<<Con::endl;
	auto center = (min +max) *0.5f;
	Con::cout<<strIndent<<"Center: ("<<center.x<<","<<center.y<<","<<center.z<<")"<<Con::endl;
	Vector3 bounds = max -min;
	Con::cout<<strIndent<<"Bounds: ("<<bounds.x<<","<<bounds.y<<","<<bounds.z<<")"<<Con::endl;
	std::vector<NodeMeshInfo*> &meshes = node->GetMeshInfo();
	Con::cout<<strIndent<<"Contained Meshes: "<<meshes.size()<<Con::endl;
	for(unsigned int i=0;i<meshes.size();i++)
	{
		NodeMeshInfo *info = meshes[i];
		CBaseEntity *ent = info->GetEntity();
		if(ent != NULL)
		{
			ModelMesh *mesh = info->GetMesh();
			auto &subMeshes = mesh->GetSubMeshes();
			std::string texture;
			if(!subMeshes.empty())
			{
				auto *subMesh = subMeshes.front().get();
				auto textureId = subMesh->GetTexture();
				auto mdlComponent = ent->GetModelComponent();
				auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
				if(mdl != nullptr)
				{
					auto &textures = mdl->GetTextures();
					if(textureId < textures.size())
						texture = textures[textureId];
				}
			}
			Con::cout<<strIndent<<"\t"<<ent->GetClass()<<" ("<<ent->GetIndex()<<")";
			if(!texture.empty())
				Con::cout<<" ("<<texture<<")";
			//auto &pos = ent->GetPosition();
			//std::cout<<" ("<<pos.x<<","<<pos.y<<","<<pos.z<<")";
			Vector3 minChild;
			Vector3 maxChild;
			info->GetBounds(&minChild,&maxChild);
			Con::cout<<" ("<<minChild.x<<","<<minChild.y<<","<<minChild.z<<") ("<<maxChild.x<<","<<maxChild.y<<","<<maxChild.z<<")";
			Con::cout<<Con::endl;
			meshCount++;
			bool bExists = false;
			for(unsigned int j=0;j<ents.size();j++)
			{
				if(ent == ents[j])
				{
					bExists = true;
					break;
				}
			}
			ents.push_back(ent);
			if(!Intersection::AABBInAABB(minChild,maxChild,min,max))
				numInvalidMeshes++;
		}
	}
	Con::cout<<strIndent<<"Child Nodes:"<<Con::endl;
	for(unsigned char i=0;i<node->GetChildCount();i++)
	{
		RenderNode *child = node->GetChild(i);
		if(child != NULL)
		{
			Vector3 minChild;
			Vector3 maxChild;
			child->GetBounds(&minChild,&maxChild);
			if(!Intersection::AABBInAABB(minChild,maxChild,min,max))
				numInvalid++;
			DebugPrint(child,ents,meshCount,numInvalid,numInvalidMeshes,indent +1);
		}
	}
}

void RenderOctTree::DebugPrint()
{
	Con::cout<<"------ OctTree Hierarchy ------"<<Con::endl;
	unsigned int meshCount = 0;
	unsigned int numInvalid = 0;
	unsigned int numInvalidMeshes = 0;
	std::vector<BaseEntity*> ents;
	DebugPrint(m_root,ents,meshCount,numInvalid,numInvalidMeshes);
	Con::cout<<"Total Mesh Count: "<<meshCount<<Con::endl;
	auto numEnts = ents.size();
	Con::cout<<"Total Entity Count: "<<ents.size()<<Con::endl;
	float percent = 0.f;
	std::vector<BaseEntity*> *gameEnts;
	c_game->GetEntities(&gameEnts);
	auto numGameEnts = gameEnts->size();
	if(numGameEnts > 0)
		percent = float(numEnts) /float(numGameEnts);
	Con::cout<<"Entity Percentage: "<<(percent *100.f)<<"%"<<Con::endl;
	Con::cout<<"Invalid Nodes: "<<numInvalid<<Con::endl;
	Con::cout<<"Invalid Meshes: "<<numInvalidMeshes<<Con::endl;
	Con::cout<<"Missing Entities:"<<Con::endl;
	for(unsigned int i=0;i<gameEnts->size();i++)
	{
		BaseEntity *ent = (*gameEnts)[i];
		bool bExists = false;
		for(unsigned int j=0;j<ents.size();j++)
		{
			if(ent == ents[j])
			{
				bExists = true;
				break;
			}
		}
		if(bExists == false)
			Con::cout<<"\t"<<ent->GetClass()<<" ("<<ent->GetIndex()<<")"<<Con::endl;
	}
	Con::cout<<"-------------------------------"<<Con::endl;
}
#endif

unsigned int RenderOctTree::GetDebugFrameBuffer()
{
#if 0
	if(!m_debugFrameBuffer.IsValid())
		return 0;
	return m_debugFrameBuffer->GetFrameBufferID();
#endif
	return 0;
}

void RenderOctTree::AddDebugRenderCallback(const CallbackHandle &hCb) {m_debugRenderCallbacks.push_back(hCb);}

void RenderOctTree::DebugRender()
{
	if(m_bShowOctTree == false)
		return;
	/*static ShaderDebug *shader = static_cast<ShaderDebug*>(c_game->GetShader("debug"));
	if(shader == NULL)
		return;
	Vector4 color = Vector4(1.f,1.f,1.f,1.f);
	for(unsigned int i=0;i<m_debugInfo.size();i++)
	{
		DebugRenderNodeInfo *info = m_debugInfo[i];
		shader->Render(info->vertexBuffer,info->vertexCount,GL_LINES,color);
	}
	m_debugFrameBuffer->Bind();
	OpenGL::ClearColor(0,0,0,1);
	m_debugFrameBuffer->Clear();

	Scene *scene = c_engine->GetScene(0);
	Camera *camScene = &scene->camera;

	std::vector<Vector3> points;
	Vector3 viewDir = camScene->GetForward();
	viewDir.y = 0.f;
	uvec::normalize(&viewDir);
	camScene->GetFrustumPoints(&points,camScene->GetZNear(),camScene->GetZFar(),camScene->GetFOVRad(),camScene->GetAspectRatio(),camScene->GetPos(),viewDir);

	Vector3 verts[4] = {
		points[static_cast<int>(FrustumPoint::NearTopLeft)],
		points[static_cast<int>(FrustumPoint::FarTopLeft)],
		points[static_cast<int>(FrustumPoint::FarTopRight)],
		points[static_cast<int>(FrustumPoint::NearTopRight)]
	};
	verts[1].y = verts[0].y;
	verts[2].y = verts[0].y;
	verts[3].y = verts[0].y;
	OpenGL::BindBuffer(m_debugFrustumBuffer,GL_ARRAY_BUFFER);
	OpenGL::BindBufferSubData(0,sizeof(Vector3) *4,&verts[0][0],GL_ARRAY_BUFFER);

	Vector3 debugPos = camScene->GetPos();
	debugPos.x += 2048.f;
	debugPos.z -= 2048.f;
	debugPos.y -= 2048.f;
	m_debugCamera->SetPos(debugPos);
	m_debugCamera->UpdateViewMatrix();

	Camera *cam = c_game->GetRenderTarget();
	c_game->SetRenderTarget(m_debugCamera);

	c_game->BindRenderTarget(m_debugCamera);
	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	OpenGL::SetViewPort(0,0,256,256);
	for(unsigned int i=0;i<m_debugInfo.size();i++)
	{
		DebugRenderNodeInfo *info = m_debugInfo[i];
		shader->Render(info->vertexBuffer,info->vertexCount,GL_LINES,color);
	}
	OpenGL::Enable(GL_BLEND);
	shader->Render(m_debugFrustumBuffer,4,GL_QUADS,Vector4(1.f,0.f,0.f,0.5f));
	OpenGL::Disable(GL_BLEND);
	auto numCallbacks = m_debugRenderCallbacks.size();
	for(size_t i=0;i<numCallbacks;i++)
	{
		CallbackHandle &hCb = m_debugRenderCallbacks[i];
		if(hCb.IsValid())
			hCb();
		else
		{
			m_debugRenderCallbacks.erase(m_debugRenderCallbacks.begin() +i);
			i--;
			numCallbacks--;
		}
	}
	OpenGL::SetViewPort(0,0,w,h);
	c_game->SetRenderTarget(cam);*/ // Vulkan TODO
}

pragma::CCameraComponent *RenderOctTree::GetDebugCamera() {return m_debugCamera.get();}

void RenderOctTree::GetNodeDebugInfo(RenderNode *node)
{
	DebugRenderNodeInfo *info = new DebugRenderNodeInfo(node);
	m_debugInfo.push_back(info);
	for(unsigned char i=0;i<8;i++)
	{
		RenderNode *child = node->GetChild(i);
		if(child != NULL)
			GetNodeDebugInfo(child);
	}
}

void RenderOctTree::ShowOctTree(bool b)
{
#if 0
	if(b == m_bShowOctTree)
		return;
	if(b == false)
	{
		if(m_hGUIDebug.IsValid())
			m_hGUIDebug->Remove();
		OpenGL::DeleteBuffer(m_debugFrustumBuffer);
		m_debugCamera = nullptr;
		m_debugTexture = GLTexturePtr(nullptr);
		m_debugFrameBuffer = GLFrameBufferPtr(nullptr);
		for(unsigned int i=0;i<m_debugInfo.size();i++)
			delete m_debugInfo[i];
		m_debugInfo.clear();
		m_bShowOctTree = b;
		if(m_debugCallback.IsValid())
			m_debugCallback.Invalidate();
		return;
	}
	if(c_game == NULL)
		return;
	m_bShowOctTree = b;
	if(m_root != NULL)
		GetNodeDebugInfo(m_root);

	auto &scene = c_game->GetScene();
	auto &cam = scene->camera;

	m_debugCamera = Camera::Create(*cam);
	m_debugCamera->SetForward(Vector3(0.f,1.f,0.f));
	m_debugCamera->SetUp(Vector3(-1.f,0.f,0.f));
	auto matProj = glm::ortho(0.f,4096.f,4096.f,0.f,cam->GetZNear(),8192.f);
	m_debugCamera->SetProjectionMatrix(matProj);
	m_debugCamera->UpdateViewMatrix();

	//
	m_debugFrustumBuffer = OpenGL::GenerateBuffer();
	OpenGL::BindBuffer(m_debugFrustumBuffer,GL_ARRAY_BUFFER);
	OpenGL::BindBufferData(sizeof(Vector3) *4,NULL,GL_DYNAMIC_DRAW,GL_ARRAY_BUFFER);
	OpenGL::BindBuffer(0,GL_ARRAY_BUFFER);
	//

	unsigned int size = 256;
	m_debugFrameBuffer = GLFrameBufferPtr(GLFrameBuffer::Create());
	m_debugTexture = GLTexturePtr(GLTexture::Create(size,size));
	auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);

	m_debugFrameBuffer->Bind();
	m_debugTexture->Bind();
	m_debugTexture->SetParameter(GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	m_debugTexture->SetParameter(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	m_debugTexture->SetParameter(GL_TEXTURE_MAX_LEVEL,0);
	m_debugFrameBuffer->AttachTexture(m_debugTexture.get());
	OpenGL::BindTexture(0,GL_TEXTURE_2D);

	OpenGL::BindFrameBuffer(bufFramePrev);

	WIRect *bg = c_game->CreateGUIElement<WIRect>();
	bg->SetColor(0.f,0.f,0.f,1.f);
	bg->SetSize(Vector2i(size,size));
	bg->SetPos(Vector2i(0,0));

	WITexturedRect *rect = c_game->CreateGUIElement<WITexturedRect>(bg);
	rect->SetSize(Vector2i(size,size));
	rect->SetPos(Vector2i(0,0));
	//rect->SetTexture(m_debugTexture.get()); // Vulkan TODO
	m_hGUIDebug = bg->GetHandle();

	m_debugCallback = c_game->AddCallback("Render",FunctionCallback<>::Create(
		std::bind(&RenderOctTree::DebugRender,this)
	));
#endif
}

////////////////////////////////////

static const float MIN_SIZE = 1.f;

RenderNode::RenderNode(RenderOctTree *octTree,unsigned int idx)
	: RenderNode(octTree,idx,Vector3(0.f,0.f,0.f),Vector3(0.f,0.f,0.f))
{}

RenderNode::RenderNode(RenderOctTree *octTree,unsigned int idx,const Vector3 &min,const Vector3 &max,RenderNode *parent)
	: BoundingNode(min,max),OctTreeNode<8>(parent),m_index(idx),m_octTree(octTree),m_life(-1),m_bReady(false),m_bBuilt(false),
	m_maxLife(8),m_buffer(0)
{}

unsigned int RenderNode::GetIndex() {return m_index;}

void RenderNode::Clear(bool bKeepQueue)
{
	m_life = -1;
	m_bReady = false;
	m_bBuilt = false;
	m_maxLife = 8;
	if(bKeepQueue == false)
	{
		for(unsigned int i=0;i<m_queue.size();i++)
			delete m_queue[i];
		m_queue.clear();
	}
	for(unsigned int i=0;i<m_meshes.size();i++)
		delete m_meshes[i];
	m_meshes.clear();
}

bool RenderNode::IsLeaf() {return (m_meshes.size() <= 1) ? true : false;} //(m_activeNodes == 0) ? true : false;}
RenderNode *RenderNode::GetParent() {return static_cast<RenderNode*>(OctTreeNode<8>::GetParent());}
RenderNode *RenderNode::GetChild(unsigned char idx) {return static_cast<RenderNode*>(OctTreeNode<8>::GetChild(idx));}
/*
RenderNode::RenderNode(Vector3 &min,Vector3 &max,CBaseEntity *ent)
	: RenderNode(min,max)
{
	AddToQueue(ent);
}
RenderNode::RenderNode(Vector3 &min,Vector3 &max,std::vector<BaseEntity*> &ents)
	: RenderNode(min,max)
{
	AddToQueue(ents);
}*/
RenderNode::~RenderNode()
{
#if 0
	for(unsigned int i=0;i<m_queue.size();i++)
		delete m_queue[i];
	for(unsigned int i=0;i<m_meshes.size();i++)
		delete m_meshes[i];
	for(unsigned char i=0;i<8;i++)
	{
		if(m_children[i] != NULL)
		{
			m_octTree->DestroyRenderNode(GetChild(i));
			m_children[i] = NULL;
		}
	}
	if(m_buffer != 0)
		glDeleteBuffers(1,&m_buffer);
#endif
}

void RenderNode::Initialize() {m_octTree->OnNodeCreated(this);}

void RenderNode::Render()
{
	//unsigned int elBuffer; // TODO
	/*OpenGL::BindBuffer(m_buffer);
	OpenGL::SetVertexAttribData(
		SHADER_VERTEX_BUFFER_LOCATION,
		3,
		GL_FLOAT,
		GL_FALSE,
		(void*)0
	);*/ // Vulkan TODO
	// TODO indices? -> Use buffer
	//OpenGL::DrawElements(GL_TRIANGLES,12,GL_UNSIGNED_BYTE,indices);
	//OpenGL::DrawElements(unsigned int mode,int count,unsigned int type,void *indices)
	//OpenGL::DrawArrays(GL_TRIANGLES,0,4);
	// TODO Bind index buffer
}

void RenderNode::GenerateBuffer()
{
#if 0
	glGenBuffers(1,&m_buffer);
	Vector3 verts[8] = {
		Vector3(m_min.x,m_min.y,m_max.z),
		Vector3(m_max.x,m_min.y,m_max.z),
		m_max,
		Vector3(m_min.x,m_max.y,m_max.z),

		m_min,
		Vector3(m_max.x,m_min.y,m_min.z),
		Vector3(m_max.x,m_max.y,m_min.z),
		Vector3(m_min.x,m_max.y,m_min.z)
	};
	glBindBuffer(GL_ARRAY_BUFFER,m_buffer);
	glBufferData(GL_ARRAY_BUFFER,sizeof(Vector3) *8,&verts[0][0],GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
#endif
}

unsigned int RenderNode::GetBuffer()
{
	if(m_buffer == 0)
		GenerateBuffer();
	return m_buffer;
}

void RenderNode::AddToQueue(CBaseEntity *ent,ModelMesh *mesh)
{
	m_bReady = false;
	m_queue.push_back(new NodeMeshInfo(this,ent,mesh));
}
void RenderNode::AddToQueue(NodeMeshInfo *info)
{
	m_bReady = false;
	m_queue.push_back(info);
}
void RenderNode::AddToQueue(std::vector<NodeMeshInfo*> &infos)
{
	for(unsigned int i=0;i<infos.size();i++)
		AddToQueue(infos[i]);
}
void RenderNode::AddToQueue(CBaseEntity *ent,std::vector<ModelMesh*> &meshes)
{
	m_bReady = false;
	for(unsigned int i=0;i<meshes.size();i++)
		AddToQueue(ent,meshes[i]);//m_queue.push_back(new NodeMeshInfo(this,ent,meshes[i]));
}

void RenderNode::AddToQueue(CBaseEntity *ent)
{
	auto mdlComponent = ent->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	auto pRenderComponent = ent->GetRenderComponent();
	if(pRenderComponent.expired())
		return;
	auto &meshes = pRenderComponent->GetLODMeshes();
	for(unsigned int i=0;i<meshes.size();i++)
		AddToQueue(ent,meshes[i].get());
}
void RenderNode::AddToQueue(std::vector<CBaseEntity*> &ents)
{
	for(unsigned int i=0;i<ents.size();i++)
		AddToQueue(static_cast<CBaseEntity*>(ents[i]));
}

void RenderNode::FindEnclosingCube()
{
	if(!m_meshes.empty())
	{
		m_min = Vector3(std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max());
		m_max = Vector3(std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest());
		for(unsigned int i=0;i<m_meshes.size();i++)
		{
			NodeMeshInfo *info = m_meshes[i];
			Vector3 min;
			Vector3 max;
			info->GetBounds(&min,&max);
			uvec::min(&m_min,min);
			uvec::max(&m_max,max);
		}
	}
	else
	{
		m_min = Vector3(0.f,0.f,0.f);
		m_max = Vector3(0.f,0.f,0.f);
	}
}

void RenderNode::FindChildEnclosingCube()
{
	for(unsigned int i=0;i<8;i++)
	{
		auto *child = GetChild(CUChar(i));
		if(child != nullptr)
		{
			Vector3 minChild;
			Vector3 maxChild;
			child->GetBounds(&minChild,&maxChild);
			uvec::min(&m_min,minChild);
			uvec::max(&m_max,maxChild);
		}
	}
}

RenderOctTree *RenderNode::GetOctTree() {return m_octTree;}

void RenderNode::OnChildResized(RenderNode*)
{
	// Make sure to fit around all new child nodes
	/*Vector3 min;
	Vector3 max;
	child->GetBounds(&min,&max);
	uvec::min(&m_min,min);
	uvec::max(&m_max,max);*/
	//
}

void RenderNode::BuildTree()
{
	if(IsLeaf())
	{
		m_bBuilt = true; // TODO: Is this correct?
		m_bReady = true;
		return;
	}
	Vector3 minLast = m_min;
	Vector3 maxLast = m_max;

	Vector3 bounds = m_max -m_min;
	//if(bounds.x == 0.f && bounds.y == 0.f && bounds.z == 0.f) // TODO: We don't really need to recalculate the bounds if the world is static, do we? (Re-enable this with some modifications?)
	//{
		FindEnclosingCube();
		bounds = m_max -m_min;
	//}
	if(bounds.x <= MIN_SIZE && bounds.y <= MIN_SIZE && bounds.z <= MIN_SIZE)
		return;
	Vector3 halfBounds = bounds *0.5f;
	Vector3 center = m_min +halfBounds;
	Octant octants[8];
	FillOctants(center,&octants[0]);
	
	std::vector<NodeMeshInfo*> octList[8];
	for(auto it=m_meshes.begin();it!=m_meshes.end();)
	{
		NodeMeshInfo *info = *it;
		if(info->m_hEnt.IsValid())
		{
			Vector3 min;
			Vector3 max;
			info->GetBounds(&min,&max);
			auto bContinue = true;
			for(unsigned char j=0;j<8;j++)
			{
				Octant &octant = octants[j];
				if(Intersection::AABBInAABB(min,max,octant.min,octant.max) == true)
				{
					octList[j].push_back(info);
					it = m_meshes.erase(it);
					bContinue = false;
					break;
				}
			}
			if(bContinue == true)
				++it;
		}
		else
		{
			delete info;
			it = m_meshes.erase(it);
		}
	}
	for(unsigned int i=0;i<8;i++)
	{
		if(!octList[i].empty())
		{
			RenderNode *child = GetChild(CUChar(i));
			if(child == NULL)
			{
				child = static_cast<RenderNode*>(m_children[i] = m_octTree->CreateRenderNode(octants[i].min,octants[i].max,this));
				m_activeNodes |= (unsigned char)(1 <<i);
				child->Initialize();
			}
			child->AddToQueue(octList[i]);
			child->UpdateTree();
		}
	}
	m_bBuilt = true;
	m_bReady = true;
}

void RenderNode::GetMeshes(std::vector<ModelMesh*> &meshes)
{
	for(unsigned int i=0;i<m_meshes.size();i++)
	{
		NodeMeshInfo *info = m_meshes[i];
		meshes.push_back(info->m_mesh);
	}
}
std::vector<NodeMeshInfo*> &RenderNode::GetMeshInfo() {return m_meshes;}

void RenderNode::FillOctants(Vector3 &center,Octant *octants)
{
	octants[0] = Octant(m_min,center);
	octants[1] = Octant(Vector3(center.x,m_min.y,m_min.z),Vector3(m_max.x,center.y,center.z));
	octants[2] = Octant(Vector3(center.x,m_min.y,center.z),Vector3(m_max.x,center.y,m_max.z));
	octants[3] = Octant(Vector3(m_min.x,m_min.y,center.z),Vector3(center.x,center.y,m_max.z));
	octants[4] = Octant(Vector3(m_min.x,center.y,m_min.z),Vector3(center.x,m_max.y,center.z));
	octants[5] = Octant(Vector3(center.x,center.y,m_min.z),Vector3(m_max.x,m_max.y,center.z));
	octants[6] = Octant(center,m_max);
	octants[7] = Octant(Vector3(m_min.x,center.y,center.z),Vector3(center.x,m_max.y,m_max.z));
}

void RenderNode::AddMeshes(CBaseEntity *ent)
{
	auto mdlComponent = ent->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	auto pRenderComponent = ent->GetRenderComponent();
	if(pRenderComponent.expired())
		return;
	auto &meshes = pRenderComponent->GetLODMeshes();
	for(unsigned int i=0;i<meshes.size();i++)
		m_meshes.push_back(new NodeMeshInfo(this,ent,meshes[i].get()));
}

void RenderNode::AddMesh(CBaseEntity *ent,ModelMesh *mesh)
{
	auto mdlComponent = ent->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	m_meshes.push_back(new NodeMeshInfo(this,ent,mesh));
}

void RenderNode::RemoveMesh(NodeMeshInfo *info)
{
	for(unsigned int i=0;i<m_queue.size();i++)
	{
		if(m_queue[i] == info)
		{
			m_queue.erase(m_queue.begin() +i);
			break;
		}
	}
	for(unsigned int i=0;i<m_meshes.size();i++)
	{
		if(m_meshes[i] == info)
		{
			m_meshes.erase(m_meshes.begin() +i);
			break;
		}
	}
	delete info;
}

void RenderNode::SetMinSize(const Vector3 &min,const Vector3 &max)
{
	uvec::min(&m_min,min);
	uvec::max(&m_max,max);
}

bool RenderNode::Insert(CBaseEntity *ent,ModelMesh *mesh)
{
	auto mdlComponent = ent->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return true;
	auto pTrComponent = ent->GetTransformComponent();
	Vector3 min;
	Vector3 max;
	auto origin = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	mesh->GetBounds(min,max);
	min += origin;
	max += origin;
	SetMinSize(min,max); // TODO: Is this okay? Could lead to overlapping child nodes if Insert is called without an octant intersection check
	if(m_meshes.size() <= 1 && m_activeNodes == 0)
	{
		if(mesh != NULL)
			AddMesh(ent,mesh);
		else
			AddMeshes(ent);
		return true;
	}
	Vector3 bounds = m_max -m_min;
	if(bounds.x <= MIN_SIZE && bounds.y <= MIN_SIZE && bounds.z <= MIN_SIZE)
	{
		if(mesh != NULL)
			AddMesh(ent,mesh);
		else
			AddMeshes(ent);
		return true;
	}
	if(mesh == NULL)
	{
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent.valid())
		{
			std::vector<std::shared_ptr<ModelMesh>> &meshes = pRenderComponent->GetLODMeshes();
			for(unsigned int i=0;i<meshes.size();i++)
				Insert(ent,meshes[i].get());
		}
		return true;
	}
	Vector3 halfBounds = bounds *0.5f;
	Vector3 center = m_min +halfBounds;

	Octant octants[8];
	FillOctants(center,&octants[0]);
	//if(Intersection::AABBInAABB(min,max,m_min,m_max) == true)
	//{
		bool bInserted = false;
		for(unsigned int i=0;i<8;i++)
		{
			Octant &octant = octants[i];
			if(Intersection::AABBInAABB(min,max,octant.min,octant.max) == true)
			{
				RenderNode *child = GetChild(CUChar(i));
				if(child == NULL)
				{
					child = static_cast<RenderNode*>(m_children[i] = m_octTree->CreateRenderNode(octant.min,octant.max,this));
					child->Insert(ent,mesh);
					child->Initialize();
					m_activeNodes |= (unsigned char)(1 <<i);
				}
				else
				{
					//child->SetMinSize(min,max);
					if(child->Insert(ent,mesh) == false)
					{
#ifdef _DEBUG
						assert(("Failed to insert mesh into render node child!",false));
#else
						Con::cwar<<"WARNING: Failed to insert mesh into render node child!"<<Con::endl;
#endif
						Insert(ent,mesh); // TODO Is this correct?
						//child->BuildTree(); // TODO Required?
					}
				}
				bInserted = true;
				break;
			}
		}
		if(bInserted == false)
			AddMesh(ent,mesh);
		else
			FindChildEnclosingCube(); // TODO Checkme
		// TODO: Bounding Sphere?
	//}
	//else
	//	return false; // TODO Is this correct?
	return true;
}

void RenderNode::UpdateTree()
{
	if(!m_bBuilt)
	{
		for(unsigned int i=0;i<m_queue.size();i++)
			m_meshes.push_back(m_queue[i]);
		m_queue.clear();
		BuildTree();
	}
	else
	{
		for(unsigned int i=0;i<m_queue.size();i++)
		{
			NodeMeshInfo *info = m_queue[i];
			EntityHandle &hEnt = info->m_hEnt;
			if(hEnt.IsValid())
			{
				ModelMesh *mesh = info->m_mesh;
				if(Insert(hEnt.get<CBaseEntity>(),mesh) == false)
				{
#ifdef _DEBUG
					assert(("Mesh out of render node bounds!",false));
#else
					Con::cwar<<"WARNING: Mesh out of render node bounds! Attempting to insert into parent node..."<<Con::endl;
					if(m_parent == nullptr)
						Con::cerr<<"ERROR: No parent node available! Mesh won't be rendered!"<<Con::endl;
					else
					{
						auto *parent = static_cast<RenderNode*>(m_parent);
						parent->AddMesh(hEnt.get<CBaseEntity>(),mesh);
						parent->UpdateTree();
					}
#endif
				}
			}
			delete info;
		}
		m_queue.clear();
	}
	m_bReady = true;
}

void RenderNode::Update(double &t)
{
	if(m_bBuilt == true)
	{
		if(m_meshes.empty())
		{
			if(HasChildren() == false)
			{
				if(m_life == -1)
					m_life = m_maxLife;
				else if(m_life > 0)
					m_life--;
			}
		}
		else
		{
			if(m_life != -1)
			{
				if(m_maxLife <= 64)
					m_maxLife *= 2;
				m_life = -1;
			}
		}
		//std::vector<NodeMeshInfo*> movedMeshes;//(m_ents.size());
		std::vector<NodeMeshInfo*> movedMeshes;
		//memset(&movedMeshes[0],NULL,sizeof(CBaseEntity*) *movedMeshes.size());
		auto numMeshes = m_meshes.size();
		for(size_t i=0;i<numMeshes;i++)
		{
			NodeMeshInfo *info = m_meshes[i];
			EntityHandle &hEnt = info->m_hEnt;
			if(hEnt.IsValid())
			{
				CBaseEntity *ent = static_cast<CBaseEntity*>(hEnt.get());
				const auto changeFlags = BaseEntity::StateFlags::CollisionBoundsChanged | BaseEntity::StateFlags::PositionChanged | BaseEntity::StateFlags::RenderBoundsChanged | BaseEntity::StateFlags::RotationChanged;
				if((ent->GetStateFlags() &changeFlags) != BaseEntity::StateFlags::None && info->m_tLastUpdate < t)
				{
					info->m_tLastUpdate = t;
					movedMeshes.push_back(m_meshes[i]);
					m_meshes.erase(m_meshes.begin() +i);
					i--;
					numMeshes--;
				}
			}
			else
			{
				delete info;
				m_meshes.erase(m_meshes.begin() +i);
				i--;
				numMeshes--;
			}
		}
		for(int flags=m_activeNodes,index=0;flags>0;flags>>=1,index++)
		{
			if((flags &1) == 1)
			{
				Double t_c = CDouble(t);
				GetChild(CUChar(index))->Update(t_c);
			}
		}
		if(!movedMeshes.empty())
		{
			auto numEls = movedMeshes.size();
			for(size_t i=numEls -1;i!=size_t(-1);i--)
			{
				NodeMeshInfo *info = movedMeshes[i];
				CBaseEntity *ent = info->m_hEnt.get<CBaseEntity>();
				Vector3 min {};
				Vector3 max {};
				auto pRenderComponent = ent->GetRenderComponent();
				if(pRenderComponent.valid())
					pRenderComponent->GetRenderBounds(&min,&max);
				auto pTrComponent = ent->GetTransformComponent();
				auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
				min += pos;
				max += pos;
				RenderNode *node = this;
				// TODO: Check for bounding sphere?
				Vector3 nodeMin;
				Vector3 nodeMax;
				GetBounds(&nodeMin,&nodeMax);
				while(Intersection::AABBInAABB(min,max,nodeMin,nodeMax) == false)
				{
					if(node->m_parent != NULL)
					{
						node = static_cast<RenderNode*>(node->m_parent);
						node->GetBounds(&nodeMin,&nodeMax);
					}
					else
						break;
				}
				ModelMesh *mesh = info->m_mesh;
				delete info; // TODO
				node->Insert(ent,mesh);
			}
		}
		for(unsigned char flags=m_activeNodes,index=0;flags>0;flags>>=1,index++)
		{
			if((flags &1) == 1)
			{
				RenderNode *child = GetChild(index);
				if(child->m_life == 0)
				{
					m_octTree->DestroyRenderNode(child);
					m_children[index] = NULL;
					m_activeNodes ^= (unsigned char)(1 <<index);
				}
			}
		}
	}
	UpdateTree(); // TODO For Testing purposes
}

////////////////////////////////////

NodeMeshInfo::NodeMeshInfo(RenderNode *node,CBaseEntity *ent,ModelMesh *mesh)
	: m_hEnt(ent->GetHandle()),m_node(node),m_mesh(mesh),m_tLastUpdate(0.0)
{}

NodeMeshInfo::~NodeMeshInfo() {}

void NodeMeshInfo::Remove() {m_node->RemoveMesh(this);}
ModelMesh *NodeMeshInfo::GetMesh() {return m_mesh;}
CBaseEntity *NodeMeshInfo::GetEntity()
{
	if(!m_hEnt.IsValid())
		return NULL;
	return m_hEnt.get<CBaseEntity>();
}

bool NodeMeshInfo::IsValid() {return m_hEnt.IsValid();}

void NodeMeshInfo::GetBounds(Vector3 *min,Vector3 *max)
{
	if(!IsValid())
		return;
	CBaseEntity *ent = m_hEnt.get<CBaseEntity>();
	Vector3 rMin;
	Vector3 rMax;
	m_mesh->GetBounds(rMin,rMax);
	auto pTrComponent = ent->GetTransformComponent();
	auto origin = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	*min = origin +rMin;
	*max = origin +rMax;
}

////////////////////////////////////

DebugRenderNodeInfo::DebugRenderNodeInfo(RenderNode *pnode)
	: node(pnode),vertexCount(0)
{
	Vector3 min;
	Vector3 max;
	node->GetBounds(&min,&max);

	std::vector<Vector3> lines;
	lines.push_back(min);
	lines.push_back(Vector3(min.x,min.y,max.z));
	
	lines.push_back(min);
	lines.push_back(Vector3(max.x,min.y,min.z));

	lines.push_back(min);
	lines.push_back(Vector3(min.x,max.y,min.z));

	lines.push_back(Vector3(min.x,min.y,max.z));
	lines.push_back(Vector3(max.x,min.y,max.z));

	lines.push_back(Vector3(min.x,min.y,max.z));
	lines.push_back(Vector3(min.x,max.y,max.z));

	lines.push_back(Vector3(max.x,min.y,max.z));
	lines.push_back(Vector3(max.x,min.y,min.z));

	lines.push_back(Vector3(max.x,min.y,max.z));
	lines.push_back(max);

	lines.push_back(Vector3(max.x,min.y,min.z));
	lines.push_back(Vector3(max.x,max.y,min.z));

	lines.push_back(Vector3(min.x,max.y,min.z));
	lines.push_back(Vector3(min.x,max.y,max.z));

	lines.push_back(Vector3(min.x,max.y,min.z));
	lines.push_back(Vector3(max.x,max.y,min.z));

	lines.push_back(max);
	lines.push_back(Vector3(max.x,max.y,min.z));

	lines.push_back(max);
	lines.push_back(Vector3(min.x,max.y,max.z));

#if 0
	OpenGL::GenerateBuffers(1,&vertexBuffer);
	OpenGL::BindBuffer(vertexBuffer);
	OpenGL::BindBufferData(
		CInt32(sizeof(float) *lines.size() *3),
		&lines[0][0],
		GL_STATIC_DRAW
	);
	vertexCount = CUInt32(lines.size());
	OpenGL::BindBuffer(0);
#endif
}

DebugRenderNodeInfo::~DebugRenderNodeInfo()
{
#if 0
	OpenGL::DeleteBuffer(vertexBuffer);
#endif
}

DLLCLIENT void CMD_debug_render_octree_static_print(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	if(c_game == nullptr)
		return;
	auto *entWorld = c_game->GetWorld();
	if(entWorld == nullptr)
	{
		Con::cwar<<"WARNING: No world entity found!"<<Con::endl;
		return;
	}
	auto meshTree = static_cast<pragma::CWorldComponent*>(entWorld)->GetMeshTree();
	if(meshTree == nullptr)
	{
		Con::cwar<<"WARNING: World-entity has no octree!"<<Con::endl;
		return;
	}
	meshTree->DebugPrint();
}

DLLCLIENT void CMD_debug_render_octree_dynamic_print(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	if(c_game == nullptr)
		return;
	auto &scene = c_game->GetScene();
	auto &octree = scene->GetOcclusionOctree();
	octree.DebugPrint();
}

static void CVAR_CALLBACK_debug_render_octree_static_draw(NetworkState*,ConVar*,bool,bool val)
{
	if(c_game == nullptr)
		return;
	auto *entWorld = c_game->GetWorld();
	if(entWorld == nullptr)
	{
		Con::cwar<<"WARNING: No world entity found!"<<Con::endl;
		return;
	}
	auto meshTree = static_cast<pragma::CWorldComponent*>(entWorld)->GetMeshTree();
	if(meshTree == nullptr)
	{
		Con::cwar<<"WARNING: World-entity has no octree!"<<Con::endl;
		return;
	}
	meshTree->SetDebugModeEnabled(val);
	/*if(c_game == nullptr)
		return;
	auto mode = OcclusionCulling::GetMode();
	if(mode != OcclusionCulling::Mode::CHC)
	{
		std::cout<<"This command requires 'cl_render_occlusion_culling' to be set to '2'"<<std::endl;
		return;
	}
	auto *octTree = OcclusionCulling::GetRenderOctTree();
	if(octTree == nullptr)
		return;
	octTree->ShowOctTree(val);
	auto *chc = OcclusionCulling::GetCHC();
	if(chc == nullptr)
		return;
	chc->SetDrawDebugTexture(val);*/
}
REGISTER_CONVAR_CALLBACK_CL(debug_render_octree_static_draw,CVAR_CALLBACK_debug_render_octree_static_draw);

static void CVAR_CALLBACK_debug_render_octree_dynamic_draw(NetworkState*,ConVar*,bool,bool val)
{
	if(c_game == nullptr)
		return;
	auto &scene = c_game->GetScene();
	auto &octree = scene->GetOcclusionOctree();
	octree.SetDebugModeEnabled(val);
}
REGISTER_CONVAR_CALLBACK_CL(debug_render_octree_dynamic_draw,CVAR_CALLBACK_debug_render_octree_dynamic_draw);

