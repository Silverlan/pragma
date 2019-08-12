#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
decltype(ShaderPBR::DESCRIPTOR_SET_MATERIAL) ShaderPBR::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Albedo Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Normal Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Ambient occlusion Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Metallic Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Roughness Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderPBR::DESCRIPTOR_SET_PBR) ShaderPBR::DESCRIPTOR_SET_PBR = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Irradiance Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Prefilter Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // BRDF Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
static std::shared_ptr<prosper::DescriptorSetGroup> g_dsgTest = nullptr;
ShaderPBR::ShaderPBR(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase{context,identifier,"world/vs_textured","world/fs_pbr"}
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
bool ShaderPBR::BindMaterialParameters(CMaterial &mat)
{
	return ShaderTextured3DBase::BindMaterialParameters(mat);
}
prosper::Shader::DescriptorSetInfo &ShaderPBR::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
bool ShaderPBR::BindMaterial(CMaterial &mat)
{
	if(ShaderTextured3DBase::BindMaterial(mat) == false)
		return false;
	return RecordBindDescriptorSet(*(*g_dsgTest)->get_descriptor_set(0u),DESCRIPTOR_SET_PBR.setIndex);
}
void ShaderPBR::InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PBR);
}

#include <wgui/types/wirect.h>
#include "pragma/gui/wiframe.h"
#include "pragma/rendering/shaders/c_shader_equirectangular_to_cubemap.hpp"
#include "pragma/rendering/shaders/c_shader_convolute_cubemap_lighting.hpp"
#include "pragma/rendering/shaders/c_shader_compute_irradiance_map_roughness.hpp"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/shaders/c_shader_brdf_convolution.hpp"
std::shared_ptr<prosper::DescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::Shader::DescriptorSetInfo &descSetInfo)
{
	auto &dev = c_engine->GetDevice();
	auto *albedoMap = mat.GetTextureInfo("albedo_map");
	if(albedoMap == nullptr || albedoMap->texture == nullptr)
		return nullptr;

	auto albedoTexture = std::static_pointer_cast<Texture>(albedoMap->texture);
	if(albedoTexture->texture == nullptr)
		return nullptr;
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,descSetInfo);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto descSet = (*descSetGroup)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(*descSet,*albedoTexture->texture,umath::to_integral(MaterialBinding::AlbedoMap));

	auto *normalMap = mat.GetNormalMap();
	if(normalMap != nullptr && normalMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,umath::to_integral(MaterialBinding::NormalMap));
	}

	auto *pAoMap = mat.GetTextureInfo("ambient_occlusion_map");
	if(pAoMap == nullptr)
		pAoMap = mat.GetTextureInfo("ao_map");
	if(pAoMap && pAoMap->texture)
	{
		auto texture = std::static_pointer_cast<Texture>(pAoMap->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,umath::to_integral(MaterialBinding::AmbientOcclusionMap));
	}

	auto *pMetallicMap = mat.GetTextureInfo("metallic_map");
	if(pMetallicMap && pMetallicMap->texture)
	{
		auto texture = std::static_pointer_cast<Texture>(pMetallicMap->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,umath::to_integral(MaterialBinding::MetallicMap));
	}

	auto *pRoughnessMap = mat.GetTextureInfo("roughness_map");
	if(pRoughnessMap && pRoughnessMap->texture)
	{
		auto texture = std::static_pointer_cast<Texture>(pRoughnessMap->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,umath::to_integral(MaterialBinding::RoughnessMap));
	}

	// TODO: Roughness map?
	/*g_dsgTest = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_PBR);
	auto *pSkybox = mat.GetTextureInfo("skybox");
	if(pSkybox && pSkybox->texture)
	{
		auto texture = std::static_pointer_cast<Texture>(pSkybox->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*(*g_dsgTest)->get_descriptor_set(0),*texture->texture,0);
	}*/
	/*auto *pRoughnessMap = mat.GetTextureInfo("roughness_map");
	if(pRoughnessMap && pRoughnessMap->texture)
	{
		auto texture = std::static_pointer_cast<Texture>(pRoughnessMap->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,umath::to_integral(MaterialBinding::RoughnessMap));
	}*/

	auto *shader = static_cast<pragma::ShaderEquirectangularToCubemap*>(c_engine->GetShader("equirectangular_to_cubemap").get());
	static auto initialized = false;
	if(shader && initialized == false)
	{
		initialized = true;
		auto cubemapTex = shader->LoadEquirectangularImage("newport_loft.hdr",512);

		auto *shaderConvolute = static_cast<pragma::ShaderConvoluteCubemapLighting*>(c_engine->GetShader("convolute_cubemap_lighting").get());
		static auto irradianceMap = shaderConvolute->ConvoluteCubemapLighting(*cubemapTex,32);


		auto *shaderRoughness = static_cast<pragma::ShaderComputeIrradianceMapRoughness*>(c_engine->GetShader("compute_irradiance_map_roughness").get());
		static auto prefilterMap = shaderRoughness->ComputeRoughness(*cubemapTex,128);


		auto *shaderBRDF = static_cast<pragma::ShaderBRDFConvolution*>(c_engine->GetShader("brdf_convolution").get());
		static auto brdfTex = shaderBRDF->CreateBRDFConvolutionMap(512);
		{
		/*	auto *pFrame = WGUI::GetInstance().Create<WIFrame>();
			pFrame->SetSize(512,512 +24);
			pFrame->RequestFocus();
			pFrame->TrapFocus(true);
			auto *p = WGUI::GetInstance().Create<WITexturedRect>(pFrame);
			p->SetSize(512,512);
			p->SetPos(0,24);
			p->SetAnchor(0.f,0.f,1.f,1.f);
			p->SetTexture(*brdfTex);
		}*/

		auto *mat = c_engine->GetClientState()->GetMaterialManager().Load("skybox/dawn2.wmi");
		// Debug test: Change skybox texture to cubemap texture
		static auto oldTex = std::static_pointer_cast<Texture>(mat->GetTextureInfo("skybox")->texture)->texture;
		std::static_pointer_cast<Texture>(mat->GetTextureInfo("skybox")->texture)->texture = prefilterMap;
		static_cast<pragma::ShaderSkybox*>(c_engine->GetShader("skybox").get())->InitializeMaterialDescriptorSet(*static_cast<CMaterial*>(mat));
		//p->SetTexture(*newTex);
		std::cout<<"New Tex: "<<prefilterMap.get()<<std::endl;

		g_dsgTest = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_PBR);
		auto &ds = *(*g_dsgTest)->get_descriptor_set(0u);
		prosper::util::set_descriptor_set_binding_texture(ds,*irradianceMap,umath::to_integral(PBRBinding::IrradianceMap));
		prosper::util::set_descriptor_set_binding_texture(ds,*prefilterMap,umath::to_integral(PBRBinding::PrefilterMap));
		prosper::util::set_descriptor_set_binding_texture(ds,*brdfTex,umath::to_integral(PBRBinding::BRDFMap));
		(*g_dsgTest)->get_descriptor_set(0)->update();
		}
	}

	return descSetGroup;
}
std::shared_ptr<prosper::DescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}
#pragma optimize("",on)
