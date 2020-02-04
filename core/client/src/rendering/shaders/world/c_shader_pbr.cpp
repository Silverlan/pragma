#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "cmaterialmanager.h"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <texture_type.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
decltype(ShaderPBR::DESCRIPTOR_SET_MATERIAL) ShaderPBR::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Material settings
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::GEOMETRY_BIT
		},
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
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Emission Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Parallax Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Wrinkle Stretch Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Wrinkle Compress Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Exponent Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Specular Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
static_assert(umath::to_integral(ShaderPBR::MaterialBinding::Count) == 12,"Number of bindings in material descriptor set does not match MaterialBinding enum count!");

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
ShaderPBR::ShaderPBR(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase{context,identifier,"world/vs_textured","world/fs_pbr"}
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
bool ShaderPBR::BindMaterialParameters(CMaterial &mat)
{
	umath::set_flag(m_extRenderFlags,RenderFlags::TranslucencyEnabled,mat.IsTranslucent());
	return ShaderTextured3DBase::BindMaterialParameters(mat);
}
prosper::Shader::DescriptorSetInfo &ShaderPBR::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
void ShaderPBR::SetForceNonIBLMode(bool b) {m_bNonIBLMode = b;}
bool ShaderPBR::BeginDraw(
	const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx,
	RecordFlags recordFlags
)
{
	m_extRenderFlags = RenderFlags::None;
	return ShaderTextured3DBase::BeginDraw(cmdBuffer,clipPlane,pipelineIdx,recordFlags);
}
bool ShaderPBR::BindSceneCamera(const rendering::RasterizationRenderer &renderer,bool bView)
{
	if(ShaderTextured3DBase::BindSceneCamera(renderer,bView) == false)
		return false;
	auto &scene = renderer.GetScene();
	auto hCam = scene.GetActiveCamera();
	if(hCam.expired())
		return false;
	if(m_bNonIBLMode == false)
	{
		auto *ds = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(hCam->GetEntity().GetPosition());
		if(ds)
			return RecordBindDescriptorSet(*ds,DESCRIPTOR_SET_PBR.setIndex);
	}
	// No reflection probe and therefore no IBL available. Fallback to non-IBL rendering.
	m_extRenderFlags |= RenderFlags::NoIBL;
	return true;
}
void ShaderPBR::UpdateRenderFlags(CModelSubMesh &mesh,RenderFlags &inOutFlags)
{
	ShaderTextured3DBase::UpdateRenderFlags(mesh,inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderPBR::InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PBR);
}

static bool bind_texture(Material &mat,prosper::DescriptorSet &ds,TextureInfo *texInfo,uint32_t bindingIndex,Texture *optDefaultTex=nullptr)
{
	auto &matManager = static_cast<CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	loadInfo.mipmapLoadMode = TextureMipmapMode::Load;

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture)
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
	else if(optDefaultTex == nullptr)
		return false;
	else
		tex = optDefaultTex->shared_from_this();
	if(tex && tex->HasValidVkTexture())
		prosper::util::set_descriptor_set_binding_texture(ds,*tex->GetVkTexture(),bindingIndex);
	return true;
}

static TextureManager::LoadInfo get_texture_load_info()
{
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	loadInfo.mipmapLoadMode = TextureMipmapMode::Load;
	return loadInfo;
}

static bool bind_default_texture(prosper::DescriptorSet &ds,const std::string &defaultTexName,uint32_t bindingIndex)
{
	auto &matManager = static_cast<CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	std::shared_ptr<void> ptrTex = nullptr;
	if(texManager.Load(*c_engine,defaultTexName,get_texture_load_info(),&ptrTex) == false)
		return false;
	auto tex = std::static_pointer_cast<Texture>(ptrTex);

	if(tex && tex->HasValidVkTexture())
		prosper::util::set_descriptor_set_binding_texture(ds,*tex->GetVkTexture(),bindingIndex);
	return true;
}

