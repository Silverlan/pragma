#ifndef __C_SHADER_H__
#define __C_SHADER_H__

 // prosper TODO
/*#include "pragma/clientdefinitions.h"
#include "pragma/networkdefinitions.h"
#include "material.h"
#include "pragma/rendering/scene/camera.h"
#include <unordered_map>

class CModelMesh;
class CModelSubMesh;
class CBrushMesh;
class CSide;
class CParticleSystem;
class CBaseEntity;
class Model;
class NormalMesh;
class GLMesh;
namespace Shader
{
	class DLLCLIENT Base3D
		: public Base
	{
	protected:
		Base3D(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		Base3D();
		bool m_bDynamicLineWidth;
		bool m_bAnimated = false;
		void Draw(CModelSubMesh *mesh,const std::function<void(const Vulkan::CommandBufferObject*,std::size_t)> &fDraw);
		virtual void InitializeDynamicStates(std::vector<vk::DynamicState> &states) override;
		bool IsWeighted(CBaseEntity *ent) const;
	public:
		virtual ~Base3D() override;
		// Old // Vulkan TODO
		virtual void Render(Camera *cam,Material *mat);
		virtual void Render(Camera *cam,Material *mat,CParticleSystem *particle);
		virtual void Render(Camera *cam,Material *mat,CBaseEntity *ent);
		virtual void Render(Camera *cam,Material *mat,CBaseEntity *ent,::Model *mdl,CModelMesh *mesh,CModelSubMesh *subMesh);
		virtual void Render(Camera *cam,Material *mat,CBaseEntity *ent,CBrushMesh *mesh,CSide *side);
		virtual void InitializeMaterial(Material *mat);
		// New
		virtual void Bind();
		virtual void Bind(Material *mat);
		virtual void Bind(Camera *cam,CBaseEntity *ent);
		virtual void Render(Camera *cam,GLMesh *mesh);
		virtual void Unbind();
		
		virtual bool BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,uint32_t descSetId,bool &bWeighted);
		virtual void Draw(CModelSubMesh *mesh);
	};
};*/

#endif