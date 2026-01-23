// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.occlusion_culling.chc;

import :engine;
import :game;

static const uint32_t maxPrevInvisNodeBatchSize = 50;

#ifdef CHC_DEBUGGING_ENABLED
static CHC *chc_debug_draw = nullptr;
#endif

CHC::CHC(pragma::CCameraComponent &cam, const std::shared_ptr<BaseOcclusionOctree> &octree)
    : m_cam(cam.GetHandle<pragma::CCameraComponent>()), m_frameID(0), m_octree(octree), m_cbOnNodeCreated(), m_cbOnNodeDestroyed()
#ifdef CHC_DEBUGGING_ENABLED
      ,
      m_bDrawDebugTexture(false), m_hGUIDebug(), m_debugMeshVertexBuffer(0), m_debugCallback(), m_debugQueryDepthOverheadCamera(nullptr), m_debugFrustumBuffer(0)
#endif
{
	Reset(octree);
}

CHC::~CHC()
{
#ifdef CHC_DEBUGGING_ENABLED
	SetDrawDebugTexture(false);
#endif
	Reset(nullptr);
}

void CHC::Reset(const std::shared_ptr<BaseOcclusionOctree> &octree)
{
	m_cbOnNodeCreated = CallbackHandle();
	m_cbOnNodeDestroyed = CallbackHandle();

	m_octree = octree;
	while(m_distanceQueue.empty() == false)
		m_distanceQueue.pop();
	while(m_visQueue.empty() == false)
		m_visQueue.pop();
	while(m_invisQueue.empty() == false)
		m_invisQueue.pop();
	m_nodes.clear();
	if(m_octree.expired() == true)
		return;
	m_cbOnNodeCreated = octree->AddNodeCreatedCallback(std::bind(&CHC::OnRenderNodeCreated, this, std::placeholders::_1));
	m_cbOnNodeDestroyed = octree->AddNodeDestroyedCallback(std::bind(&CHC::OnRenderNodeDestroyed, this, std::placeholders::_1));

	InitializeNodes(octree->GetRootNode());
}

void CHC::InitializeNodes(const BaseOcclusionOctree::Node &node)
{
	auto idx = node.GetIndex();
	if(idx >= m_nodes.size()) {
		auto maxNodes = idx + 1;
		maxNodes += 10; // Add several nodes as buffer
		auto numNodes = static_cast<uint32_t>(m_nodes.size());
		m_nodes.resize(maxNodes);
		memset(&m_nodes.at(numNodes), 0, (maxNodes - numNodes) * sizeof(CHCNode *));
	}
	auto &dstNode = m_nodes.at(idx);
	if(dstNode != nullptr) {
		if(dstNode->m_node.expired() == false && dstNode->m_node.lock().get() == &node)
			return;
		m_nodes.at(idx) = nullptr;
	}
	m_nodes.at(idx) = std::shared_ptr<CHCNode>(new CHCNode(this, node));
}

void CHC::OnRenderNodeCreated(std::reference_wrapper<const BaseOcclusionOctree::Node> node)
{
	auto *octree = node.get().GetTree();
	auto numNodes = static_cast<uint32_t>(m_nodes.size());
	auto maxNodes = octree->GetMaxNodeCount();
	if(maxNodes > numNodes) {
		maxNodes += 10; // Add several nodes as buffer
		m_nodes.resize(maxNodes);
		memset(&m_nodes[numNodes], 0, (maxNodes - numNodes) * sizeof(CHCNode *));
	}
	auto idx = node.get().GetIndex();
	auto &dstNode = m_nodes.at(idx);
	if(dstNode != nullptr) {
		if(dstNode->m_node.expired() == false && dstNode->m_node.lock().get() == &node.get())
			return;
		m_nodes.at(idx) = nullptr;
	}
	m_nodes.at(idx) = std::shared_ptr<CHCNode>(new CHCNode(this, node));
}
void CHC::OnRenderNodeDestroyed(std::reference_wrapper<const BaseOcclusionOctree::Node> node)
{
	auto idx = node.get().GetIndex();
	if(idx >= m_nodes.size() || m_nodes[idx] == nullptr)
		return;
	m_nodes[idx] = nullptr;
}

