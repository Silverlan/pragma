#ifndef __C_SHADERDEBUGDEPTHBUFFER_H__
#define __C_SHADERDEBUGDEPTHBUFFER_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

namespace Shader
{
	class DLLCLIENT DebugDepthBuffer
		: public Textured
	{
	protected:
		unsigned int m_locNearPlane;
		unsigned int m_locFarPlane;

		unsigned int m_shaderCSM;
		unsigned int m_locTextureCSM;
		unsigned int m_locNearPlaneCSM;
		unsigned int m_locFarPlaneCSM;
		unsigned int m_locLayerCSM;

		unsigned int m_shaderCube;
		unsigned int m_locTextureCube;
		unsigned int m_locNearPlaneCube;
		unsigned int m_locFarPlaneCube;
		unsigned int m_locSideCube;
	public:
		DebugDepthBuffer();
		void InitializeShader();
		void Initialize();
		void Reload();
		void Render(unsigned int target,unsigned int texture,unsigned int vertexBuffer,unsigned int zNear,unsigned int zFar,int layer=-1,unsigned int tgtTexture=0);
	};
};
#endif
#endif