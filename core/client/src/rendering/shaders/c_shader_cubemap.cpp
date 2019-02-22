#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader_cubemap.h"

using namespace Shader;

LINK_SHADER_TO_CLASS(WGUICubemap,WGUICubemap);

WGUICubemap::WGUICubemap()
	: Textured("WGUICubemap","wgui/vs_wgui_cubemap","wgui/fs_wgui_cubemap","u_texture")
{}
DLLCLIENT void WGUICubemap::InitializeShader()
{
	//Textured::InitializeShader();
	//m_locMVP = OpenGL::GetUniformLocation(m_shader,"u_mvp"); // Vulkan TODO
}
DLLCLIENT void WGUICubemap::Render(unsigned int,const Mat4&,unsigned int,unsigned int,unsigned int,unsigned int)
{
	/*OpenGL::UseProgram(m_shader);
	OpenGL::SetActiveTexture(GL_TEXTURE0);
	OpenGL::BindTexture(texture,GL_TEXTURE_CUBE_MAP);
	//OpenGL::BindTexture(0,GL_TEXTURE_CUBE_MAP);
	//OpenGL::SetUniform1i(m_locTexture,0);
	OpenGL::EnableVertexAttribArray(0);
	OpenGL::BindBuffer(vertexBuffer);
	//OpenGL::AttachFrameBufferTexture2D(GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X,texture,0,GL_FRAMEBUFFER);
	//err = glGetError();
	OpenGL::SetVertexAttribData(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		(void*)0
	);
	OpenGL::EnableVertexAttribArray(1);
	OpenGL::BindBuffer(uvBuffer);
	OpenGL::SetVertexAttribData(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		(void*)0
	);
	OpenGL::SetUniformMatrix4(m_locMVP,1,&mat[0][0]);
	OpenGL::EnableVertexAttribArray(1);
	OpenGL::BindBuffer(uvBuffer);
	OpenGL::SetVertexAttribData(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		(void*)0
	);
		OpenGL::DrawArrays(renderMode,0,vertCount);
	OpenGL::DisableVertexAttribArray(1);
	OpenGL::DisableVertexAttribArray(0);

	OpenGL::BindTexture(0,GL_TEXTURE_CUBE_MAP);*/ // Vulkan TODO
}
#endif