#ifdef CHC_DEBUGGING_ENABLED
void CHC::DebugRender(CHCNode *, bool)
{
	/*static ShaderOcclusion *shader = static_cast<ShaderOcclusion*>(pragma::get_cgame()->GetShader("occlusion"));
	auto *renderNode = node->GetRenderNode();
	auto buffer = renderNode->GetBuffer();
	auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	glDepthMask(GL_TRUE);
	OpenGL::Enable(GL_CULL_FACE);
	unsigned int frameBuffer = chc_debug_draw->GetDebugQueryFrameBuffer();
	unsigned int texture = chc_debug_draw->GetDebugQueryDepthTexture();
	unsigned int elBuffer = chc_debug_draw->m_octree->GetVertexIndexBuffer();
	Camera *cam = pragma::get_cgame()->GetRenderTarget();
	OpenGL::BindFrameBuffer(frameBuffer);
	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	OpenGL::SetViewPort(0,0,256,256);
		shader->Shader3DTexturedBase::Render(buffer,elBuffer,36);
	glDepthMask(GL_FALSE);

	// Screen
	OpenGL::SetColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);
	static ShaderDebugDepthBuffer *shaderDepth = static_cast<ShaderDebugDepthBuffer*>(pragma::get_cgame()->GetShader("debugdepthbuffer"));
	unsigned int frameBufferScreen = chc_debug_draw->GetDebugQueryScreenFrameBuffer();
	//unsigned int textureScreen = chc_debug_draw->GetDebugQueryScreenDepthTexture();
	OpenGL::BindFrameBuffer(frameBufferScreen);
	shaderDepth->Render(GL_TEXTURE_2D,texture,pragma::get_cgame()->GetScreenVertexBuffer(),CUInt32(cam->GetZNear()),1024);//cam->GetZFar());

	OpenGL::SetColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	//

	// Overhead
	Camera *camDebug = chc_debug_draw->GetDebugQueryOverheadCamera();
	pragma::get_cgame()->SetRenderTarget(camDebug);
	pragma::get_cgame()->BindRenderTarget(camDebug);

	if(node->IsVisible())
		OpenGL::SetColorMask(GL_TRUE,GL_TRUE,(node->IsLeaf() == true) ? GL_FALSE : GL_TRUE,GL_FALSE);
	else
		OpenGL::SetColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
	static ShaderDebug *shaderDebug = static_cast<ShaderDebug*>(pragma::get_cgame()->GetShader("debug"));
	auto frameBufferOverhead = chc_debug_draw->GetDebugQueryOverheadFrameBuffer();
	OpenGL::BindFrameBuffer(frameBufferOverhead);
	shaderDebug->Render(buffer,elBuffer,36,GL_LINES,Vector4(1.f,1.f,1.f,1.f));
	pragma::get_cgame()->SetRenderTarget(cam);
	pragma::get_cgame()->BindRenderTarget(cam);
	OpenGL::SetColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	//
	OpenGL::Disable(GL_CULL_FACE);
	OpenGL::SetViewPort(0,0,w,h);
	OpenGL::BindFrameBuffer(bufFramePrev);
	shader->PrepareBatchRendering(elBuffer);

	for(unsigned char i=0;i<node->GetChildCount();i++)
	{
		auto *child = node->GetChild(i);
		if(child != nullptr)
			DebugRender(child,(bVisible == true && WasVisible(child) == true) ? true : false);
	}*/ // Vulkan TODO
}
void CHC::DebugRender()
{
	/*OpenGL::Enable(GL_BLEND);
	unsigned int indexBuffer = m_octree->GetVertexIndexBuffer();

	Vector4 col = Vector4(0.f,0.78f,1.f,0.3f);
	Vector4 colWorld = Vector4(0.5f,0.f,1.f,0.2f);
	for(unsigned int i=0;i<m_renderMeshes.size();i++)
	{
		auto &info = m_renderMeshes[i];
		if(info.hEntity.IsValid())
		{
			CBaseEntity *ent = static_cast<pragma::ecs::CBaseEntity*>(info.hEntity.get());
			Vector3 &origin = ent->GetPosition();
			Vector3 min;
			Vector3 max;
			info.mesh->GetBounds(&min,&max);
			min += origin;
			max += origin;

			Vector3 verts[8] = {
				Vector3(min.x,min.y,max.z),
				Vector3(max.x,min.y,max.z),
				max,
				Vector3(min.x,max.y,max.z),

				min,
				Vector3(max.x,min.y,min.z),
				Vector3(max.x,max.y,min.z),
				Vector3(min.x,max.y,min.z)
			};
			OpenGL::BindBuffer(m_debugMeshVertexBuffer);
			OpenGL::BindBufferSubData(0,sizeof(Vector3) *8,&verts[0][0]);

			static ShaderDebug *shader = static_cast<ShaderDebug*>(pragma::get_cgame()->GetShader("debug"));
			shader->Render(m_debugMeshVertexBuffer,indexBuffer,36,GL_TRIANGLES,ent->IsWorld() ? colWorld : col);
		}
	}
	OpenGL::Disable(GL_BLEND);*/ // Vulkan TODO
}

