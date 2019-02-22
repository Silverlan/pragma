#ifndef __C_SHADER_CUBEMAP_H__
#define __C_SHADER_CUBEMAP_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

namespace Shader
{
	class DLLCLIENT WGUICubemap
		: public Textured
	{
	protected:
		virtual void InitializeShader() override;
		unsigned int m_locMVP;
	public:
		WGUICubemap();
		void Render(unsigned int renderMode,const Mat4 &mat,unsigned int texture,unsigned int vertexBuffer,unsigned int uvBuffer,unsigned int vertCount);
	};
};
#endif
#endif