#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/world/c_shader_reflective.h"
#include "pragma/model/c_side.h"
#include "cmaterialmanager.h"
 // prosper TODO
#if 0
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/uniformbinding.h"
#include <texturemanager/texturemanager.h>
#include "textureinfo.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(Reflective,reflective);

bool Reflective::m_bSkip = false;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

Reflective::Reflective(std::string identifier,std::string vsShader,std::string fsShader,std::string uniTexture) // Vulkan TODO
	: TexturedBase3D(identifier,vsShader,fsShader),m_cam(NULL),m_texture(NULL)
{}

Reflective::Reflective()
	: Reflective("Reflective","world/vs_reflective","world/fs_reflective","") // Vulkan TODO
{}

Reflective::~Reflective()
{
	if(m_cam != NULL)
		delete m_cam;
	if(m_texture != NULL)
		delete m_texture;
}

void Reflective::Render(Camera*,Material*,CBaseEntity*,CBrushMesh*,CSide*)
{
	/*Use();
	if(!RenderReflection(cam,mat,ent,mesh,side))
		return;
	Shader3DTexturedBase::Render(cam,mat,ent,mesh,side);*/ // Vulkan TODO
}

bool Reflective::RenderReflection(Camera *cam,Material*,CBaseEntity *ent,CBrushMesh*,CSide *side)
{
	if(m_bSkip == true)
		return false;
	Vector3 n = *side->GetNormal();
	float d = side->GetDistance();
	Vector3 posPlane = n *-d;
	Vector3 dir = cam->GetPos() -posPlane;
	float dot = glm::dot(dir,n);
	if(dot < 0.f)
		return false;
	Vector3 offset = cam->GetPos() -posPlane;
	float dCamera = uvec::dot(n,offset);
	m_cam->SetPos(cam->GetPos() -n *(dCamera *2.f));
	/*
	Mat4 matReflection(
		1.f -2.f *n.x *n.x,-2.f *n.x *n.y,-2.f *n.x *n.z,-2.f *n.x *d,
		-2.f *n.x *n.y,1.f -2.f *n.y *n.y,-2.f *n.y *n.z,-2.f *n.y *d,
		-2.f *n.x *n.z,-2.f *n.y *n.z,1.f -2.f *n.z *n.z,-2.f *n.z *d,
		0.f,0.f,0.f,1.f
	);
	*/
	Mat4 matReflection(
		1.f -2.f *n.x *n.x,-2.f *n.x *n.y,-2.f *n.x *n.z,0.f,
		-2.f *n.x *n.y,1.f -2.f *n.y *n.y,-2.f *n.y *n.z,0.f,
		-2.f *n.x *n.z,-2.f *n.y *n.z,1.f -2.f *n.z *n.z,0.f,
		0.f,0.f,0.f,1.f
	);
	matReflection = glm::translate(matReflection,n *d *2.f);
	Mat4 matView = cam->GetViewMatrix() *matReflection;
	m_cam->SetViewMatrix(matView);
	Mat4 *matTrans = ent->GetTransformationMatrix();
	Mat4 mvp = m_cam->GetProjectionMatrix() *matView *(*ent->GetTransformationMatrix());
	//OpenGL::SetUniformMatrix4(m_locMVPReflection,1,&mvp[0][0]);

	Vector4 clipPlane(n.x,n.y,n.z,d);
	RenderWorld(m_bufFrame,*matTrans,clipPlane,true,m_cam);
	return true;
}

