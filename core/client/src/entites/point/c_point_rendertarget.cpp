#include "stdafx_client.h"
#include "pragma/entities/point/c_point_rendertarget.h"
#include "pragma/entities/c_entityfactories.h"
#include <cmaterialmanager.h>
#include "pragma/rendering/scene/camera.h"
#include <mathutil/umat.h>
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/c_engine.h"
#include <texturemanager/texturemanager.h>
#include "pragma/rendering/scene/scene.h"
#include "textureinfo.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

#pragma message ("FIXME: If point_rendertarget is out of view of the local player, but one of the texture targets isn't, they won't get updated! Find a solution!")

LINK_ENTITY_TO_CLASS(point_rendertarget,CPointRenderTarget);

void CRenderTargetComponent::ReceiveData(NetPacket &packet)
{
	m_kvMaterial = packet->ReadString();
	m_kvFOV = packet->Read<float>();
	m_kvRefreshRate = packet->Read<float>();
	m_kvRenderWidth = packet->Read<float>();
	m_kvRenderHeight = packet->Read<float>();
	m_kvNearZ = packet->Read<float>();
	m_kvFarZ = packet->Read<float>();
	m_kvRenderDepth = packet->Read<int>();
}

void CRenderTargetComponent::SetRenderSize(Vector2 &size) {SetRenderSize(size.x,size.y);}
void CRenderTargetComponent::SetRenderSize(float w,float h) {m_kvRenderWidth = w;m_kvRenderHeight = h;}
Vector2 CRenderTargetComponent::GetRenderSize() {return Vector2(m_kvRenderWidth,m_kvRenderHeight);}
void CRenderTargetComponent::GetRenderSize(float *w,float *h) {*w = m_kvRenderWidth;*h = m_kvRenderHeight;}

void CRenderTargetComponent::SetRenderMaterial(Material *mat) {m_matRender = mat;}
Material *CRenderTargetComponent::GetRenderMaterial() {return m_matRender;}
void CRenderTargetComponent::SetRenderMaterial(std::string mat) {SetRenderMaterial(client->LoadMaterial(mat.c_str()));}

void CRenderTargetComponent::SetRefreshRate(float f) {m_kvRefreshRate = f;}
float CRenderTargetComponent::GetRefreshRate() {return m_kvRefreshRate;}

void CRenderTargetComponent::SetRenderFOV(float fov) {m_kvFOV = fov;}
float CRenderTargetComponent::GetRenderFOV() {return m_kvFOV;}

void CRenderTargetComponent::SetRenderDepth(unsigned int depth) {m_kvRenderDepth = depth;}
unsigned int CRenderTargetComponent::GetRenderDepth() {return m_kvRenderDepth;}

void CRenderTargetComponent::Spawn()
{
#if 0
	CBaseEntity::Spawn();
	SetRenderMaterial(m_kvMaterial);

	if(m_kvRenderWidth == 0.f || m_kvRenderHeight == 0.f)
	{
		int w,h;
		OpenGL::GetViewportSize(&w,&h);
		if(m_kvRenderWidth == 0.f)
			m_kvRenderWidth = CFloat(w);
		if(m_kvRenderHeight == 0.f)
			m_kvRenderHeight = CFloat(h);
	}
	if(m_kvNearZ == 0.f || m_kvFarZ == 0.f)
	{
		auto &scene = c_game->GetScene();
		auto &cam = scene->camera;
		if(m_kvNearZ == 0.f)
			m_kvNearZ = cam->GetZNear();
		if(m_kvFarZ == 0.f)
			m_kvFarZ = cam->GetZFar();
	}
	float aspectRatio = (m_kvRenderHeight > 0.f) ? (m_kvRenderWidth /m_kvRenderHeight) : 0.f;

	/*m_cam = new Camera(m_kvFOV,m_kvFOV,aspectRatio,m_kvNearZ,m_kvFarZ);
	m_bufFrame = OpenGL::GenerateFrameBuffer();
	m_bufRender = OpenGL::GenerateRenderBuffer();
	m_bufTexture = OpenGL::GenerateTexture();
	m_texture = new Texture(m_bufTexture,CUInt32(m_kvRenderWidth),CUInt32(m_kvRenderHeight));
	auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	unsigned int bufTexPrev = OpenGL::GetInt(GL_TEXTURE_BINDING_2D);
	OpenGL::BindFrameBuffer(m_bufFrame);
	OpenGL::BindTexture(m_texture->GetTextureID());

	OpenGL::Set2DTextureImage(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		CInt32(m_kvRenderWidth),
		CInt32(m_kvRenderHeight),
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		0
	);
	OpenGL::SetTextureParameter(GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAX_LEVEL,0);
	OpenGL::AttachFrameBufferTexture2D(GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_bufTexture,0);

	OpenGL::BindRenderBuffer(m_bufRender);
	OpenGL::SetRenderBufferStorage(GL_DEPTH24_STENCIL8,CInt32(m_kvRenderWidth),CInt32(m_kvRenderHeight));
	OpenGL::AttachFrameBufferRenderBuffer(GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,m_bufRender);

	if(m_matRender != NULL)
		CMaterialSystem::SetTexture(m_matRender,"diffusemap",m_texture);

	OpenGL::BindTexture(bufTexPrev);
	OpenGL::BindFrameBuffer(bufFramePrev);*/ // Vulkan TODO
#endif
}