bool CHC::GetDrawDebugTexture() { return m_bDrawDebugTexture; }
void CHC::SetDrawDebugTexture(bool b)
{
	/*m_bDrawDebugTexture = b;
	if(b == true)
	{
		if(!m_debugQueryDepthFrameBuffer.IsValid())
		{
			unsigned int size = 256;
			auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
			m_debugQueryDepthFrameBuffer = GLFrameBufferPtr(GLFrameBuffer::Create());
			m_debugQueryDepthFrameBuffer->Bind();

			unsigned int bufTexPrev = OpenGL::GetInt(GL_TEXTURE_BINDING_2D);
			m_debugQueryDepthTexture = GLTexturePtr(GLTexture::Create(size,size,GL_DEPTH_COMPONENT16,GL_DEPTH_COMPONENT,GL_FLOAT));

			m_debugQueryDepthFrameBuffer->AttachTexture(m_debugQueryDepthTexture.get(),GL_DEPTH_ATTACHMENT);
			m_debugQueryDepthTexture->SetParameter(GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			m_debugQueryDepthTexture->SetParameter(GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			m_debugQueryDepthTexture->SetParameter(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER);
			m_debugQueryDepthTexture->SetParameter(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
			m_debugQueryDepthTexture->SetParameter(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
			m_debugQueryDepthTexture->SetParameter(GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL);

			m_debugQueryDepthFrameBuffer->SetDrawColorBuffer(GL_NONE);
			m_debugQueryDepthFrameBuffer->SetColorBufferSource(GL_NONE);

			OpenGL::BindFrameBuffer(bufFramePrev);
			OpenGL::BindTexture(bufTexPrev,GL_TEXTURE_2D);

			// Overhead
			m_debugQueryDepthOverheadFrameBuffer = GLFrameBufferPtr(GLFrameBuffer::Create());
			m_debugQueryDepthOverheadScreenTexture = GLTexturePtr(GLTexture::Create(size,size));
			m_debugQueryDepthOverheadFrameBuffer->Bind();
			m_debugQueryDepthOverheadScreenTexture->Bind();
			m_debugQueryDepthOverheadScreenTexture->SetParameter(GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			m_debugQueryDepthOverheadScreenTexture->SetParameter(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			m_debugQueryDepthOverheadScreenTexture->SetParameter(GL_TEXTURE_MAX_LEVEL,0);

			m_debugQueryDepthOverheadFrameBuffer->AttachTexture(m_debugQueryDepthOverheadScreenTexture.get());

			auto &scene = pragma::get_cgame()->GetScene();
			auto &cam = scene->camera;
			m_debugQueryDepthOverheadCamera = Camera::Create(*cam.get());
			auto *debugCamera = m_debugQueryDepthOverheadCamera.get();
			debugCamera->SetForward(Vector3(0.f,1.f,0.f));
			debugCamera->SetUp(Vector3(-1.f,0.f,0.f));
			auto matProj = glm::gtc::ortho(0.f,8192.f,8192.f,0.f,cam->GetZNear(),32768.f);//glm::gtc::ortho(0.f,16384.f,16384.f,0.f,cam->GetZNear(),32768.f);
			debugCamera->SetProjectionMatrix(matProj);
			debugCamera->UpdateViewMatrix();

			m_debugFrustumBuffer = OpenGL::GenerateBuffer();
			OpenGL::BindBuffer(m_debugFrustumBuffer,GL_ARRAY_BUFFER);
			OpenGL::BindBufferData(sizeof(Vector3) *4,nullptr,GL_DYNAMIC_DRAW,GL_ARRAY_BUFFER);
			OpenGL::BindBuffer(0,GL_ARRAY_BUFFER);
			//

			// Screen Texture
			m_debugQueryDepthScreenFrameBuffer = GLFrameBufferPtr(GLFrameBuffer::Create());
			m_debugQueryDepthScreenTexture = GLTexturePtr(GLTexture::Create(size,size));
			m_debugQueryDepthScreenFrameBuffer->Bind();
			m_debugQueryDepthScreenTexture->Bind();
			m_debugQueryDepthScreenTexture->SetParameter(GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			m_debugQueryDepthScreenTexture->SetParameter(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			m_debugQueryDepthScreenTexture->SetParameter(GL_TEXTURE_MAX_LEVEL,0);

			m_debugQueryDepthFrameBuffer->AttachTexture(m_debugQueryDepthScreenTexture.get());
			OpenGL::BindTexture(0,GL_TEXTURE_2D);
			OpenGL::BindFrameBuffer(0);
			//

			WIRect *bg = pragma::get_cgame()->CreateGUIElement<WIRect>();
			bg->SetColor(0.f,0.f,0.f,1.f);
			bg->SetSize(Vector2i(size *2,size));
			bg->SetPos(Vector2i(size,0));

			WITexturedRect *rect = pragma::get_cgame()->CreateGUIElement<WITexturedRect>(bg);
			rect->SetSize(Vector2i(size,size));
			rect->SetPos(Vector2i(0,0));
			//rect->SetTexture(m_debugQueryDepthScreenTexture.get()); // Vulkan TODO

			WITexturedRect *rectOverhead = pragma::get_cgame()->CreateGUIElement<WITexturedRect>(bg);
			rectOverhead->SetSize(Vector2i(size,size));
			rectOverhead->SetPos(Vector2i(size,0));
			//rectOverhead->SetTexture(m_debugQueryDepthOverheadScreenTexture.get()); // Vulkan TODO
			m_hGUIDebug = bg->GetHandle();

			m_debugMeshVertexBuffer = OpenGL::GenerateBuffer();
			OpenGL::BindBuffer(m_debugMeshVertexBuffer);
			OpenGL::BindBufferData(sizeof(Vector3) *8,nullptr,GL_STREAM_DRAW);
			OpenGL::BindBuffer(0);

			m_debugCallback = FunctionCallback<>::Create(
				std::bind(static_cast<void(CHC::*)()>(&CHC::DebugRender),this)
			);
			//m_octree->AddDebugRenderCallback(m_debugCallback); // TODO
		}
	}
	else
	{
		if(m_hGUIDebug.IsValid())
			m_hGUIDebug->Remove();
		m_debugQueryDepthFrameBuffer = GLFrameBufferPtr(nullptr);
		m_debugQueryDepthTexture = GLTexturePtr(nullptr);
		m_debugQueryDepthScreenFrameBuffer = GLFrameBufferPtr(nullptr);
		m_debugQueryDepthScreenTexture = GLTexturePtr(nullptr);
		m_debugQueryDepthOverheadFrameBuffer = GLFrameBufferPtr(nullptr);
		m_debugQueryDepthOverheadScreenTexture = GLTexturePtr(nullptr);
		m_debugQueryDepthOverheadCamera = nullptr;
		OpenGL::DeleteBuffer(m_debugFrustumBuffer);
		m_debugFrustumBuffer = 0;
		if(m_debugCallback.IsValid())
			m_debugCallback.Invalidate();
		m_debugCallback = CallbackHandle();
	}*/
}
#endif