bool Reflective::RenderWorld(unsigned int,Mat4&,Vector4&,bool,Camera*)
{
	/*if(m_bSkip == true)
		return false;
	unsigned int clipPlaneBuffer = GLUniformBlockManager::GetBuffer(UNIFORM_BINDING_CLIPPLANE);
	OpenGL::BindBuffer(clipPlaneBuffer,GL_UNIFORM_BUFFER);
	OpenGL::BindBufferData(sizeof(Vector4),&clipPlane[0],GL_STREAM_DRAW,GL_UNIFORM_BUFFER);

	unsigned int renderFlags = FRENDER_ALL;
	renderFlags &= ~FRENDER_VIEW;

	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	auto bufPrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	OpenGL::BindFrameBuffer(frameBuffer);
		//OpenGL::SetViewPort(0,0,m_kvRenderWidth,m_kvRenderHeight);
		OpenGL::Clear(GL_COLOR_BUFFER_BIT);
		OpenGL::Clear(GL_DEPTH_BUFFER_BIT);
		//c_game->SetShaderOverride(c_game->GetShader("reflective"));
		if(cam != NULL)
			c_game->SetRenderTarget(cam);

			m_bSkip = true;
			if(bCW == true)
				glFrontFace(GL_CW);

				c_game->Render(renderFlags);
			if(bCW == true)
				glFrontFace(GL_CCW);
			m_bSkip = false;
		if(cam != NULL)
		{
			c_game->SetRenderTarget();
			c_game->BindRenderTarget();
		}
		//c_game->SetShaderOverride(NULL);
	OpenGL::BindFrameBuffer(bufPrev);
	//OpenGL::SetViewPort(0,0,w,h);
	Use();
	OpenGL::BindBuffer(clipPlaneBuffer,GL_UNIFORM_BUFFER);
	clipPlane = Vector4(0.f,0.f,0.f,0.f);
	OpenGL::BindBufferData(sizeof(Vector4),&clipPlane[0],GL_STREAM_DRAW,GL_UNIFORM_BUFFER);

#pragma message ("TODO: Check CBaseEntity::Render -> We have to rebind the buffer here, but we should put it into its own method.")
	cam = c_game->GetRenderTarget();
	unsigned int bufferViewProj = GLUniformBlockManager::GetBuffer(UNIFORM_BINDING_VIEWPROJECTION);
	OpenGL::BindBuffer(bufferViewProj,GL_UNIFORM_BUFFER);
	OpenGL::BindBufferSubData(0,sizeof(Mat4),&matTrans[0][0],GL_UNIFORM_BUFFER);
	Mat4 mvp = cam->GetProjectionMatrix() *cam->GetViewMatrix() *matTrans;
	OpenGL::BindBufferSubData(sizeof(Mat4) *3,sizeof(Mat4),&mvp[0][0],GL_UNIFORM_BUFFER);
	OpenGL::BindBuffer(0,GL_UNIFORM_BUFFER);!*/ // Vulkan TODO
	return true;
}

void Reflective::Initialize()
{
	/*ShaderWorldBase::Initialize();
	m_bufFrame = OpenGL::GenerateFrameBuffer();

	m_bufTexture = OpenGL::GenerateTexture();
	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	m_texture = new Texture(m_bufTexture,w,h);
	auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	unsigned int bufTexPrev = OpenGL::GetInt(GL_TEXTURE_BINDING_2D);
	OpenGL::BindFrameBuffer(m_bufFrame);
	OpenGL::BindTexture(m_bufTexture);

	OpenGL::Set2DTextureImage(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		w,
		h,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		0
	);
	OpenGL::SetTextureParameter(GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAX_LEVEL,0);
	OpenGL::AttachFrameBufferTexture2D(GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_bufTexture,0);

	m_bufDepth = OpenGL::GenerateTexture();
	OpenGL::BindTexture(m_bufDepth);
	OpenGL::Set2DTextureImage(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT24,
		w,h,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		0
	);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	OpenGL::SetTextureParameter(GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	OpenGL::SetTextureParameter(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	OpenGL::SetTextureParameter(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	OpenGL::AttachFrameBufferTexture2D(GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,m_bufDepth,0);

	OpenGL::BindTexture(bufTexPrev);
	OpenGL::BindFrameBuffer(bufFramePrev);

	Scene *scene = c_engine->GetScene(0);
	Camera *cam = &scene->camera;

	m_cam = new Camera(cam->GetFOV(),cam->GetViewFOV(),cam->GetAspectRatio(),cam->GetZNear(),cam->GetZFar());*/ // Vulkan TODO
}

void Reflective::InitializeShader()
{
	//ShaderWorldBase::InitializeShader();
	//m_locMVPReflection = OpenGL::GetUniformLocation(m_shader,"MVPReflection"); // Vulkan TODO
}

void Reflective::InitializeMaterial(Material*,bool bReload)
{
	//CMaterialSystem::SetTexture(mat,"reflectionmap",m_texture);
}

void Reflective::BindDiffuseMap(Material*)
{
	//TextureInfo *diffuse = mat->GetTextureInfo("reflectionmap");
	//if(diffuse != NULL)
	//	BindTexture(diffuse); // Vulkan TODO
}
#endif
