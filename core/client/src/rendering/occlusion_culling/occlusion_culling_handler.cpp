/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

OcclusionMeshInfo::OcclusionMeshInfo(CBaseEntity &ent, CModelMesh &mesh) : mesh {&mesh}, hEntity {ent.GetHandle()} {}

bool OcclusionCullingHandler::ShouldExamine(CModelMesh &mesh, const Vector3 &pos, bool bViewModel, std::size_t numMeshes, const std::vector<umath::Plane> *planes) const
{
	if((planes == nullptr) || bViewModel == true || numMeshes == 1)
		return true;
	Vector3 min, max;
	mesh.GetBounds(min, max);
	min += pos;
	max += pos;
	return (umath::intersection::aabb_in_plane_mesh(min, max, planes->begin(), planes->end()) != umath::intersection::Intersect::Outside) ? true : false;
}
bool OcclusionCullingHandler::ShouldExamine(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CBaseEntity &ent, bool &outViewModel, std::vector<umath::Plane> **outPlanes) const
{
	if(ent.IsInScene(scene) == false)
		return false;
	if(outPlanes == nullptr)
		return true; // Skip the frustum culling
	auto pRenderComponent = ent.GetRenderComponent();
	if(ent.IsSpawned() == false || !pRenderComponent || pRenderComponent->ShouldDraw() == false)
		return false;
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return false;
	outViewModel = (pRenderComponent->GetSceneRenderPass() == pragma::rendering::SceneRenderPass::View) ? true : false; // TODO: Remove me once the render bounds accurately encompass animation bounds
	*outPlanes = (pRenderComponent->GetSceneRenderPass() == pragma::rendering::SceneRenderPass::Sky) ? const_cast<std::vector<umath::Plane> *>(&renderer.GetFrustumPlanes()) : const_cast<std::vector<umath::Plane> *>(&renderer.GetClippedFrustumPlanes());
	if(pRenderComponent->IsExemptFromOcclusionCulling() || outViewModel)
		return true; // Always draw
	auto &sphere = pRenderComponent->GetUpdatedAbsoluteRenderSphere();
	if(umath::intersection::sphere_in_plane_mesh(sphere.pos, sphere.radius, (*outPlanes)->begin(), (*outPlanes)->end(), true) == umath::intersection::Intersect::Outside)
		return false;
	auto &aabb = pRenderComponent->GetAbsoluteRenderBounds();
	return umath::intersection::aabb_in_plane_mesh(aabb.min, aabb.max, (*outPlanes)->begin(), (*outPlanes)->end()) != umath::intersection::Intersect::Outside;
}
void OcclusionCullingHandler::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, std::vector<pragma::CParticleSystemComponent *> &particlesOut)
{
	auto &cam = scene.GetActiveCamera();
	auto &posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	PerformCulling(scene, renderer, posCam, particlesOut);
}
void OcclusionCullingHandler::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, std::vector<OcclusionMeshInfo> &culledMeshesOut)
{
	auto &cam = scene.GetActiveCamera();
	auto &posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	PerformCulling(scene, renderer, posCam, culledMeshesOut);
}
void OcclusionCullingHandler::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<pragma::CParticleSystemComponent *> &particlesOut)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	particlesOut.reserve(entIt.GetCount());
	particlesOut.clear();
	auto &frustumPlanes = renderer.GetFrustumPlanes();
	for(auto *ent : entIt) {
		if(static_cast<CBaseEntity *>(ent)->IsInScene(scene) == false)
			continue;
		auto hPt = ent->GetComponent<pragma::CParticleSystemComponent>();
		auto &bounds = hPt->GetRenderBounds();
		static auto bAlwaysPass = false;
		if(bAlwaysPass || umath::intersection::aabb_in_plane_mesh(bounds.first, bounds.second, frustumPlanes.begin(), frustumPlanes.end()) != umath::intersection::Intersect::Outside)
			particlesOut.push_back(hPt.get());
	}
}
void OcclusionCullingHandler::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const std::vector<pragma::CLightComponent *> &lightsIn, std::vector<pragma::CLightComponent *> &lightsOut)
{
	auto &cam = scene.GetActiveCamera();
	auto &pos = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	std::vector<float> distances;
	auto sz = lightsIn.size();
	lightsOut.clear();
	lightsOut.reserve(sz);
	distances.reserve(sz);
	auto &frustumPlanes = renderer.GetFrustumPlanes();
	for(auto it = lightsIn.begin(); it != lightsIn.end(); ++it) {
		auto *light = *it;
		if(light == nullptr)
			continue;
		auto &ent = light->GetEntity();
		auto pToggleComponent = ent.GetComponent<pragma::CToggleComponent>();
		if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn()) {
			auto type = pragma::LightType::Undefined;
			auto *pLight = light->GetLight(type);
			if(type == pragma::LightType::Directional) {
				lightsOut.insert(lightsOut.begin(), light);
				distances.push_back(0.f);
			}
			else {
				auto pTrComponent = ent.GetTransformComponent();
				auto pRadiusComponent = ent.GetComponent<pragma::CRadiusComponent>();
				if(pTrComponent != nullptr && pRadiusComponent.valid()) {
					auto &posLight = pTrComponent->GetPosition();
					if(umath::intersection::sphere_in_plane_mesh(posLight, pRadiusComponent->GetRadius(), frustumPlanes.begin(), frustumPlanes.end()) != umath::intersection::Intersect::Outside) {
						auto dist = uvec::length_sqr(posLight - pos);
						auto bInserted = false;
						for(UInt i = 0; i < lightsOut.size(); i++) {
							//auto *lightOther = lightsOut[i];
							auto distOther = distances[i];
							if(dist < distOther) {
								lightsOut.insert(lightsOut.begin() + i, light);
								distances.insert(distances.begin() + i, dist);
								bInserted = true;
								break;
							}
						}
						if(bInserted == false) {
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
	for(auto it = lightsOut.begin(); it != lightsOut.end();) {
		auto *light = *it;
		auto type = pragma::LightType::Undefined;
		auto *pLight = light->GetLight(type);
		if(type == pragma::LightType::Spot) {
			if(++numSpotLights > static_cast<uint32_t>(GameLimits::MaxActiveShadowMaps))
				it = lightsOut.erase(it);
			else
				++it;
		}
		else if(type == pragma::LightType::Point) {
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
void OcclusionCullingHandler::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &origin, float radius, std::vector<pragma::OcclusionMeshInfo> &culledMeshesOut)
{
	auto radiusSqr = umath::pow(radius, 2.f);
	culledMeshesOut.clear();
	culledMeshesOut.reserve(10);

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	for(auto *e : entIt) {
		if(e == nullptr)
			continue;
		auto *ent = static_cast<CBaseEntity *>(e);
		if(ent->IsInScene(scene) == false)
			continue;
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent->ShouldDrawShadow() == false)
			continue;
		auto exemptFromCulling = pRenderComponent->IsExemptFromOcclusionCulling();
		auto pTrComponent = ent->GetTransformComponent();
		auto &sphere = pRenderComponent->GetAbsoluteRenderSphere();
		auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
		if(exemptFromCulling || uvec::length_sqr(sphere.pos - origin) <= radiusSqr + umath::pow(sphere.radius, 2.f)) {
			auto &meshes = pRenderComponent->GetLODMeshes();
			for(auto itMesh = meshes.begin(); itMesh != meshes.end(); ++itMesh) {
				auto *mesh = static_cast<CModelMesh *>(itMesh->get());
				Vector3 min;
				Vector3 max;
				mesh->GetBounds(min, max);
				min += pos;
				max += pos;
				if(exemptFromCulling || umath::intersection::aabb_sphere(min, max, origin, radius) == true) {
					if(culledMeshesOut.capacity() - culledMeshesOut.size() == 0)
						culledMeshesOut.reserve(culledMeshesOut.capacity() + 10);
					culledMeshesOut.push_back(OcclusionMeshInfo {*ent, *mesh});
				}
			}
		}
	}
}

static void cl_render_occlusion_culling_callback(NetworkState *, const ConVar &, int, int val)
{
	/*if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene();
	if(scene != nullptr)
		scene->GetSceneRenderDesc().ReloadOcclusionCullingHandler();*/
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_occlusion_culling, cl_render_occlusion_culling_callback);
