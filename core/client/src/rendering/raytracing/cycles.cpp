#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/clientstate/clientstate.h"
#include <sharedutils/util_library.hpp>

using namespace pragma::rendering;

#pragma optimize("",off)
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
		const Vector3&,const Quat&,float,float,umath::Degree,
		const std::function<bool(BaseEntity&)>&,util::ParallelJob<std::shared_ptr<util::ImageBuffer>> &outJob
	) = nullptr;

	void(*bake_ao)(Model&,uint32_t,uint32_t,uint32_t,uint32_t,bool,bool,util::ParallelJob<std::shared_ptr<util::ImageBuffer>> &outJob) = nullptr;
	void(*bake_lightmaps)(BaseEntity&,uint32_t,uint32_t,uint32_t,bool,bool,util::ParallelJob<std::shared_ptr<util::ImageBuffer>> &outJob) = nullptr;

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

util::ParallelJob<std::shared_ptr<util::ImageBuffer>> cycles::render_image(ClientState &client,const SceneInfo &sceneInfo,const RenderImageInfo &renderImageInfo)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	auto fEntityFilter = renderImageInfo.entityFilter ? renderImageInfo.entityFilter : [](BaseEntity &ent) -> bool {
		return true;
	};
	util::ParallelJob<std::shared_ptr<util::ImageBuffer>> job = {};
	cyclesInterface->render_image(
		sceneInfo.width,sceneInfo.height,sceneInfo.samples,sceneInfo.hdrOutput,sceneInfo.denoise,
		renderImageInfo.cameraPosition,renderImageInfo.cameraRotation,renderImageInfo.nearZ,renderImageInfo.farZ,renderImageInfo.fov,
		fEntityFilter,job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}
util::ParallelJob<std::shared_ptr<util::ImageBuffer>> cycles::bake_ambient_occlusion(ClientState &client,const SceneInfo &sceneInfo,Model &mdl,uint32_t materialIndex)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<std::shared_ptr<util::ImageBuffer>> job = {};
	cyclesInterface->bake_ao(
		mdl,materialIndex,sceneInfo.width,sceneInfo.height,sceneInfo.samples,sceneInfo.hdrOutput,sceneInfo.denoise,job
	);
	if(job.IsValid() == false)
		return {};
	return job;
}
util::ParallelJob<std::shared_ptr<util::ImageBuffer>> cycles::bake_lightmaps(ClientState &client,const SceneInfo &sceneInfo,BaseEntity &entTarget)
{
	auto cyclesInterface = initialize_library(client);
	if(cyclesInterface.has_value() == false)
		return {};
	util::ParallelJob<std::shared_ptr<util::ImageBuffer>> job = {};
	cyclesInterface->bake_lightmaps(entTarget,sceneInfo.width,sceneInfo.height,sceneInfo.samples,sceneInfo.hdrOutput,sceneInfo.denoise,job);
	if(job.IsValid() == false)
		return {};
	return job;
}
#pragma optimize("",on)