unsigned int CRenderTargetComponent::GetTextureBuffer() {return 0;} // Obsolete

void CRenderTargetComponent::Render(RenderMode renderMode)
{
	 // prosper TODO
#if 0
	if(!IsTurnedOn())
		return;
	auto &scene = c_game->GetScene();
	auto &cam = scene->camera;

	m_cam->SetViewMatrix(cam->GetViewMatrix());
	m_cam->SetProjectionMatrix(cam->GetProjectionMatrix());
	m_cam->SetViewProjectionMatrix(cam->GetViewProjectionMatrix());

	Plane plane(
		Vector3(1546.f,-241.f,32.f),
		Vector3(1546.f,-21.f,32.f),
		Vector3(1546.f,-21.f,-192.f)
	);
	Vector3 n = Vector3(0,1,0);//plane.GetNormal();
	double d = 174.f;//plane.GetDistance();
	Mat4 matReflection(
		1.f -2.f *n.x *n.x,-2.f *n.x *n.y,-2.f *n.x *n.z,-2.f *n.x *d,
		-2.f *n.x *n.y,1.f -2.f *n.y *n.y,-2.f *n.y *n.z,-2.f *n.y *d,
		-2.f *n.x *n.z,-2.f *n.y *n.z,1.f -2.f *n.z *n.z,-2.f *n.z *d,
		0.f,0.f,0.f,1.f
	);
	/*
	Mat4 mReflection(
		1.f -2.f *n.x *n.x,-2.f *n.x *n.y,-2.f *n.x *n.z,-2.f *n.x *d,
		-2.f *n.x *n.y,1.f -2.f *n.y *n.y,-2.f *n.y *n.z,-2.f *n.y *d,
		-2.f *n.x *n.z,-2.f *n.y *n.z,1.f -2.f *n.z *n.z,-2.f *n.z *d,
		0.f,0.f,0.f,1.f
	);*/
	//matView = glm::inverse(matView *mReflection);
	//matView = glm::inverse(matView);
	/*Vector3 forward,right,up;
	GetOrientation(&forward,&right,&up);
	Vector3 pos = *GetPosition();
	pos.x *= -1.f;
	m_cam->SetPos(pos *2.f);
	m_cam->SetUp(up);
	//forward = glm::rotateX(forward,90.f);
	//m_cam->Get
	//forward.x *= -1.f;
	m_cam->SetForward(forward);
	m_cam->UpdateViewMatrix();
	Mat4 &mat = m_cam->GetViewMatrix();
	mat = cam.GetViewMatrix();
	mat = glm::scale(mat,-1.f,1.f,1.f);
	//mat = cam.GetViewMatrix();
	//mat = glm::scale(mat,-1.f,1.f,1.f);
	Mat4 &matProj = m_cam->GetProjectionMatrix();
	matProj = cam.GetProjectionMatrix();
	Mat4 &matViewProj = m_cam->GetViewProjectionMatrix();
	matProj = cam.GetViewProjectionMatrix();*/
	//m_cam->SetForward(forward);

	//Scene *scene = c_engine->GetScene(0);
	//Camera &cam = scene->camera;
	//m_cam->SetForward(glm::inverse(forward));

	/*CBaseEntity::Render();
	double t = c_game->CurTime();
	double tDelta = t -m_tLastRefresh;
	float refresh = GetRefreshRate();
	if((refresh >= 0.f && (tDelta >= refresh || m_tLastRefresh == 0)) || m_tLastRefresh == 0)
	{
		m_curDepth++;
		if(m_curDepth <= CUInt32(m_kvRenderDepth))
		{
			int w,h;
			OpenGL::GetViewportSize(&w,&h);
			auto bufPrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
			OpenGL::BindFrameBuffer(m_bufFrame);
				OpenGL::SetViewPort(0,0,CInt32(m_kvRenderWidth),CInt32(m_kvRenderHeight));
				OpenGL::Clear(GL_COLOR_BUFFER_BIT);
				OpenGL::Clear(GL_DEPTH_BUFFER_BIT);
				//c_game->SetShaderOverride(c_game->GetShader("reflective"));
				c_game->SetRenderTarget(m_cam);//m_cam);
					//c_game->Render();
				c_game->SetRenderTarget();
				c_game->UpdateCameraBuffer();
				//c_game->SetShaderOverride(NULL);
			OpenGL::BindFrameBuffer(bufPrev);
			OpenGL::SetViewPort(0,0,w,h);
			m_tLastRefresh = t;
			m_curDepth = 0;
		}
	}*/
#endif
}
luabind::object CRenderTargetComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CRenderTargetComponentHandleWrapper>(l);}

void CPointRenderTarget::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRenderTargetComponent>();
}
