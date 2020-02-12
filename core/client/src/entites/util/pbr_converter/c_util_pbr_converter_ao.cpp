#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "cmaterial.h"
#include <pragma/physics/collisionmesh.h>
#include <pragma/physics/transform.hpp>
#include <sharedutils/util_file.h>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <prosper_util.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
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
	if(item.hModel.IsValid() == false || item.hMaterial.IsValid() == false)
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
	// These values are a good compromise between quality and render time
	sceneInfo.width = 512;
	sceneInfo.height = 512;
	sceneInfo.samples = 512;

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
		if(hMat.IsValid() == false || hMdl.IsValid() == false)
			return;
		auto imgBuffer = worker.GetResult();
		WriteAOMap(*hMdl.get(),static_cast<CMaterial&>(*hMat.get()),*imgBuffer,imgBuffer->GetWidth(),imgBuffer->GetHeight());
	});
	item.job.Start();
	c_engine->AddParallelJob(item.job,"Ambient Occlusion");
}

void CPBRConverterComponent::UpdateAmbientOcclusion(Model &mdl)
{
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
		if(IsPBR(*mat) == false || mat->GetAmbientOcclusionMap())
			continue;
		PBRAOBakeJob job {mdl,*mat};
		m_workQueue.push(job);
	}
}
void CPBRConverterComponent::ApplyAOMap(CMaterial &mat,const std::string &aoName) const
{
	if(mat.GetAmbientOcclusionMap())
		return; // Don't overwrite the existing ao map
	auto &dataBlock = mat.GetDataBlock();
	dataBlock->AddValue("texture",Material::AO_MAP_IDENTIFIER,aoName);
	mat.UpdateTextures();
	if(mat.Save(mat.GetName(),"addons/converted/") == false)
		return;
	auto nameNoExt = mat.GetName();
	ufile::remove_extension_from_filename(nameNoExt);
	client->LoadMaterial(nameNoExt,true,true); // Reload material immediately
}
void CPBRConverterComponent::WriteAOMap(Model &mdl,CMaterial &mat,uimg::ImageBuffer &imgBuffer,uint32_t w,uint32_t h) const
{
	Con::cout<<"Ambient occlusion map has been generated for material '"<<mat.GetName()<<"' of model '"<<mdl.GetName()<<"'! Saving texture file..."<<Con::endl;

	uimg::TextureInfo imgWriteInfo {};
	imgWriteInfo.alphaMode = uimg::TextureInfo::AlphaMode::None;
	imgWriteInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
	imgWriteInfo.flags = uimg::TextureInfo::Flags::GenerateMipmaps;
	imgWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::R8G8B8A8_UInt;
	imgWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::GradientMap;

	std::string materialsRootDir = "materials/";
	auto matName = materialsRootDir +mat.GetName();
	ufile::remove_extension_from_filename(matName);

	std::string aoName = matName +"_ao";
	Con::cout<<"Writing ambient occlusion texture file '"<<aoName<<"'..."<<Con::endl;
	if(c_game->SaveImage(imgBuffer,"addons/converted/" +aoName,imgWriteInfo) == false)
		aoName = "";

	if(aoName.empty() == false)
		aoName = aoName.substr(materialsRootDir.length());
	else
		aoName = "white";

	ApplyAOMap(mat,aoName);

	// Apply to skins as well
	auto &mats = mdl.GetMaterials();
	auto itMat = std::find_if(mats.begin(),mats.end(),[&mat](const MaterialHandle &hMat) {
		return hMat.get() == &mat;
	});
	if(itMat == mats.end())
		return;
	auto matIdx = itMat -mats.begin();
	auto &texGroups = mdl.GetTextureGroups();
	if(texGroups.size() < 2)
		return;
	auto &mainTexGroup = texGroups.front();
	auto itTex = std::find(mainTexGroup.textures.begin(),mainTexGroup.textures.end(),matIdx);
	if(itTex == mainTexGroup.textures.end())
		return;
	auto texIdx = itTex -mainTexGroup.textures.begin();
	for(auto i=decltype(texGroups.size()){1};i<texGroups.size();++i)
	{
		auto &texGroup = texGroups.at(i);
		if(texIdx >= texGroup.textures.size())
			continue;
		auto matIdx = texGroup.textures.at(texIdx);
		auto *mat = mdl.GetMaterial(matIdx);
		if(mat == nullptr)
			continue;
		ApplyAOMap(static_cast<CMaterial&>(*mat),aoName);
	}
}
#pragma optimize("",on)
