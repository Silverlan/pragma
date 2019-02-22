#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/debug/c_shader_debugdepthbuffer.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(DebugDepthBuffer,debugdepthbuffer);

DebugDepthBuffer::DebugDepthBuffer()
	: Textured("DebugDepthBuffer","screen/vs_screen_uv","debug/fs_debugdepthbuffer","u_depth_texture")
{}

void DebugDepthBuffer::InitializeShader()
{
	Textured::InitializeShader();
	/*m_locNearPlane = OpenGL::GetUniformLocation(m_shader,"u_nearPlane");
	m_locFarPlane = OpenGL::GetUniformLocation(m_shader,"u_farPlane");

	m_locTextureCSM = OpenGL::GetUniformLocation(m_shaderCSM,"u_depth_texture");
	OpenGL::SetUniform1i(m_locTextureCSM,0);
	m_locNearPlaneCSM = OpenGL::GetUniformLocation(m_shaderCSM,"u_nearPlane");
	m_locFarPlaneCSM = OpenGL::GetUniformLocation(m_shaderCSM,"u_farPlane");
	m_locLayerCSM = OpenGL::GetUniformLocation(m_shaderCSM,"u_layer");

	m_locTextureCube = OpenGL::GetUniformLocation(m_shaderCube,"u_depth_texture");
	OpenGL::SetUniform1i(m_locTextureCube,0);
	m_locNearPlaneCube = OpenGL::GetUniformLocation(m_shaderCube,"u_nearPlane");
	m_locFarPlaneCube = OpenGL::GetUniformLocation(m_shaderCube,"u_farPlane");
	m_locSideCube = OpenGL::GetUniformLocation(m_shaderCube,"u_cubeSide");*/ // Vulkan TODO
}

void DebugDepthBuffer::Initialize()
{
	Textured::Initialize();
}
void DebugDepthBuffer::Reload()
{
	//ShaderBase::InitializeShader("DebugDepthBufferCSM","screen/vs_screen.gls","debug/fs_debugdepthbuffer_csm.gls",&m_shaderCSM,true);
	//ShaderBase::InitializeShader("DebugDepthBufferCube","screen/vs_screen.gls","debug/fs_debugdepthbuffer_cube.gls",&m_shaderCube,true); // Vulkan TODO
	Textured::Reload();
}

void DebugDepthBuffer::Render(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,int,unsigned int)
{
	/*if(target == GL_TEXTURE_2D_ARRAY)
		glUseProgram(m_shaderCSM);
	else if(target == GL_TEXTURE_CUBE_MAP)
		glUseProgram(m_shaderCube);
	else
		Use();

	OpenGL::SetActiveTexture(GL_TEXTURE0);
	OpenGL::BindTexture(texture,target);
	//BindTexture(texture,target);
	if(target == GL_TEXTURE_2D_ARRAY)
	{
		OpenGL::SetUniform1i(m_locTextureCSM,0);
		OpenGL::SetUniform1f(m_locNearPlaneCSM,CFloat(zNear));
		OpenGL::SetUniform1f(m_locFarPlaneCSM,CFloat(zFar));
		OpenGL::SetUniform1i(m_locLayerCSM,layer);
	}
	else if(target == GL_TEXTURE_CUBE_MAP)
	{
		OpenGL::SetUniform1i(m_locTextureCube,0);
		OpenGL::SetUniform1f(m_locNearPlaneCube,CFloat(zNear));
		OpenGL::SetUniform1f(m_locFarPlaneCube,CFloat(zFar));
	}
	else
	{
		//OpenGL::SetUniform1i(m_locTexture,0);
		OpenGL::SetUniform1f(m_locNearPlane,CFloat(zNear));
		OpenGL::SetUniform1f(m_locFarPlane,CFloat(zFar));
	}
	OpenGL::EnableVertexAttribArray(0);
	OpenGL::BindBuffer(vertexBuffer);
	OpenGL::SetVertexAttribData(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		(void*)0
	);
	if(target == GL_TEXTURE_CUBE_MAP)
	{
		for(int i=0;i<6;i++)
		{
			OpenGL::SetUniform1i(m_locSideCube,i);
			OpenGL::AttachFrameBufferTexture2D(GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X +i,tgtTexture,0,GL_FRAMEBUFFER);
			OpenGL::DrawArrays(GL_TRIANGLES,0,6);
		}
	}
	else
		OpenGL::DrawArrays(GL_TRIANGLES,0,6);
	OpenGL::DisableVertexAttribArray(0);
	UnbindTexture(target);*/ // Vulkan TODO
}
#endif
