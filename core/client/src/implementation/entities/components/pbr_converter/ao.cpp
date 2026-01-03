// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.util_pbr_converter;
import :client_state;
import :engine;

using namespace pragma;

PBRAOBakeJob::PBRAOBakeJob(asset::Model &mdl, material::Material &mat) : hModel {mdl.GetHandle()}, hMaterial {mat.GetHandle()} {}

void CPBRConverterComponent::ProcessQueue()
{
	if(m_workQueue.empty())
		return;
	auto &item = m_workQueue.front();
	if(item.isRunning) {
		if(item.job.IsComplete()) {
			// Job is complete, just remove the item from the queue and continue with the next one
			m_workQueue.pop();
			ProcessQueue();
		}
		return;
	}
	item.isRunning = true;
	if(item.hModel.expired() || item.hMaterial == nullptr)
		return;
	auto &mat = *item.hMaterial.get();
	auto &mdl = *item.hModel.get();
	auto &mats = mdl.GetMaterials();
	auto itMat = std::find_if(mats.begin(), mats.end(), [&mat](const material::MaterialHandle &hMat) { return hMat.get() == &mat; });
	assert(itMat != mats.end());
	if(itMat == mats.end())
		return; // Material doesn't exist in model, this shouldn't happen!
	rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.denoise = true;
	sceneInfo.hdrOutput = false;
	sceneInfo.width = item.width;
	sceneInfo.height = item.height;
	sceneInfo.samples = item.samples;

	if(item.hEntity.valid())
		item.job = rendering::cycles::bake_ambient_occlusion(*get_client_state(), sceneInfo, *item.hEntity.get(), itMat - mats.begin() /* materialIndex */);
	else
		item.job = rendering::cycles::bake_ambient_occlusion(*get_client_state(), sceneInfo, *item.hModel.get(), itMat - mats.begin() /* materialIndex */);
	if(item.job.IsValid() == false)
		return;
	auto hMat = item.hMaterial;
	auto hMdl = item.hModel;
	item.job.SetCompletionHandler([this, hMat, hMdl](util::ParallelWorker<image::ImageLayerSet> &worker) {
		if(worker.IsSuccessful() == false) {
			Con::CWAR << "Generating ambient occlusion map failed: " << worker.GetResultMessage() << Con::endl;
			return;
		}
		if(hMat == nullptr || hMdl.expired())
			return;
		auto imgBuffer = worker.GetResult().images.begin()->second;
		WriteAOMap(*hMdl.get(), const_cast<material::CMaterial &>(static_cast<const material::CMaterial &>(*hMat.get())), *imgBuffer, imgBuffer->GetWidth(), imgBuffer->GetHeight());
	});
	item.job.Start();
	get_cengine()->AddParallelJob(item.job, "Ambient Occlusion");
}

void CPBRConverterComponent::UpdateAmbientOcclusion(asset::Model &mdl, const AmbientOcclusionInfo &aoInfo, ecs::BaseEntity *optEnt)
{
	ConvertMaterialsToPBR(mdl);

	// We will have to generate ao maps for all materials of the first skin.
	// For all other skins, we can just re-use the generated ones.
	auto *texGroup = mdl.GetTextureGroup(0);
	if(texGroup == nullptr)
		return;
	for(auto texId : texGroup->textures) {
		auto *mat = static_cast<material::CMaterial *>(mdl.GetMaterial(texId));
		if(mat == nullptr)
			continue;
		// Make sure it's a PBR material and it doesn't already have an ao map
		if(IsPBR(*mat) == false)
			continue;
		if(aoInfo.rebuild == false && mat->GetProperty("rma_info/requires_ao_update", false) == false)
			continue;
		PBRAOBakeJob job {mdl, *mat};
		job.width = aoInfo.width;
		job.height = aoInfo.height;
		job.samples = aoInfo.samples;
		job.hEntity = optEnt ? optEnt->GetHandle() : EntityHandle {};
		m_workQueue.push(job);
	}
	SetTickPolicy(TickPolicy::Always);
}
void CPBRConverterComponent::WriteAOMap(asset::Model &mdl, material::CMaterial &mat, image::ImageBuffer &imgBuffer, uint32_t w, uint32_t h) const
{
	Con::COUT << "Ambient occlusion map has been generated for material '" << mat.GetName() << "' of model '" << mdl.GetName() << "'! Combining with RMA map..." << Con::endl;

	auto *shader = static_cast<ShaderComposeRMA *>(get_cengine()->GetShader("compose_rma").get());
	if(shader == nullptr)
		return;
	auto *texInfoRMA = mat.GetRMAMap();
	if(texInfoRMA == nullptr)
		return;
	auto resWatcherLock = get_cengine()->ScopeLockResourceWatchers();
	auto rmaName = texInfoRMA->name;
	ufile::remove_extension_from_filename(rmaName);

	auto outPath = util::Path {rmaName};
	auto requiresSave = false;
	if(outPath.GetFront() == "pbr") {
		// We don't want to overwrite the default pbr materials, so we'll use a different rma path
		outPath = mat.GetName();
		outPath.RemoveFileExtension();
		rmaName = outPath.GetString() + "_rma";
		mat.SetTextureProperty(material::ematerial::RMA_MAP_IDENTIFIER, rmaName);
		requiresSave = true;
	}

	shader->InsertAmbientOcclusion(get_cengine()->GetRenderContext(), rmaName, imgBuffer);

	if(requiresSave) {
		mat.UpdateTextures();
		std::string err;
		mat.Save(err);
	}
}