void CHC::UpdateFrustum()
{
	//bool bFogEnabled = FogController::IsFogEnabled();
	m_frustumPlanes.clear();
	m_cam->GetFrustumPlanes(m_frustumPlanes);
	/*if(bFogEnabled == true)
	{
		float fogDist = FogController::GetFarDistance();
		float farZ = m_cam->GetZFar();
		if(fogDist < farZ)
			farZ = fogDist;
		Plane &farPlane = m_frustumPlanes[static_cast<int>(FrustumPlane::Far)];
		Vector3 &start = m_frustumPlanes[static_cast<int>(FrustumPlane::Near)].GetCenterPos();
		Vector3 dir = farPlane.GetCenterPos() -start;
		uvec::normalize(&dir);
		farPlane.MoveToPos(start +dir *farZ); // TODO Checkme
	}*/
}

void CHC::TraverseNode(CHCNode *cNode)
{
	//if(cNode->IsLeaf()) // TODO We need to render all meshes in this node, even if it's not a leaf node!?
	Render(cNode);
	//else
	//{
	for(unsigned int i = 0; i < cNode->GetChildCount(); i++) {
		auto *child = cNode->GetChild(i);
		if(child != nullptr)
			m_distanceQueue.push(child->GetHandle());
	}
	if(!cNode->IsLeaf()) // TODO (Originally no condition)
		cNode->m_bVisible = false;
	//}
}

