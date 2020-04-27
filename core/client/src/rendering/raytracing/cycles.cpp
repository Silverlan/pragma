/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/clientstate/clientstate.h"
#include <sharedutils/util_library.hpp>

using namespace pragma::rendering;


struct CyclesModuleInterface
{
	CyclesModuleInterface(util::Library &lib)
	{
		render_image = FindSymbolAddress<decltype(render_image)>(lib,"pr_cycles_render_image");
		bake_ao = FindSymbolAddress<decltype(bake_ao)>(lib,"pr_cycles_bake_ao");
		bake_lightmaps = FindSymbolAddress<decltype(bake_lightmaps)>(lib,"pr_cycles_bake_lightmaps");
	}
	void(*render_image)(
		uint32_t,uint32_t,uint32_t,bool,bool,
		const Vector3&,const Quat&,const Mat4&,float,float,umath::Degree,
		bool,std::string,EulerAngles,float,uint32_t,
		const std::function<bool(BaseEntity&)>&,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>&
	) = nullptr;

	void(*bake_ao)(Model&,uint32_t,uint32_t,uint32_t,uint32_t,bool,bool,const std::string&,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob) = nullptr;
	void(*bake_lightmaps)(BaseEntity&,uint32_t,uint32_t,uint32_t,bool,bool,std::string,EulerAngles,float,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob) = nullptr;

	bool IsValid() const {return m_bValid;}
private:
	template<typename TFunc>
		TFunc FindSymbolAddress(util::Library &lib,const std::string &name)
	{
		auto *p = lib.FindSymbolAddress<TFunc>(name);
		if(p == nullptr)
			m_bValid = false;
		return p;
	}
	bool m_bValid = true;
};

static std::optional<CyclesModuleInterface> initialize_library(ClientState &client)
{
	std::string err;
	auto hLib = client.InitializeLibrary("cycles/pr_cycles",&err);
	if(hLib == nullptr)
	{
		Con::cwar<<"WARNING: Unable to create cycles scene for raytracing: Unable to load module: "<<err<<Con::endl;
		return {};
	}
	CyclesModuleInterface cyclesInterface {*hLib};
	return cyclesInterface.IsValid() ? cyclesInterface : std::optional<CyclesModuleInterface>{};
}

util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> cycles::render_image(ClientState &client,const SceneInfo &sceneInfo,const RenderImageInfo &renderImageInfo)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	auto fEntityFilter = renderImageInfo.entityFilter ? renderImageInfo.entityFilter : [](BaseEntity &ent) -> bool {
		return true;
	};
	util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> job = {};
	cyclesInterface->render_image(
		sceneInfo.width,sceneInfo.height,sceneInfo.samples,sceneInfo.hdrOutput,sceneInfo.denoise,
		renderImageInfo.cameraPosition,renderImageInfo.cameraRotation,renderImageInfo.viewProjectionMatrix,renderImageInfo.nearZ,renderImageInfo.farZ,renderImageInfo.fov,
		sceneInfo.cullObjectsOutsidePvs,sceneInfo.sky,sceneInfo.skyAngles,sceneInfo.skyStrength,sceneInfo.maxTransparencyBounces,
		fEntityFilter,job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}
util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> cycles::bake_ambient_occlusion(ClientState &client,const SceneInfo &sceneInfo,Model &mdl,uint32_t materialIndex)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> job = {};
	auto sDeviceType = (sceneInfo.device == SceneInfo::DeviceType::GPU) ? "gpu" : "cpu";
	cyclesInterface->bake_ao(
		mdl,materialIndex,sceneInfo.width,sceneInfo.height,sceneInfo.samples,sceneInfo.hdrOutput,sceneInfo.denoise,sDeviceType,job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}
util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> cycles::bake_lightmaps(ClientState &client,const SceneInfo &sceneInfo,BaseEntity &entTarget)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> job = {};
	cyclesInterface->bake_lightmaps(
		entTarget,sceneInfo.width,sceneInfo.height,sceneInfo.samples,sceneInfo.hdrOutput,sceneInfo.denoise,
		sceneInfo.sky,sceneInfo.skyAngles,sceneInfo.skyStrength,
		job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}

