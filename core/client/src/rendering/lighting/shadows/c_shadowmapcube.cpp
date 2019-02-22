#include "stdafx_client.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcube.h"
#include <pragma/console/conout.h>
#include <pragma/console/convars.h>
#include "pragma/console/c_cvar.h"

ShadowMapCube::ShadowMapCube()
	: ShadowMap(6)
{}

/*void ShadowMapCube::ClearDepth()
{
	auto bufPrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	unsigned int frameBuffer = GetFrameBuffer();
	unsigned int depthTexture = GetDepthTexture();
	OpenGL::BindFrameBuffer(frameBuffer,GL_FRAMEBUFFER);
	for(int i=0;i<6;i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_CUBE_MAP_POSITIVE_X +i,depthTexture,0);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	OpenGL::BindFrameBuffer(bufPrev,GL_FRAMEBUFFER);
}*/ // Vulkan TODO

static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
/*void ShadowMapCube::ReloadDepthTextures()
{
	unsigned int size = cvShadowmapSize->GetInt();
	m_bValid = true;
	auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	OpenGL::BindFrameBuffer(m_frameBuffer,GL_FRAMEBUFFER);
	unsigned int bufTexPrev;
	GenerateTextures();
	bufTexPrev = OpenGL::GetInt(GL_TEXTURE_BINDING_CUBE_MAP);
	OpenGL::BindTexture(m_depthTexture,GL_TEXTURE_CUBE_MAP);
	OpenGL::SetTextureParameter(GL_DEPTH_TEXTURE_MODE,GL_LUMINANCE,GL_TEXTURE_CUBE_MAP);
	OpenGL::SetTextureParameter(GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL,GL_TEXTURE_CUBE_MAP);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAX_LEVEL,0,GL_TEXTURE_CUBE_MAP);

	//OpenGL::SetTextureParameter(GL_TEXTURE_COMPARE_MODE,GL_COMPARE_R_TO_TEXTURE,GL_TEXTURE_CUBE_MAP);
	//OpenGL::SetTextureParameter(GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL,GL_TEXTURE_CUBE_MAP);
	//OpenGL::SetTextureParameter(GL_DEPTH_TEXTURE_MODE,GL_INTENSITY,GL_TEXTURE_CUBE_MAP);

	OpenGL::SetTextureParameter(GL_TEXTURE_COMPARE_MODE,GL_COMPARE_R_TO_TEXTURE,GL_TEXTURE_CUBE_MAP);
	for(int i=0;i<6;i++)
	{
		OpenGL::Set2DTextureImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i,0,GL_DEPTH_COMPONENT,size,size,0,GL_DEPTH_COMPONENT,GL_FLOAT,0);
		if(glGetError() == GL_OUT_OF_MEMORY)
		{
			Con::cwar<<"WARNING: Not enough memory to create shadowmap! Try reducing 'cl_render_shadow_resolution'."<<Con::endl;
			m_bValid = false;
			OpenGL::BindFrameBuffer(bufFramePrev);
			return;
		}
		OpenGL::AttachFrameBufferTexture2D(GL_DEPTH_ATTACHMENT,GL_TEXTURE_CUBE_MAP_POSITIVE_X +i,m_depthTexture,0,GL_FRAMEBUFFER);
		//OpenGL::AttachFrameBufferTexture(GL_DEPTH_ATTACHMENT,m_depthTexture,0,GL_FRAMEBUFFER);
	}

	OpenGL::SetDrawColorBuffer(GL_NONE);
	//unsigned int modePrev = OpenGL::GetInt(GL_READ_BUFFER);
	OpenGL::SetColorBufferSource(GL_NONE);

	if(!m_bValid || OpenGL::GetFrameBufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		m_bValid = false;
	else m_bValid = true;
	OpenGL::BindFrameBuffer(bufFramePrev);
	//OpenGL::SetColorBufferSource(modePrev);
	OpenGL::BindTexture(bufTexPrev,GL_TEXTURE_CUBE_MAP);
}*/ // Vulkan TODO

ShadowMap::Type ShadowMapCube::GetType() {return ShadowMap::Type::Cube;}