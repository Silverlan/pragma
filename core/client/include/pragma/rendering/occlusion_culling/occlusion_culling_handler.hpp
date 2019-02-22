#ifndef __OCCLUSION_CULLING_HANDLER_HPP__
#define __OCCLUSION_CULLING_HANDLER_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/baseentity_handle.h>
#include <pragma/util/util_bsp_tree.hpp>

class Scene;
class CBaseEntity;
class CModelMesh;
namespace pragma
{
	class CParticleSystemComponent;
	class CLightComponent;
	struct DLLCLIENT OcclusionMeshInfo
	{
		OcclusionMeshInfo(CBaseEntity &ent,CModelMesh &mesh);
		CModelMesh *mesh;
		EntityHandle hEntity;
	};
	class DLLCLIENT OcclusionCullingHandler
	{
	public:
		virtual ~OcclusionCullingHandler()=default;
		virtual void Update(const Vector3 &camPos) {}
		virtual void Initialize() {}
		// This function will not cull anything, culling behavior has to be implemented by derived classes
		virtual void PerformCulling(const Scene &scene,std::vector<OcclusionMeshInfo> &culledMeshesOut)=0;

		virtual void PerformCulling(const Scene &scene,std::vector<pragma::CParticleSystemComponent*> &particlesOut);
		virtual void PerformCulling(const Scene &scene,const std::vector<pragma::CLightComponent*> &lightsIn,std::vector<pragma::CLightComponent*> &lightsOut);
		virtual void PerformCulling(const Scene &scene,const Vector3 &origin,float radius,std::vector<OcclusionMeshInfo> &culledMeshesOut);
	protected:
		OcclusionCullingHandler()=default;
		virtual bool ShouldExamine(CModelMesh &mesh,const Vector3 &pos,bool bViewModel,std::size_t numMeshes,const std::vector<Plane> &planes) const;
		virtual bool ShouldExamine(const Scene &scene,CBaseEntity &cent,bool &outViewModel,std::vector<Plane> **outPlanes) const;
	};
};

#endif