static bool bind_texture(Material &mat,prosper::DescriptorSet &ds,TextureInfo *texInfo,uint32_t bindingIndex,const std::string &defaultTexName)
{
	auto &matManager = static_cast<CMaterialManager&>(client->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<Texture> tex = nullptr;
	if(texInfo && texInfo->texture)
	{
		tex = std::static_pointer_cast<Texture>(texInfo->texture);
		if(tex->HasValidVkTexture())
		{
			prosper::util::set_descriptor_set_binding_texture(ds,*tex->GetVkTexture(),bindingIndex);
			return true;
		}
	}
	else if(defaultTexName.empty())
		return false;
	return bind_default_texture(ds,defaultTexName,bindingIndex);
}

std::shared_ptr<prosper::DescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::Shader::DescriptorSetInfo &descSetInfo)
{
	auto &dev = c_engine->GetDevice();
	auto *albedoMap = mat.GetDiffuseMap();
	if(albedoMap == nullptr || albedoMap->texture == nullptr)
		return nullptr;

	auto albedoTexture = std::static_pointer_cast<Texture>(albedoMap->texture);
	if(albedoTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,descSetInfo);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_texture(descSet,*albedoTexture->GetVkTexture(),umath::to_integral(MaterialBinding::AlbedoMap));
	auto matData = InitializeMaterialBuffer(descSet,mat);
	if(matData.has_value() == false)
		return nullptr;

	if(bind_texture(mat,descSet,mat.GetNormalMap(),umath::to_integral(MaterialBinding::NormalMap),"black") == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetAmbientOcclusionMap(),umath::to_integral(MaterialBinding::AmbientOcclusionMap),"white") == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetMetalnessMap(),umath::to_integral(MaterialBinding::MetallicMap)) == false)
	{
		// No metalness map, but might have metalness factor
		float factor;
		if(mat.GetDataBlock()->GetFloat("metalness_factor",&factor))
		{
			// Let metalness factor decide metalness by using white metalness map
			if(bind_default_texture(descSet,"white",umath::to_integral(MaterialBinding::MetallicMap)) == false)
				return false;
		}
		else
		{
			// Assume no metalness
			if(bind_texture(mat,descSet,mat.GetMetalnessMap(),umath::to_integral(MaterialBinding::MetallicMap),"black") == false)
				return false;
		}
	}

	if(bind_texture(mat,descSet,mat.GetRoughnessMap(),umath::to_integral(MaterialBinding::RoughnessMap)) == false)
	{
		// No roughness map; Neutralize it by using a white texture
		bind_default_texture(descSet,"white",umath::to_integral(MaterialBinding::RoughnessMap));
		if(bind_texture(mat,descSet,mat.GetTextureInfo("specular_map"),umath::to_integral(MaterialBinding::SpecularMap)) == false)
		{
			// No specular map available either, neutralize it as well (specular is inverted, so we have to use black to neutralize)
			if(bind_default_texture(descSet,"black",umath::to_integral(MaterialBinding::SpecularMap)) == false)
				return false;

			float factor;
			if(mat.GetDataBlock()->GetFloat("roughness_factor",&factor) == false && mat.GetDataBlock()->GetFloat("specular_factor",&factor) == false)
			{
				// Use half roughness map as default
				if(bind_default_texture(descSet,"pbr/rough_half",umath::to_integral(MaterialBinding::RoughnessMap)) == false)
					return false;
			}
		}
	}

	bind_texture(mat,descSet,mat.GetGlowMap(),umath::to_integral(MaterialBinding::EmissionMap));

	if(bind_texture(mat,descSet,mat.GetParallaxMap(),umath::to_integral(MaterialBinding::ParallaxMap),"black") == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetTextureInfo("wrinkle_stretch_map"),umath::to_integral(MaterialBinding::WrinkleStretchMap),albedoTexture.get()) == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetTextureInfo("wrinke_compress_map"),umath::to_integral(MaterialBinding::WrinkleCompressMap),albedoTexture.get()) == false)
		return false;

	if(bind_texture(mat,descSet,mat.GetTextureInfo("exponent_map"),umath::to_integral(MaterialBinding::ExponentMap),"white") == false)
		return false;

	// TODO: FIXME: It would probably be a good idea to update the descriptor set lazily (i.e. not update it here), but
	// that seems to cause crashes in some cases
	if(descSet->update() == false)
		return false;
	return descSetGroup;
}
std::shared_ptr<prosper::DescriptorSetGroup> ShaderPBR::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}
#pragma optimize("",on)