void CHC::Render(CHCNode *cNode)
{
	/*std::vector<NodeMeshInfo*> &meshes = cNode->m_node->GetMeshInfo();
	for(unsigned int i=0;i<meshes.size();i++)
	{
		NodeMeshInfo *info = meshes[i];
		m_renderMeshes.push_back(pragma::OcclusionMeshInfo(info->GetEntity(),static_cast<pragma::geometry::CModelMesh*>(info->GetMesh())));
	}*/ // TODO
}

void CHC::PullUpVisibility(CHCNode *cNode)
{
	while(cNode != nullptr && !cNode->m_bVisible) {
		cNode->m_bVisible = true;
		cNode = cNode->GetParent();
	}
}

void CHC::QueryPreviouslyInvisibleNode(CHCNode *cNode)
{
	m_invisQueue.push(cNode->GetHandle());
	if(m_invisQueue.size() > maxPrevInvisNodeBatchSize)
		IssueMultiQueries();
}

//static Vulkan::QueryPool pool = nullptr; // TODO
//static Vulkan::Buffer vertBuf = nullptr; // prosper TODO
static std::size_t vertCount = 0;
void CHC::IssueMultiQueries()
{
	// prosper TODO
#if 0
	static auto hShader = pragma::get_cengine()->GetShader("occlusion");
	if(hShader.IsValid() == false)
		return;
	if(m_invisQueue.empty())
		return;

		// Occlusion Query Test
	auto &context = pragma::get_cengine()->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();

	if(pool == nullptr)
	{
		//pool = Vulkan::QueryPool::Create(context,vk::QueryType::eOcclusion,100); // prosper TODO

		auto min = Vector3(-256,-256,-256);
		auto max = Vector3(256,256,256);
		std::vector<Vector3> meshVerts = {
			min,
			{min.x,min.y,max.z},
			{min.x,max.y,max.z},
			{max.x,max.y,min.z},
			min,
			{min.x,max.y,min.z},
			{max.x,min.y,max.z},
			min,
			{max.x,min.y,min.z},
			{max.x,max.y,min.z},
			{max.x,min.y,min.z},
			min,
			min,
			{min.x,max.y,max.z},
			{min.x,max.y,min.z},
			{max.x,min.y,max.z},
			{min.x,min.y,max.z},
			min,
			{min.x,max.y,max.z},
			{min.x,min.y,max.z},
			{max.x,min.y,max.z},
			max,
			{max.x,min.y,min.z},
			{max.x,max.y,min.z},
			{max.x,min.y,min.z},
			max,
			{max.x,min.y,max.z},
			max,
			{max.x,max.y,min.z},
			{min.x,max.y,min.z},
			max,
			{min.x,max.y,min.z},
			{min.x,max.y,max.z},
			max,
			{min.x,max.y,max.z},
			{max.x,min.y,max.z}
		};
		vertCount = meshVerts.size();
		//vertBuf = Vulkan::Buffer::Create(context,prosper::BufferUsageFlags::VertexBufferBit,meshVerts.size() *sizeof(Vector3),meshVerts.data()); // prosper TODO
	}
	else
	{
		/*std::vector<uint32_t> queryStates = {};
		queryStates.resize(2);
		auto res = drawCmd->GetQueryPoolResults(pool,queryStates,vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::eWithAvailability);
		switch(res)
		{
			case Vulkan::QueryResult::ErrorDeviceLost:
				Con::CERR<<"Device Lost"<<Con::endl;
				break;
			case Vulkan::QueryResult::ErrorOutOfDeviceMemory:
				Con::CERR<<"Out of device memory"<<Con::endl;
				break;
			case Vulkan::QueryResult::ErrorOutOfHostMemory:
				Con::CERR<<"Out of host memory"<<Con::endl;
				break;
			case Vulkan::QueryResult::NotReady:
				Con::CERR<<"Not ready"<<Con::endl;
				break;
			case Vulkan::QueryResult::Success:
				Con::CERR<<"Success"<<Con::endl;
				break;
		}
		Con::CWAR<<"Query result: "<<queryStates.front()<<" (Available: "<<queryStates.at(1)<<")"<<Con::endl;*/
	}
		
	//drawCmd->ResetQueryPool(pool,1);

	auto &shader = static_cast<Shader::Occlusion&>(*hShader.get());
	if(shader.BeginDraw(drawCmd) == true)
	{
		drawCmd->BindVertexBuffer(vertBuf);
		while(m_invisQueue.empty() == false)
		{
			auto hNode = m_invisQueue.front();
			m_invisQueue.pop();
			if(hNode.IsValid())
			{
				auto query = pragma::util::make_shared<CHCQuery>(*hNode.get());
				query->Run();
				m_queryQueue.push(query);
			}
		}
		shader.EndDraw();
	}
#endif
}

