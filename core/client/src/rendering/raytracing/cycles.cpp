#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/clientstate/clientstate.h"
#include <sharedutils/util_library.hpp>

using namespace pragma::rendering;

#pragma optimize("",off)
util::WeakHandle<cycles::Scene> cycles::Scene::Create(ClientState &client,const CreateInfo &createInfo)
{
	std::string err;
	auto hLib = client.InitializeLibrary("cycles/pr_cycles",&err);
	if(hLib == nullptr)
	{
		Con::cwar<<"WARNING: Unable to create cycles scene for raytracing: Unable to load module: "<<err<<Con::endl;
		return nullptr;
	}

	auto scene = std::shared_ptr<Scene>{new Scene{}};
	scene->m_scene = scene;
	scene->f_calc_raytraced_scene = hLib->FindSymbolAddress<decltype(f_calc_raytraced_scene)>("pr_cycles_calc_raytraced_scene");
	scene->f_cycles_bake_lighting = hLib->FindSymbolAddress<decltype(f_cycles_bake_lighting)>("pr_cycles_bake_lighting");
	scene->f_cycles_get_scene_process = hLib->FindSymbolAddress<decltype(f_cycles_get_scene_process)>("pr_cycles_get_scene_process");
	scene->f_cycles_is_scene_complete = hLib->FindSymbolAddress<decltype(f_cycles_is_scene_complete)>("pr_cycles_is_scene_complete");
	scene->f_cycles_cancel_scene = hLib->FindSymbolAddress<decltype(f_cycles_cancel_scene)>("pr_cycles_cancel_scene");
	scene->f_cycles_set_scene_progress_callback = hLib->FindSymbolAddress<decltype(f_cycles_set_scene_progress_callback)>("pr_cycles_set_scene_progress_callback");
	scene->f_cycles_is_scene_cancelled = hLib->FindSymbolAddress<decltype(f_cycles_is_scene_cancelled)>("pr_cycles_is_scene_cancelled");
	scene->f_cycles_wait_for_scene_completion = hLib->FindSymbolAddress<decltype(f_cycles_wait_for_scene_completion)>("pr_cycles_wait_for_scene_completion");
	if(
		scene->f_calc_raytraced_scene == nullptr ||
		scene->f_cycles_bake_lighting == nullptr ||
		scene->f_cycles_get_scene_process == nullptr ||
		scene->f_cycles_is_scene_complete == nullptr ||
		scene->f_cycles_cancel_scene == nullptr ||
		scene->f_cycles_set_scene_progress_callback == nullptr ||
		scene->f_cycles_is_scene_cancelled == nullptr ||
		scene->f_cycles_wait_for_scene_completion == nullptr
	)
	{
		Con::cwar<<"WARNING: Unable to create cycles scene for raytracing: Symbol(s) not found!"<<Con::endl;
		return nullptr;
	}
	auto *ptrScene = scene.get();
	auto outputHandler = createInfo.outputHandler;
	auto fEntityFilter = createInfo.entityFilter ? createInfo.entityFilter : [](BaseEntity &ent) -> bool {
		return true;
	};
	switch(createInfo.type)
	{
	case Type::RenderImage:
		scene->f_calc_raytraced_scene(
			createInfo.width,createInfo.height,createInfo.samples,createInfo.hdrOutput,createInfo.denoise,
			createInfo.cameraPosition,createInfo.cameraRotation,createInfo.nearZ,createInfo.farZ,createInfo.fov,
			fEntityFilter,
			[outputHandler,ptrScene](const uint8_t *data,int width,int height,int channels) {
				assert(channels == 4);
				if(channels != 4 || ptrScene->IsCancelled())
					return;
				outputHandler(data,width,height);
			},scene->m_moduleScene
		);
		break;
	case Type::BakeDiffuseLighting:
	case Type::BakeAmbientOcclusion: // TODO
		scene->f_cycles_bake_lighting(
			createInfo.width,createInfo.height,createInfo.samples,createInfo.denoise,createInfo.nearZ,createInfo.farZ,createInfo.fov,
			[outputHandler,ptrScene](const uint8_t *data,int width,int height,int channels) {
				assert(channels == 4);
				if(channels != 4 || ptrScene->IsCancelled())
					return;
				outputHandler(data,width,height);
			},scene->m_moduleScene
		);
		break;
	}

	if(scene->m_moduleScene == nullptr)
		return util::WeakHandle<cycles::Scene>{};
	scene->f_cycles_set_scene_progress_callback(scene->m_moduleScene,[ptrScene](float progress) {
		ptrScene->OnProgressChanged(progress);
	});
	scene->m_cbThink = client.AddCallback("Think",FunctionCallback<void>::Create([ptrScene]() mutable {
		ptrScene->Update();
	}));
	scene->m_cbOnClose = client.AddCallback("OnClose",FunctionCallback<void>::Create([ptrScene]() {
		ptrScene->Cancel();
	}));
	return scene;
}
cycles::Scene::~Scene()
{
	m_moduleScene = nullptr;
	if(m_cbThink.IsValid())
		m_cbThink.Remove();
	if(m_cbOnClose.IsValid())
		m_cbOnClose.Remove();
}
bool cycles::Scene::Update()
{
	if(f_cycles_is_scene_complete(m_moduleScene) == false)
		return false;
	if(IsCancelled() == false)
		f_cycles_wait_for_scene_completion(m_moduleScene); // Make sure it's 100% complete
	m_scene = nullptr; // Destroy the last instance of the scene; This will also invoke the output handler
	return true;
}
void cycles::Scene::SetProgressCallback(const std::function<void(float)> &progressCallback)
{
	m_progressCallback = progressCallback;
}
void cycles::Scene::OnProgressChanged(float progress)
{
	if(m_progressCallback)
		m_progressCallback(progress);
}
float cycles::Scene::GetProgress() const
{
	return f_cycles_get_scene_process(m_moduleScene);
}
bool cycles::Scene::IsComplete() const
{
	return m_bComplete;
}
bool cycles::Scene::IsCancelled() const
{
	return m_bCancelled;
}
void cycles::Scene::Cancel()
{
	m_bCancelled = true;
	if(IsComplete() == false)
		f_cycles_cancel_scene(m_moduleScene);
}
#pragma optimize("",on)
