#ifndef __WIBASEBLUR_H__
#define __WIBASEBLUR_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/umath.h>

class DLLCLIENT WIBaseBlur
{
protected:
	//Shader::GaussianBlur::BlurBuffer m_blurBuffer; // prosper TODO
	//Vulkan::RenderPass m_blurRenderPass; // prosper TODO
	//Vulkan::Framebuffer m_blurFramebuffer; // prosper TODO

	uint32_t m_blurWidth;
	uint32_t m_blurHeight;
	void DestroyBlur();
	void RenderBlur(UInt32 targetFBO,Float blurSize,Int32 w,Int32 h);
	void InitializeBlur(uint32_t w,uint32_t h);
public:
	WIBaseBlur();
	virtual ~WIBaseBlur();
};

#endif