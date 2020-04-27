/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "cmaterial.h"
#include <pragma/physics/collisionmesh.h>
#include <pragma/physics/transform.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_path.hpp>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

PBRAOBakeJob::PBRAOBakeJob(Model &mdl,Material &mat)
	: hModel{mdl.GetHandle()},hMaterial{mat.GetHandle()}
{}

void CPBRConverterComponent::ProcessQueue()
{
	if(m_workQueue.empty())
		return;
	auto &item = m_workQueue.front();
	if(item.isRunning)
	{
		if(item.job.IsComplete())
		{
			// Job is complete, just remove the item from the queue and continue with the next one
			m_workQueue.pop();
			ProcessQueue();
		}
		return;
	}
	item.isRunning = true;
	if(item.hModel.expired() || item.hMaterial.IsValid() == false)
		return;
	auto &mat = *item.hMaterial.get();
	auto &mdl = *item.hModel.get();
	auto &mats = mdl.GetMaterials();
	auto itMat = std::find_if(mats.begin(),mats.end(),[&mat](const MaterialHandle &hMat) {
		return hMat.get() == &mat;
	});
	assert(itMat != mats.end());
	if(itMat == mats.end())
		return; // Material doesn't exist in model, this shouldn't happen!
	pragma::rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.denoise = true;
	sceneInfo.hdrOutput = false;
	sceneInfo.width = item.width;
	sceneInfo.height = item.height;
	sceneInfo.samples = item.samples;

	item.job = rendering::cycles::bake_ambient_occlusion(*client,sceneInfo,*item.hModel.get(),itMat -mats.begin() /* materialIndex */);
	if(item.job.IsValid() == false)
		return;
	auto hMat = item.hMaterial;
	auto hMdl = item.hModel;
	item.job.SetCompletionHandler([this,hMat,hMdl](util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>> &worker) {
		if(worker.IsSuccessful() == false)
		{
			Con::cwar<<"WARNING: Generating ambient occlusion map failed: "<<worker.GetResultMessage()<<Con::endl;
			return;
		}
		if(hMat.IsValid() == false || hMdl.expired())
			return;
		auto imgBuffer = worker.GetResult();
		WriteAOMap(*hMdl.get(),static_cast<CMaterial&>(*hMat.get()),*imgBuffer,imgBuffer->GetWidth(),imgBuffer->GetHeight());
	});
	item.job.Start();
	c_engine->AddParallelJob(item.job,"Ambient Occlusion");
}

void CPBRConverterComponent::UpdateAmbientOcclusion(Model &mdl,const AmbientOcclusionInfo &aoInfo)
{
	// TODO: Re-enable this (but don't run it automatically?)
#if 0
	ConvertMaterialsToPBR(mdl);

	// We will have to generate ao maps for all materials of the first skin.
	// For all other skins, we can just re-use the generated ones.
	auto *texGroup = mdl.GetTextureGroup(0);
	if(texGroup == nullptr)
		return;
	for(auto texId : texGroup->textures)
	{
		auto *mat = static_cast<CMaterial*>(mdl.GetMaterial(texId));
		if(mat == nullptr)
			continue;
		// Make sure it's a PBR material and it doesn't already have an ao map
		if(IsPBR(*mat) == false)
			continue;
		auto rmaInfo = mat->GetDataBlock()->GetBlock("rma_info");
		if(aoInfo.rebuild == false && (rmaInfo == nullptr || rmaInfo->GetBool("requires_ao_update") == false))
			continue;
		PBRAOBakeJob job {mdl,*mat};
		job.width = aoInfo.width;
		job.height = aoInfo.height;
		job.samples = aoInfo.samples;
		m_workQueue.push(job);
	}
#endif
}
void CPBRConverterComponent::WriteAOMap(Model &mdl,CMaterial &mat,uimg::ImageBuffer &imgBuffer,uint32_t w,uint32_t h) const
{
	Con::cout<<"Ambient occlusion map has been generated for material '"<<mat.GetName()<<"' of model '"<<mdl.GetName()<<"'! Combining with RMA map..."<<Con::endl;

	auto *shader = static_cast<pragma::ShaderComposeRMA*>(c_engine->GetShader("compose_rma").get());
	if(shader == nullptr)
		return;
	auto *texInfoRMA = mat.GetRMAMap();
	if(texInfoRMA == nullptr)
		return;
	auto resWatcherLock = c_engine->ScopeLockResourceWatchers();
	auto rmaName = texInfoRMA->name;
	ufile::remove_extension_from_filename(rmaName);

	auto outPath = util::Path{rmaName};
	auto requiresSave = false;
	if(outPath.GetFront() == "pbr")
	{
		// We don't want to overwrite the default pbr materials, so we'll use a different rma path
		outPath = mat.GetName();
		outPath.RemoveFileExtension();
		rmaName = outPath.GetString() +"_rma";
		mat.GetDataBlock()->AddValue("texture",Material::RMA_MAP_IDENTIFIER,rmaName);
		requiresSave = true;
	}

	shader->InsertAmbientOcclusion(*c_engine,rmaName,imgBuffer);

	if(requiresSave)
	{
		mat.UpdateTextures();
		mat.Save();
	}
}
