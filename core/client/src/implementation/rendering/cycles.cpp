// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.cycles;
import :client_state;

using namespace pragma::rendering;

struct CyclesModuleInterface {
	CyclesModuleInterface(pragma::util::Library &lib)
	{
		render_image = FindSymbolAddress<decltype(render_image)>(lib, "pr_cycles_render_image");
		bake_ao = FindSymbolAddress<decltype(bake_ao)>(lib, "pr_cycles_bake_ao");
		bake_ao_ent = FindSymbolAddress<decltype(bake_ao_ent)>(lib, "pr_cycles_bake_ao_ent");
		bake_lightmaps = FindSymbolAddress<decltype(bake_lightmaps)>(lib, "pr_cycles_bake_lightmaps");
	}
	void (*render_image)(const cycles::SceneInfo &sceneInfo, const cycles::RenderImageInfo &renderImageInfo, const std::function<bool(pragma::ecs::BaseEntity &)> &entFilter, pragma::util::ParallelJob<pragma::image::ImageLayerSet> &outJob) = nullptr;

	void (*bake_ao)(const cycles::SceneInfo &sceneInfo, pragma::asset::Model &mdl, uint32_t materialIndex, pragma::util::ParallelJob<pragma::image::ImageLayerSet> &outJob) = nullptr;
	void (*bake_ao_ent)(const cycles::SceneInfo &sceneInfo, pragma::ecs::BaseEntity &ent, uint32_t materialIndex, pragma::util::ParallelJob<pragma::image::ImageLayerSet> &outJob) = nullptr;
	void (*bake_lightmaps)(const cycles::SceneInfo &sceneInfo, pragma::util::ParallelJob<pragma::image::ImageLayerSet> &outJob) = nullptr;

	bool IsValid() const { return m_bValid; }
  private:
	template<typename TFunc>
	TFunc FindSymbolAddress(pragma::util::Library &lib, const std::string &name)
	{
		auto *p = lib.FindSymbolAddress<TFunc>(name);
		if(p == nullptr)
			m_bValid = false;
		return p;
	}
	bool m_bValid = true;
};

static std::optional<CyclesModuleInterface> initialize_library(pragma::ClientState &client)
{
	std::string err;
	auto hLib = client.InitializeLibrary("unirender/pr_unirender", &err);
	if(hLib == nullptr) {
		Con::CWAR << "Unable to create unirender scene for raytracing: Unable to load module: " << err << Con::endl;
		return {};
	}
	CyclesModuleInterface cyclesInterface {*hLib};
	return cyclesInterface.IsValid() ? cyclesInterface : std::optional<CyclesModuleInterface> {};
}

pragma::util::ParallelJob<pragma::image::ImageLayerSet> cycles::render_image(ClientState &client, const SceneInfo &sceneInfo, const RenderImageInfo &renderImageInfo)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	auto fEntityFilter = renderImageInfo.entityFilter ? renderImageInfo.entityFilter : [](ecs::BaseEntity &ent) -> bool { return true; };
	util::ParallelJob<image::ImageLayerSet> job = {};
	cyclesInterface->render_image(sceneInfo, renderImageInfo, fEntityFilter, job);
	if(job.IsValid() == false)
		return {};
	return job;
}
pragma::util::ParallelJob<pragma::image::ImageLayerSet> cycles::bake_ambient_occlusion(ClientState &client, const SceneInfo &sceneInfo, ecs::BaseEntity &ent, uint32_t materialIndex)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<image::ImageLayerSet> job = {};
	cyclesInterface->bake_ao_ent(sceneInfo, ent, materialIndex, job);
	if(job.IsValid() == false)
		return {};
	return job;
}
pragma::util::ParallelJob<pragma::image::ImageLayerSet> cycles::bake_ambient_occlusion(ClientState &client, const SceneInfo &sceneInfo, asset::Model &mdl, uint32_t materialIndex)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<image::ImageLayerSet> job = {};
	cyclesInterface->bake_ao(sceneInfo, mdl, materialIndex, job);
	if(job.IsValid() == false)
		return {};
	return job;
}
pragma::util::ParallelJob<pragma::image::ImageLayerSet> cycles::bake_lightmaps(ClientState &client, const SceneInfo &sceneInfo)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<image::ImageLayerSet> job = {};
	cyclesInterface->bake_lightmaps(sceneInfo, job);
	if(job.IsValid() == false)
		return {};
	return job;
}
