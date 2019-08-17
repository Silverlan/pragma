#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <wrappers/memory_block.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <sharedutils/util_library.hpp>
#include <cmaterialmanager.h>
#include <texturemanager/texturemanager.h>
#include <texture_type.h>
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/rendering/shaders/c_shader_equirectangular_to_cubemap.hpp"
#include "pragma/rendering/shaders/c_shader_convolute_cubemap_lighting.hpp"
#include "pragma/rendering/shaders/c_shader_compute_irradiance_map_roughness.hpp"
#include "pragma/rendering/shaders/c_shader_brdf_convolution.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/pbr/stb_image_write.h"
#include "pr_dds.hpp"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_reflection_probe,CEnvReflectionProbe);

#pragma optimize("",off)
pragma::rendering::IBLData::IBLData(const std::shared_ptr<prosper::Texture> &irradianceMap,const std::shared_ptr<prosper::Texture> &prefilterMap,const std::shared_ptr<prosper::Texture> &brdfMap)
	: irradianceMap{irradianceMap},prefilterMap{prefilterMap},brdfMap{brdfMap}
{}


void CReflectionProbeComponent::BuildAllReflectionProbes(Game &game)
{
	auto *ent = game.CreateEntity("env_reflection_probe");
	ent->Spawn();
	//ent->GetComponent<pragma::CReflectionProbeComponent>()->LoadIBLReflectionsFromFile("env/generic/generic");
	ent->GetComponent<pragma::CReflectionProbeComponent>()->GenerateIBLReflectionsFromEnvMap("newport_loft.hdr");
	

	ent = game.CreateEntity("util_pbr_converter");
	ent->Spawn();
	/*
	EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CReflectionProbeComponent>>();
	for(auto *ent : entIt)
	{
		auto reflProbeC = ent->GetComponent<CReflectionProbeComponent>();
		//std::cout<<"Capture: "<<reflProbeC->CaptureIBLReflectionsFromScene()<<std::endl;
		std::cout<<"GenerateIBLReflectionsFromEnvMap: "<<reflProbeC->GenerateIBLReflectionsFromEnvMap("newport_loft.hdr")<<std::endl;
		std::cout<<"Save: "<<reflProbeC->SaveIBLReflectionsToFile("test.hdr")<<std::endl;
	}*/
}
luabind::object CReflectionProbeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CReflectionProbeComponentHandleWrapper>(l);}

bool CReflectionProbeComponent::SaveIBLReflectionsToFile(const std::string &localPath)
{
	if(m_iblData == nullptr)
		return false;
	std::string path = "materials/" +localPath;
	FileManager::CreatePath(path.c_str());

	auto &imgPrefilter = m_iblData->prefilterMap->GetImage();
	auto &imgBrdf = m_iblData->brdfMap->GetImage();
	auto &imgIrradiance = m_iblData->irradianceMap->GetImage();

	ImageWriteInfo imgWriteInfo {};
	imgWriteInfo.inputFormat = ImageWriteInfo::InputFormat::R32G32B32A32_Float;
	imgWriteInfo.outputFormat = ImageWriteInfo::OutputFormat::BC6;
	auto fErrorHandler = [](const std::string &errMsg) {
		Con::cwar<<"WARNING: Unable to create IBL reflection files: "<<errMsg<<Con::endl;
	};
	if(c_game->SaveImage(*imgPrefilter,path +"/prefilter",imgWriteInfo) == false ||
		c_game->SaveImage(*imgPrefilter,path +"/irradiance",imgWriteInfo) == false ||
		c_game->SaveImage(*imgPrefilter,"materials/env/brdf",imgWriteInfo) == false
	)
		return false;

	auto *mat = client->CreateMaterial("ibl");
	if(mat == nullptr)
		return false;
	auto &dataBlock = mat->GetDataBlock();
	dataBlock->AddValue("texture","prefilter",localPath +"/prefilter");
	dataBlock->AddValue("texture","irradiance",localPath +"/irradiance");
	dataBlock->AddValue("texture","brdf","env/brdf");
	return mat->Save("env/generic/generic.wmi");
}
bool CReflectionProbeComponent::CaptureIBLReflectionsFromScene()
{
	// Not yet implemented
	return false;
}

