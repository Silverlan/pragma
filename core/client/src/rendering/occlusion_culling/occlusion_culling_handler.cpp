/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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

bool OcclusionCullingHandler::ShouldExamine(CModelMesh &mesh,const Vector3 &pos,bool bViewModel,std::size_t numMeshes,const std::vector<Plane> *planes) const
{
	if((planes == nullptr) || bViewModel == true || numMeshes == 1)
		return true;
	Vector3 min,max;
	mesh.GetBounds(min,max);
	min += pos;
	max += pos;
	return (Intersection::AABBInPlaneMesh(min,max,*planes) != INTERSECT_OUTSIDE) ? true : false;
}
bool OcclusionCullingHandler::ShouldExamine(const rendering::RasterizationRenderer &renderer,CBaseEntity &ent,bool &outViewModel,std::vector<Plane> **outPlanes) const
{
	if(outPlanes == nullptr)
		return true; // Skip the frustum culling
	auto &scene = renderer.GetScene();
	auto *cam = c_game->GetPrimaryCamera();
	auto &posCam = cam ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	auto pRenderComponent = ent.GetRenderComponent();
	if(ent.IsSpawned() == false || ent.IsInScene(scene) == false || pRenderComponent.expired() || pRenderComponent->ShouldDraw(posCam) == false)
		return false;
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return false;
	outViewModel = (pRenderComponent->GetRenderMode() == RenderMode::View) ? true : false; // TODO: Remove me once the render bounds accurately encompass animation bounds
	*outPlanes = (pRenderComponent->GetRenderMode() == RenderMode::Skybox) ? const_cast<std::vector<Plane>*>(&renderer.GetFrustumPlanes()) : const_cast<std::vector<Plane>*>(&renderer.GetClippedFrustumPlanes());
	if(pRenderComponent->IsExemptFromOcclusionCulling() || outViewModel)
		return true; // Always draw
	auto sphere = pRenderComponent->GetRenderSphereBounds();
	umath::Transform pose;
	ent.GetPose(pose);
	auto pos = pose.GetOrigin();
	if(Intersection::SphereInPlaneMesh(pos +sphere.pos,sphere.radius,*(*outPlanes),true) == INTERSECT_OUTSIDE)
		return false;
	Vector3 min;
	Vector3 max;
	pRenderComponent->GetRenderBounds(&min,&max);
	min = pose *min;
	max = pose *max;
	uvec::to_min_max(min,max);
	return Intersection::AABBInPlaneMesh(min,max,*(*outPlanes)) != INTERSECT_OUTSIDE;
}
void OcclusionCullingHandler::PerformCulling(const rendering::RasterizationRenderer &renderer,std::vector<pragma::CParticleSystemComponent*> &particlesOut)
{
	auto &scene = renderer.GetScene();
	auto &cam = scene.GetActiveCamera();
	auto &posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	PerformCulling(renderer,posCam,particlesOut);
}
void OcclusionCullingHandler::PerformCulling(const rendering::RasterizationRenderer &renderer,std::vector<OcclusionMeshInfo> &culledMeshesOut)
{
	auto &scene = renderer.GetScene();
	auto &cam = scene.GetActiveCamera();
	auto &posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	PerformCulling(renderer,posCam,culledMeshesOut);
}
void OcclusionCullingHandler::PerformCulling(
	const rendering::RasterizationRenderer &renderer,const Vector3 &camPos,
	std::vector<pragma::CParticleSystemComponent*> &particlesOut
)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	particlesOut.reserve(entIt.GetCount());
	particlesOut.clear();
	auto &scene = renderer.GetScene();
	auto &frustumPlanes = renderer.GetFrustumPlanes();
	for(auto *ent : entIt)
	{
		if(static_cast<CBaseEntity*>(ent)->IsInScene(scene) == false)
			continue;
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
	auto &cam = scene.GetActiveCamera();
	auto &pos = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
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
			auto type = util::pragma::LightType::Undefined;
			auto *pLight = light->GetLight(type);
			if(type == util::pragma::LightType::Directional)
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
		auto type = util::pragma::LightType::Undefined;
		auto *pLight = light->GetLight(type);
		if(type == util::pragma::LightType::Spot)
		{
			if(++numSpotLights > static_cast<uint32_t>(GameLimits::MaxActiveShadowMaps))
				it = lightsOut.erase(it);
			else
				++it;
		}
		else if(type == util::pragma::LightType::Point)
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
	auto *cam = c_game->GetPrimaryCamera();
	auto &posCam = cam ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	auto &scene = renderer.GetScene();
	auto radiusSqr = umath::pow(radius,2.f);
	culledMeshesOut.clear();
	culledMeshesOut.reserve(10);
	std::vector<CBaseEntity*> *ents = nullptr;
	c_game->GetEntities(&ents);
	for(auto *ent : *ents)
	{
		if(ent == nullptr)
			continue;
		auto pRenderComponent = (ent != nullptr) ? ent->GetRenderComponent() : util::WeakHandle<pragma::CRenderComponent>{};
		if(pRenderComponent.valid() && pRenderComponent->ShouldDrawShadow(posCam))
		{
			auto exemptFromCulling = pRenderComponent->IsExemptFromOcclusionCulling();
			auto pTrComponent = ent->GetTransformComponent();
			auto sphere = pRenderComponent->GetRenderSphereBounds();
			auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
			if(exemptFromCulling || uvec::length_sqr((pos +sphere.pos) -origin) <= radiusSqr +umath::pow(sphere.radius,2.f))
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
					if(exemptFromCulling || Intersection::AABBSphere(min,max,origin,radius) == true)
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
