#ifndef __C_SHADER_REFLECTIVE_H__
#define __C_SHADER_REFLECTIVE_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

struct Camera;
class Texture;
namespace Shader
{
	class DLLCLIENT Reflective
		: public TexturedBase3D
	{
	protected:
		static bool m_bSkip;
		Reflective(std::string identifier,std::string vsShader,std::string fsShader,std::string uniTexture);
		Texture *m_texture;
		unsigned int m_locMVPReflection;
		unsigned int m_bufFrame;
		unsigned int m_bufTexture;
		unsigned int m_bufDepth;
		Camera *m_cam;
		virtual void InitializeShader() override;
		bool RenderReflection(Camera *cam,Material *mat,CBaseEntity *ent,CBrushMesh *mesh,CSide *side);
		bool RenderWorld(unsigned int frameBuffer,Mat4 &matTrans,Vector4 &clipPlane,bool bCW=false,Camera *cam=NULL);
		void BindDiffuseMap(Material *mat);
	public:
		Reflective();
		virtual ~Reflective() override;
		virtual void Render(Camera *cam,Material *mat,CBaseEntity *ent,CBrushMesh *mesh,CSide *side) override;
		virtual void Initialize() override;
		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
	};
};
#endif
#endif