static auto cvCulling = pragma::console::get_client_con_var("cl_render_occlusion_culling");
bool CHC::InsideViewFrustum(CHCNode *cNode)
{
	if(cvCulling->GetInt() == 0)
		return true;
	return pragma::math::intersection::aabb_in_plane_mesh(cNode->GetMin(), cNode->GetMax(), m_frustumPlanes.begin(), m_frustumPlanes.end()) != pragma::math::intersection::Intersect::Outside;
}

bool CHC::WasVisible(CHCNode *cNode) { return (cNode->IsVisible() && (cNode->GetLastVisited() == (m_frameID - 1))) ? true : false; }

void CHC::HandleReturnedQuery(CHCQuery *query)
{
	long long r = query->GetResult();
	if(r > 0) {
		auto &hNode = query->m_hNode;
		if(hNode.IsValid()) {
			auto *node = hNode.get();
			PullUpVisibility(node);
			TraverseNode(node);
		}
	}
}

void CHC::IssueQuery(CHCNode *node)
{
	auto query = pragma::util::make_shared<CHCQuery>(*node);
	query->Run();
	m_queryQueue.push(query);
}

bool CHC::QueryReasonable(CHCNode *) { return true; }

CHCNode *CHC::GetNode(unsigned int idx)
{
	if(idx >= m_nodes.size())
		return nullptr;
	return m_nodes.at(idx).get();
}

