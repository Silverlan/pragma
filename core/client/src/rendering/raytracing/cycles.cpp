/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
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
		bake_ao_ent = FindSymbolAddress<decltype(bake_ao_ent)>(lib,"pr_cycles_bake_ao_ent");
		bake_lightmaps = FindSymbolAddress<decltype(bake_lightmaps)>(lib,"pr_cycles_bake_lightmaps");
	}
	void(*render_image)(
		const pragma::rendering::cycles::SceneInfo &sceneInfo,const pragma::rendering::cycles::RenderImageInfo &renderImageInfo,const std::function<bool(BaseEntity&)> &entFilter,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob
	) = nullptr;

	void(*bake_ao)(const pragma::rendering::cycles::SceneInfo &sceneInfo,Model &mdl,uint32_t materialIndex,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob) = nullptr;
	void(*bake_ao_ent)(const pragma::rendering::cycles::SceneInfo &sceneInfo,BaseEntity &ent,uint32_t materialIndex,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob) = nullptr;
	void(*bake_lightmaps)(const pragma::rendering::cycles::SceneInfo &sceneInfo,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob) = nullptr;

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
		sceneInfo,renderImageInfo,
		fEntityFilter,job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}
util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> cycles::bake_ambient_occlusion(ClientState &client,const SceneInfo &sceneInfo,BaseEntity &ent,uint32_t materialIndex)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> job = {};
	cyclesInterface->bake_ao_ent(
		sceneInfo,ent,materialIndex,job
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
	cyclesInterface->bake_ao(
		sceneInfo,mdl,materialIndex,job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}
util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> cycles::bake_lightmaps(ClientState &client,const SceneInfo &sceneInfo)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> job = {};
	cyclesInterface->bake_lightmaps(
		sceneInfo,
		job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}

