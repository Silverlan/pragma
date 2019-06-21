#include "stdafx_client.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

OcclusionMeshInfo::OcclusionMeshInfo(CBaseEntity &ent,CModelMesh &mesh)
	: mesh{&mesh},hEntity{ent.GetHandle()}
{}

bool OcclusionCullingHandler::ShouldExamine(CModelMesh &mesh,const Vector3 &pos,bool bViewModel,std::size_t numMeshes,const std::vector<Plane> &planes) const
{
	if(bViewModel == true || numMeshes == 1)
		return true;
	Vector3 min,max;
	mesh.GetBounds(min,max);
	min += pos;
	max += pos;
	return (Intersection::AABBInPlaneMesh(min,max,planes) != INTERSECT_OUTSIDE) ? true : false;
}
bool OcclusionCullingHandler::ShouldExamine(const rendering::RasterizationRenderer &renderer,CBaseEntity &ent,bool &outViewModel,std::vector<Plane> **outPlanes) const
{
	auto &cam = *c_game->GetRenderCamera();
	auto pRenderComponent = ent.GetRenderComponent();
	if(ent.IsSpawned() == false || pRenderComponent.expired() || pRenderComponent->ShouldDraw(cam.GetPos()) == false)
		return false;
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return false;
	auto pTrComponent = ent.GetTransformComponent();
	auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	Vector3 min;
	Vector3 max;
	pRenderComponent->GetRenderBounds(&min,&max);
	min += pos;
	max += pos;
	auto sphere = pRenderComponent->GetRenderSphereBounds();
	*outPlanes = (pRenderComponent->GetRenderMode() == RenderMode::Skybox) ? const_cast<std::vector<Plane>*>(&renderer.GetFrustumPlanes()) : const_cast<std::vector<Plane>*>(&renderer.GetClippedFrustumPlanes());
	outViewModel = (pRenderComponent->GetRenderMode() == RenderMode::View) ? true : false; // TODO: Remove me once the render bounds accurately encompass animation bounds
	return (outViewModel == true || (Intersection::SphereInPlaneMesh(pos +sphere.pos,sphere.radius,*(*outPlanes),true) != INTERSECT_OUTSIDE && Intersection::AABBInPlaneMesh(min,max,*(*outPlanes)) != INTERSECT_OUTSIDE)) ? true : false;
}
void OcclusionCullingHandler::PerformCulling(const rendering::RasterizationRenderer &renderer,std::vector<pragma::CParticleSystemComponent*> &particlesOut)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	particlesOut.reserve(entIt.GetCount());
	particlesOut.clear();
	auto &scene = renderer.GetScene();
	auto &frustumPlanes = renderer.GetFrustumPlanes();
	for(auto *ent : entIt)
	{
		auto hPt = ent->GetComponent<pragma::CParticleSystemComponent>();
		auto &bounds = hPt->GetRenderBounds();
		static auto bAlwaysPass = false;
		if(bAlwaysPass || Intersection::AABBInPlaneMesh(bounds.first,bounds.second,frustumPlanes) != INTERSECT_OUTSIDE)
			particlesOut.push_back(hPt.get());
	}
}
void OcclusionCullingHandler::PerformCulling(const rendering::RasterizationRenderer &renderer,const std::vector<pragma::CLightComponent*> &lightsIn,std::vector<pragma::CLightComponent*> &lightsOut)
{
	auto &scene = renderer.GetScene();
	auto &cam = scene.camera;
	auto &pos = cam->GetPos();
	std::vector<float> distances;
	auto sz = lightsIn.size();
	lightsOut.clear();
	lightsOut.reserve(sz);
	distances.reserve(sz);
	auto &frustumPlanes = renderer.GetFrustumPlanes();
	for(auto it=lightsIn.begin();it!=lightsIn.end();++it)
	{
		auto *light = *it;
		if(light == nullptr)
			continue;
		auto &ent = light->GetEntity();
		auto pToggleComponent = ent.GetComponent<pragma::CToggleComponent>();
		if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn())
		{
			auto type = LightType::Invalid;
			auto *pLight = light->GetLight(type);
			if(type == LightType::Directional)
			{
				lightsOut.insert(lightsOut.begin(),light);
				distances.push_back(0.f);
			}
			else
			{
				auto pTrComponent = ent.GetTransformComponent();
				auto pRadiusComponent = ent.GetComponent<pragma::CRadiusComponent>();
				if(pTrComponent.valid() && pRadiusComponent.valid())
				{
					auto &posLight = pTrComponent->GetPosition();
					if(Intersection::SphereInPlaneMesh(posLight,pRadiusComponent->GetRadius(),frustumPlanes) != INTERSECT_OUTSIDE)
					{
						auto dist = uvec::length_sqr(posLight -pos);
						auto bInserted = false;
						for(UInt i=0;i<lightsOut.size();i++)
						{
							//auto *lightOther = lightsOut[i];
							auto distOther = distances[i];
							if(dist < distOther)
							{
								lightsOut.insert(lightsOut.begin() +i,light);
								distances.insert(distances.begin() +i,dist);
								bInserted = true;
								break;
							}
						}
						if(bInserted == false)
						{
							lightsOut.push_back(light);
							distances.push_back(dist);
						}
					}
				}
			}
		}
	}

	uint32_t numSpotLights = 0;
	uint32_t numPointLights = 0;
	for(auto it=lightsOut.begin();it!=lightsOut.end();)
	{
		auto *light = *it;
		auto type = LightType::Invalid;
		auto *pLight = light->GetLight(type);
		if(type == LightType::Spot)
		{
			if(++numSpotLights > static_cast<uint32_t>(GameLimits::MaxActiveShadowMaps))
				it = lightsOut.erase(it);
			else
				++it;
		}
		else if(type == LightType::Point)
		{
			if(++numPointLights > static_cast<uint32_t>(GameLimits::MaxActiveShadowCubeMaps))
				it = lightsOut.erase(it);
			else
				++it;
		}
		else
			++it;
	}
	if(lightsOut.size() > static_cast<uint32_t>(GameLimits::MaxAbsoluteLights))
		lightsOut.resize(static_cast<uint32_t>(GameLimits::MaxAbsoluteLights));
}
void OcclusionCullingHandler::PerformCulling(const rendering::RasterizationRenderer &renderer,const Vector3 &origin,float radius,std::vector<pragma::OcclusionMeshInfo> &culledMeshesOut)
{
	auto &cam = *c_game->GetRenderCamera();
	auto &posCam = cam.GetPos();
	auto &scene = renderer.GetScene();
	auto &ents = scene.GetEntities();
	auto radiusSqr = umath::pow(radius,2.f);
	culledMeshesOut.clear();
	culledMeshesOut.reserve(10);
	for(auto &hEnt : ents)
	{
		if(hEnt.IsValid() == false)
			continue;
		auto *ent = static_cast<CBaseEntity*>(hEnt.get());
		auto pRenderComponent = (ent != nullptr) ? ent->GetRenderComponent() : util::WeakHandle<pragma::CRenderComponent>{};
		if(pRenderComponent.valid() && pRenderComponent->ShouldDrawShadow(posCam))
		{
			auto pTrComponent = ent->GetTransformComponent();
			auto sphere = pRenderComponent->GetRenderSphereBounds();
			auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
			if(uvec::length_sqr((pos +sphere.pos) -origin) <= radiusSqr +umath::pow(sphere.radius,2.f))
			{
				auto &meshes = pRenderComponent->GetLODMeshes();
				for(auto itMesh=meshes.begin();itMesh!=meshes.end();++itMesh)
				{
					auto *mesh = static_cast<CModelMesh*>(itMesh->get());
					Vector3 min;
					Vector3 max;
					mesh->GetBounds(min,max);
					min += pos;
					max += pos;
					if(Intersection::AABBSphere(min,max,origin,radius) == true)
					{
						if(culledMeshesOut.capacity() -culledMeshesOut.size() == 0)
							culledMeshesOut.reserve(culledMeshesOut.capacity() +10);
						culledMeshesOut.push_back(OcclusionMeshInfo{*ent,*mesh});
					}
				}
			}
		}
	}
}

static void cl_render_occlusion_culling_callback(NetworkState*,ConVar*,int,int val)
{
	if(c_game == nullptr)
		return;
	auto &scene = c_game->GetScene();
	if(scene != nullptr)
	{
		auto *renderer = scene->GetRenderer();
		if(renderer && renderer->IsRasterizationRenderer())
			static_cast<pragma::rendering::RasterizationRenderer*>(renderer)->ReloadOcclusionCullingHandler();
	}
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_occlusion_culling,cl_render_occlusion_culling_callback);
