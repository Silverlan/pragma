#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/gui/wibaseblur.h"
#include <wgui/wgui.h>

extern CGame *c_game;
WIBaseBlur::WIBaseBlur()
	: /*m_blurRenderPass(nullptr),m_blurFramebuffer(nullptr),*/m_blurWidth(std::numeric_limits<uint32_t>::max()),m_blurHeight(std::numeric_limits<uint32_t>::max()) // prosper TODO
{}

void WIBaseBlur::DestroyBlur()
{
	//m_blurBuffer.Clear(); // prosper TODO
	//m_blurRenderPass = nullptr; // prosper TODO
	//m_blurFramebuffer = nullptr; // prosper TODO
}

void WIBaseBlur::InitializeBlur(uint32_t w,uint32_t h)
{
	DestroyBlur();
	m_blurWidth = w;
	m_blurHeight = h;
	//auto &context = *WGUI::GetContext();
	//m_blurRenderPass = Vulkan::RenderPass::Create(context,Anvil::Format::R8G8B8A8_UNORM);
	//m_blurFrameBuffer = Vulkan::Framebuffer::Create(context,m_blurRenderPass,w,h,{imgView});
	/*DestroyBlur();
	m_blurWidth = w;
	m_blurHeight = h;
	if(m_blurFBO != 0)
		OpenGL::DeleteFrameBuffer(m_blurFBO);
	m_blurFBO = OpenGL::GenerateFrameBuffer();
	OpenGL::BindFrameBuffer(m_blurFBO,GL_FRAMEBUFFER);
	if(m_blurTexture != 0)
		OpenGL::DeleteTexture(m_blurTexture);
	m_blurTexture = OpenGL::GenerateTexture();
	OpenGL::BindTexture(m_blurTexture,GL_TEXTURE_2D);
	OpenGL::Set2DTextureImage(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		w,h,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		0
	);
	OpenGL::SetTextureParameter(GL_TEXTURE_MAG_FILTER,GL_NEAREST,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_MIN_FILTER,GL_NEAREST,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE,GL_TEXTURE_2D);
	OpenGL::AttachFrameBufferTexture2D(GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_blurTexture,0);
	OpenGL::BindTexture(0,GL_TEXTURE_2D);
	OpenGL::BindFrameBuffer(0);*/ // Vulkan TODO
}

WIBaseBlur::~WIBaseBlur()
{
	DestroyBlur();
}

void WIBaseBlur::RenderBlur(UInt32,Float,Int32,Int32)
{
	//static ShaderGaussianBlur *blur = static_cast<ShaderGaussianBlur*>(c_game->GetShader("pp_gaussianblur")); // Vulkan TODO
	//OpenGL::BindFrameBuffer(m_blurFBO,GL_DRAW_FRAMEBUFFER);
	//OpenGL::BlitFrameBuffer(0,0,w,h,0,0,m_blurWidth,m_blurHeight);
	//const int kernelSize = 9;
	//blur->Render(CUInt32(w),CUInt32(h),ShaderGaussianBlur::TextureInfo(m_blurTexture,m_blurWidth,m_blurHeight),targetFBO,blurSize,kernelSize);
}