std::vector<CHCMeshInfo> &CHC::PerformCulling()
{
	if(m_octree.expired() == true)
		return m_renderMeshes;
#ifdef CHC_DEBUGGING_ENABLED
	if(m_bDrawDebugTexture == true) {
		//auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
		/*glClearDepth(1.f);

		auto &scene = pragma::get_cgame()->GetScene();
		auto &camScene = scene->camera;
		Vector3 debugPos = camScene->GetPos();
		auto forward = camScene->GetForward();
		forward.y = 0.f;
		uvec::normalize(&forward);
		debugPos += forward *1024.f;
		debugPos.x += 4096.f;
		debugPos.z -= 4096.f;
		debugPos.y -= 4096.f;
		auto *camDebug = m_debugQueryDepthOverheadCamera.get();
		camDebug->SetPos(debugPos);
		camDebug->UpdateViewMatrix();*/ // prosper TODO

		/*static ShaderDebug *shader = static_cast<ShaderDebug*>(pragma::get_cgame()->GetShader("debug"));
		m_debugQueryDepthOverheadFrameBuffer->Bind();
		auto *cam = pragma::get_cgame()->GetRenderTarget();
		pragma::get_cgame()->SetRenderTarget(camDebug);
		pragma::get_cgame()->BindRenderTarget(camDebug);

		std::vector<Vector3> points;
		Vector3 viewDir = camDebug->GetForward();
		viewDir.y = 0.f;
		uvec::normalize(&viewDir);
		cam->GetFrustumPoints(&points,cam->GetZNear(),cam->GetZFar(),cam->GetFOVRad(),cam->GetAspectRatio(),cam->GetPos(),viewDir);

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
		//OpenGL::SetColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);
		shader->Render(m_debugFrustumBuffer,4,GL_QUADS,Vector4(1.f,0.f,0.f,1.f)); // Doesn't render view frustum; Why?
		//OpenGL::SetColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

		pragma::get_cgame()->SetRenderTarget(cam);
		pragma::get_cgame()->BindRenderTarget(cam);

		OpenGL::BindFrameBuffer(bufFramePrev);*/ // Vulkan TODO
	}
#endif
	//static ShaderOcclusion *shader = static_cast<ShaderOcclusion*>(pragma::get_cgame()->GetShader("occlusion"));
	//shader->PrepareBatchRendering(m_octree->GetVertexIndexBuffer()); // Vulkan TODO

	UpdateFrustum();
	m_renderMeshes.clear();
	auto &rRoot = m_octree.lock()->GetRootNode();
	auto idx = rRoot.GetIndex();
	auto *root = GetNode(idx);
	if(root == nullptr)
		return m_renderMeshes;

	m_distanceQueue.push(root->GetHandle());
#ifdef CHC_DEBUGGING_ENABLED
	if(GetDrawDebugTexture())
		chc_debug_draw = this;
#endif
	while(m_distanceQueue.empty() == false || m_queryQueue.empty() == false) {
		while(m_queryQueue.empty() == false) {
			auto q = m_queryQueue.front();
			if(q->IsValid() == false) {
				m_queryQueue.pop();
				continue;
			}
			if(q->IsAvailable() == true || m_distanceQueue.empty()) // || m_visQueue.empty())
			{
				m_queryQueue.pop();
				HandleReturnedQuery(q.get());
			}
			else if(m_visQueue.empty()) // TODO Is this correct?
				break;
			else {
				auto hNode = m_visQueue.front();
				m_visQueue.pop();
				if(hNode.IsValid())
					IssueQuery(hNode.get());
			}
		}
		if(!m_distanceQueue.empty()) {
			auto hNode = m_distanceQueue.front();
			m_distanceQueue.pop();
			if(hNode.IsValid()) {
				auto *n = hNode.get();
				if(InsideViewFrustum(n)) {
					auto bWasVisible = WasVisible(n);
					auto bLeafOrWasInvisible = (!bWasVisible || n->IsLeaf()) ? true : false;
					n->m_bVisible = false;
					n->m_lastVisited = m_frameID;
					if(bLeafOrWasInvisible == true)
						QueryPreviouslyInvisibleNode(n);
					else {
						if(n->IsLeaf() && QueryReasonable(n))
							m_visQueue.push(n->GetHandle());
						if(bWasVisible == true)
							TraverseNode(n);
					}
				}
			}
		}
		if(m_distanceQueue.empty())
			IssueMultiQueries();
	}
#ifdef CHC_DEBUGGING_ENABLED
	if(chc_debug_draw != nullptr)
		DebugRender(root, WasVisible(root));
#endif
	while(m_visQueue.empty() == false) {
		auto hNode = m_visQueue.front();
		m_visQueue.pop();
		if(hNode.IsValid())
			IssueQuery(hNode.get());
	}
#ifdef CHC_DEBUGGING_ENABLED
	chc_debug_draw = nullptr;
#endif
	m_frameID++;
	//shader->EndBatchRendering();
	return m_renderMeshes;
}

///////////////////////////////////////////

CHCNode::CHCNode(CHC *chc, const BaseOcclusionOctree::Node &node) : m_chc(chc), m_node(node.shared_from_this()), m_handle(pragma::util::to_shared_handle<CHCNode>(shared_from_this())) {}

CHCNode::~CHCNode() { m_handle.Invalidate(); }