bool CReflectionProbeComponent::GenerateIBLReflectionsFromEnvMap(const std::string &envMapFileName)
{
	auto *shaderEquiRectToCubemap = static_cast<pragma::ShaderEquirectangularToCubemap*>(c_engine->GetShader("equirectangular_to_cubemap").get());
	auto *shaderConvolute = static_cast<pragma::ShaderConvoluteCubemapLighting*>(c_engine->GetShader("convolute_cubemap_lighting").get());
	auto *shaderRoughness = static_cast<pragma::ShaderComputeIrradianceMapRoughness*>(c_engine->GetShader("compute_irradiance_map_roughness").get());
	auto *shaderBRDF = static_cast<pragma::ShaderBRDFConvolution*>(c_engine->GetShader("brdf_convolution").get());
	if(shaderEquiRectToCubemap == nullptr || shaderConvolute == nullptr || shaderRoughness == nullptr || shaderBRDF == nullptr)
		return false;
	auto cubemapTex = shaderEquiRectToCubemap->LoadEquirectangularImage(envMapFileName,512);
	if(cubemapTex == nullptr)
		return false;
	auto irradianceMap = shaderConvolute->ConvoluteCubemapLighting(*cubemapTex,32);
	auto prefilterMap = shaderRoughness->ComputeRoughness(*cubemapTex,128);

	TextureManager::LoadInfo loadInfo {};
	loadInfo.mipmapLoadMode = TextureMipmapMode::Load;
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	std::shared_ptr<prosper::Texture> brdfTex = nullptr;
	std::shared_ptr<void> texPtr = nullptr;
	// Load BRDF texture from disk, if it already exists
	if(static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(*c_engine,"materials/env/brdf.ktx",loadInfo,&texPtr) == true)
		brdfTex = std::static_pointer_cast<Texture>(texPtr)->texture;

	// Otherwise generate it
	if(brdfTex == nullptr)
		brdfTex = shaderBRDF->CreateBRDFConvolutionMap(512);

	if(irradianceMap == nullptr || prefilterMap == nullptr)
		return false;
	m_iblData = std::make_unique<rendering::IBLData>(irradianceMap,prefilterMap,brdfTex);
	InitializeDescriptorSet();
	return true;
}
bool CReflectionProbeComponent::LoadIBLReflectionsFromFile(const std::string &fileName)
{
	auto *mat = client->LoadMaterial(fileName,true,false);
	if(mat == nullptr)
		return false;
	auto *pPrefilter = mat->GetTextureInfo("prefilter");
	auto *pIrradiance = mat->GetTextureInfo("irradiance");
	auto *pBrdf = mat->GetTextureInfo("brdf");
	if(pPrefilter == nullptr || pIrradiance == nullptr || pBrdf == nullptr)
		return false;
	auto texPrefilter = std::static_pointer_cast<Texture>(pPrefilter->texture);
	auto texIrradiance = std::static_pointer_cast<Texture>(pIrradiance->texture);
	auto texBrdf = std::static_pointer_cast<Texture>(pBrdf->texture);
	if(
		texPrefilter == nullptr || texPrefilter->texture == nullptr ||
		texIrradiance == nullptr || texIrradiance->texture == nullptr ||
		texBrdf == nullptr || texBrdf->texture == nullptr
	)
		return false;
	m_iblData = std::make_unique<rendering::IBLData>(texIrradiance->texture,texPrefilter->texture,texBrdf->texture);
	InitializeDescriptorSet();
	return true;
}
void CReflectionProbeComponent::InitializeDescriptorSet()
{
	m_iblDsg = nullptr;
	if(m_iblData == nullptr)
		return;
	auto &dev = c_engine->GetDevice();
	m_iblDsg = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPBR::DESCRIPTOR_SET_PBR);
	auto &ds = *(*m_iblDsg)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(ds,*m_iblData->irradianceMap,umath::to_integral(pragma::ShaderPBR::PBRBinding::IrradianceMap));
	prosper::util::set_descriptor_set_binding_texture(ds,*m_iblData->prefilterMap,umath::to_integral(pragma::ShaderPBR::PBRBinding::PrefilterMap));
	prosper::util::set_descriptor_set_binding_texture(ds,*m_iblData->brdfMap,umath::to_integral(pragma::ShaderPBR::PBRBinding::BRDFMap));
}
Anvil::DescriptorSet *CReflectionProbeComponent::GetIBLDescriptorSet()
{
	return m_iblDsg ? (*m_iblDsg)->get_descriptor_set(0u) : nullptr;
}

const rendering::IBLData *CReflectionProbeComponent::GetIBLData() const {return m_iblData.get();}

////////

void CEnvReflectionProbe::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CReflectionProbeComponent>();
}
#pragma optimize("",on)