uint32_t CHCNode::GetIndex() const
{
	if(m_node.expired() == true)
		return 0;
	return m_node.lock()->GetIndex();
}
bool CHCNode::IsVisible() const { return m_bVisible; }
bool CHCNode::IsLeaf() const
{
	if(m_node.expired() == true)
		return true;
	return m_node.lock()->IsLeaf();
}

uint8_t CHCNode::GetChildCount() const
{
	if(m_node.expired() == true)
		return 0;
	return m_node.lock()->GetChildCount();
}
uint64_t CHCNode::GetLastVisited() const { return m_lastVisited; }

CHCNodeHandle CHCNode::GetHandle() const { return m_handle; }
CHCNode *CHCNode::GetParent()
{
	if(m_node.expired() == true)
		return nullptr;
	auto parent = m_node.lock()->GetParent();
	if(parent == nullptr)
		return nullptr;
	return m_chc->GetNode(parent->GetIndex());
}
const CHCNode *CHCNode::GetChild(uint32_t idx) const
{
	if(m_node.expired() == true)
		return nullptr;
	auto *children = m_node.lock()->GetChildren();
	if(children == nullptr)
		return nullptr;
	auto &rNode = children->at(idx);
	if(rNode == nullptr)
		return nullptr;
	return m_chc->GetNode(rNode->GetIndex());
}
void CHCNode::GetBounds(Vector3 &min, Vector3 &max) const
{
	if(m_node.expired() == true)
		return;
	auto &worldBounds = m_node.lock()->GetWorldBounds();
	min = worldBounds.first;
	max = worldBounds.second;
}
const Vector3 &CHCNode::GetMin() const
{
	if(m_node.expired() == true)
		return uvec::PRM_ORIGIN;
	return m_node.lock()->GetWorldBounds().first;
}
const Vector3 &CHCNode::GetMax() const
{
	if(m_node.expired() == true)
		return uvec::PRM_ORIGIN;
	return m_node.lock()->GetWorldBounds().second;
}
void CHCNode::Render()
{
	//auto &context = pragma::get_cengine()->GetRenderContext();
	//auto &drawCmd = context.GetDrawCmd();
	//drawCmd->Draw(vertCount); // prosper TODO
}
const BaseOcclusionOctree::Node *CHCNode::GetRenderNode() const
{
	if(m_node.expired() == true)
		return nullptr;
	return m_node.lock().get();
}

///////////////////////////////////////////

CHCQuery::CHCQuery(const CHCNode &node) : m_hNode(node.GetHandle()), m_queryId(std::numeric_limits<decltype(m_queryId)>::max())
{
	/*pool->RequestFreeQuery(m_queryId);
	auto &context = pragma::get_cengine()->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	drawCmd->ResetQueryPool(pool,1,m_queryId);*/ // prosper TODO
}

CHCQuery::~CHCQuery()
{
	//pool->FreeQuery(m_queryId); // prosper TODO
}

uint32_t CHCQuery::GetResult() const
{
	uint32_t r = 0;
	GetResult(r);
	return r;
}

bool CHCQuery::GetResult(uint32_t &r) const
{
	/*auto &context = pragma::get_cengine()->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	std::vector<uint32_t> queryResult(2);
	auto err = drawCmd->GetQueryPoolResults(pool,queryResult,vk::QueryResultFlagBits::eWithAvailability,m_queryId); // TODO: This might refer to whatever query had been executed previously, not the new query. This can be avoided with a fence or event.
	//if(err != Vulkan::QueryResult::Success)
		return false;
	r = queryResult.at(0);
	return (queryResult.at(1) == 1) ? true : false;*/ // prosper TODO
	return false;
}

bool CHCQuery::IsAvailable() const
{
	uint32_t r;
	return GetResult(r);
}

bool CHCQuery::IsValid() const { return m_hNode.IsValid(); }

void CHCQuery::Run()
{
	// prosper TODO
#if 0
	if(m_hNode.IsValid() == false)
		return;
	auto &context = pragma::get_cengine()->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	auto *node = m_hNode.get();
	drawCmd->BeginQuery(pool,m_queryId,vk::QueryControlFlagBits::ePrecise);
		node->Render();
	drawCmd->EndQuery(pool,m_queryId);
#endif
}
