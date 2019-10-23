#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/math/intersection.h>
#include <pragma/physics/collisionmesh.h>
#include <sharedutils/util_file.h>
#include <image/prosper_image.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pr_dds.hpp>
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/rendering/shaders/util/c_shader_specular_to_roughness.hpp"
#include "pragma/rendering/shaders/util/c_shader_extract_diffuse_ambient_occlusion.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(util_pbr_converter,CUtilPBRConverter);

#pragma optimize("",off)
luabind::object CPBRConverterComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPBRConverterComponentHandleWrapper>(l);}

void CPBRConverterComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<LogicComponent>();
	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		PollEvents();
	});

	auto libGpl = client->InitializeLibrary("pr_gpl");
	if(libGpl == nullptr)
	{
		GetEntity().RemoveSafely();
		return;
	}
	auto *fCalcGeometryData = libGpl->FindSymbolAddress<void(*)(const std::vector<Vector3>&,const std::vector<uint16_t>&,std::vector<float>*,std::vector<Vector3>*,uint32_t)>("pr_gpl_calc_geometry_data");
	if(fCalcGeometryData == nullptr)
	{
		GetEntity().RemoveSafely();
		return;
	}
	m_fCalcGeometryData = fCalcGeometryData;
}

void CPBRConverterComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	while(m_workQueue.empty() == false)
	{
		auto &item = m_workQueue.front();
		item.job.Cancel();
		item.job.Wait();
		m_workQueue.pop();
	}
	PollEvents();

	if(m_cbOnModelLoaded.IsValid())
		m_cbOnModelLoaded.Remove();
	if(m_cbOnMaterialLoaded.IsValid())
		m_cbOnMaterialLoaded.Remove();
	for(auto &pair : m_onModelMaterialsLoadedCallbacks)
	{
		auto &cb = pair.second;
		if(cb.IsValid())
			cb.Remove();
	}
}

void CPBRConverterComponent::ConvertMaterialsToPBR(Model &mdl)
{
	for(auto hMat : mdl.GetMaterials())
	{
		if(hMat.IsValid() == false)
			continue;
		auto &mat = static_cast<CMaterial&>(*hMat.get());
		if(ShouldConvertMaterial(mat) == false)
			continue;
		ConvertToPBR(static_cast<CMaterial&>(*hMat.get()));
	}
}

void CPBRConverterComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

	m_cbOnModelLoaded = c_game->AddCallback("OnModelLoaded",FunctionCallback<void,std::reference_wrapper<std::shared_ptr<Model>>>::Create([this](std::reference_wrapper<std::shared_ptr<Model>> mdl) {
		auto pMdl = mdl.get();
		auto cb = mdl.get()->CallOnMaterialsLoaded([this,pMdl]() {
			UpdateMetalness(*pMdl);
			UpdateAmbientOcclusion(*pMdl);
			auto it = m_onModelMaterialsLoadedCallbacks.find(pMdl.get());
			if(it != m_onModelMaterialsLoadedCallbacks.end())
			{
				auto &cb = it->second;
				if(cb.IsValid())
					cb.Remove();
				m_onModelMaterialsLoadedCallbacks.erase(it);
			}
		});
		if(cb.IsValid())
			m_onModelMaterialsLoadedCallbacks.insert(std::make_pair(mdl.get().get(),cb));
	}));
	m_cbOnMaterialLoaded = client->AddCallback("OnMaterialLoaded",FunctionCallback<void,CMaterial*>::Create([this](CMaterial *mat) {
		if(ShouldConvertMaterial(*mat) == false)
			return;
		ConvertToPBR(*mat);
	}));

	for(auto &pair : client->GetMaterialManager().GetMaterials())
	{
		auto &hMat = pair.second;
		if(hMat.IsValid() == false || hMat.get()->IsLoaded() == false || ShouldConvertMaterial(static_cast<CMaterial&>(*hMat.get())) == false)
			continue;
		ConvertToPBR(static_cast<CMaterial&>(*hMat.get()));
	}

	auto &models = c_game->GetModels();
	for(auto &pair : models)
	{
		auto &mdl = pair.second;
		UpdateMetalness(*mdl);
		UpdateAmbientOcclusion(*mdl);
	}
}
bool CPBRConverterComponent::ShouldConvertMaterial(CMaterial &mat) const
{
	if(m_convertedMaterials.find(mat.GetName()) != m_convertedMaterials.end())
		return false;
	auto shader = mat.GetShaderIdentifier();
	ustring::to_lower(shader);
	return (shader == "textured" || shader == "texturedalphatransition") && shader != "pbr";
}

bool CPBRConverterComponent::IsPBR(CMaterial &mat) const
{
	auto shader = mat.GetShaderIdentifier();
	ustring::to_lower(shader);
	return shader == "pbr";
}

void CPBRConverterComponent::PollEvents()
{
	ProcessQueue();
}

bool CPBRConverterComponent::ConvertToPBR(CMaterial &matTraditional)
{
	Con::cout<<"Converting material '"<<matTraditional.GetName()<<"' to PBR..."<<Con::endl;
	m_convertedMaterials.insert(matTraditional.GetName());
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	auto &dev = c_engine->GetDevice();

	auto *matPbr = client->CreateMaterial("pbr");
	auto &dataPbr = matPbr->GetDataBlock();

	auto matName = matTraditional.GetName();
	ufile::remove_extension_from_filename(matName);

	// Albedo map
	// TODO: Extract ambient occlusion from diffuse map, if possible
	auto *diffuseMap = matTraditional.GetDiffuseMap();
	if(diffuseMap && diffuseMap->texture && std::static_pointer_cast<Texture>(diffuseMap->texture)->HasValidVkTexture())
		dataPbr->AddValue("texture",Material::ALBEDO_MAP_IDENTIFIER,diffuseMap->name);
	//

	// Roughness map
	auto *specularMap = matTraditional.GetSpecularMap();
	if(specularMap && specularMap->texture && std::static_pointer_cast<Texture>(specularMap->texture)->HasValidVkTexture())
	{
		auto roughnessMap = ConvertSpecularMapToRoughness(*std::static_pointer_cast<Texture>(specularMap->texture)->GetVkTexture());
		if(roughnessMap)
		{
			auto roughnessName = matName +"_roughness";
			ImageWriteInfo imgWriteInfo {};
			imgWriteInfo.alphaMode = ImageWriteInfo::AlphaMode::None;
			imgWriteInfo.containerFormat = ImageWriteInfo::ContainerFormat::DDS;
			imgWriteInfo.flags = ImageWriteInfo::Flags::GenerateMipmaps;
			imgWriteInfo.inputFormat = ImageWriteInfo::InputFormat::R8G8B8A8_UInt;
			imgWriteInfo.outputFormat = ImageWriteInfo::OutputFormat::GradientMap;
			c_game->SaveImage(*roughnessMap->GetImage(),"addons/converted/materials/" +roughnessName,imgWriteInfo);
			dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,roughnessName);
		}
	}
	else
		dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,"pbr/rough"); // Generic roughness map with 100% roughness
	//

	// Normal map
	auto bGenerateNormalMap = false;
	auto *normalMap = matTraditional.GetNormalMap();
	if(normalMap && normalMap->texture && std::static_pointer_cast<Texture>(normalMap->texture)->HasValidVkTexture())
		dataPbr->AddValue("texture",Material::NORMAL_MAP_IDENTIFIER,normalMap->name);
	//

	// Ambient occlusion map
	auto bGenerateAOMap = false;
	auto *aoMap = matTraditional.GetAmbientOcclusionMap();
	if(aoMap && aoMap->texture && std::static_pointer_cast<Texture>(aoMap->texture)->GetVkTexture())
		dataPbr->AddValue("texture",Material::AO_MAP_IDENTIFIER,aoMap->name);
	//

	// Metalness map
	auto valSurfMat = dataPbr->GetDataValue("surfacematerial");
	if(valSurfMat)
	{
		// Attempt to determine whether this is a metal material or not
		if(IsSurfaceMaterialMetal(valSurfMat->GetString()))
			dataPbr->AddValue("texture",Material::METALNESS_MAP_IDENTIFIER,"pbr/metal"); // 100% metalness
	}
	// Note: If no surface material could be found in the material,
	// the model's surface material will be checked as well in 'GenerateGeometryBasedTextures'.
	//

	// Emission map
	auto *glowMap = matTraditional.GetGlowMap();
	if(glowMap && glowMap->texture && std::static_pointer_cast<Texture>(glowMap->texture)->GetVkTexture())
	{
		dataPbr->AddValue("texture",Material::EMISSION_MAP_IDENTIFIER,glowMap->name);
		dataPbr->AddValue("bool","glow_alpha_only","1");
		dataPbr->AddValue("float","glow_scale","1.0");
		dataPbr->AddValue("int","glow_blend_diffuse_mode","1");
		dataPbr->AddValue("float","glow_blend_diffuse_scale","3.0");
	}
	//

	matPbr->UpdateTextures();
	// Overwrite old material with new PBR settings
	if(matPbr->Save(matTraditional.GetName(),"addons/converted/"))
		client->LoadMaterial(matName,true,true); // Reload material immediately
	Con::cout<<"Conversion complete!"<<Con::endl;
	return true;
}
std::shared_ptr<prosper::Texture> CPBRConverterComponent::ConvertSpecularMapToRoughness(prosper::Texture &specularMap)
{
	auto *shaderSpecularToRoughness = static_cast<pragma::ShaderSpecularToRoughness*>(c_engine->GetShader("specular_to_roughness").get());
	if(shaderSpecularToRoughness == nullptr)
		return nullptr;
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	auto &dev = c_engine->GetDevice();
	// Specular descriptor set
	auto dsgSpecular = prosper::util::create_descriptor_set_group(dev,pragma::ShaderSpecularToRoughness::DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*dsgSpecular->GetDescriptorSet(),specularMap,0u);

	// Initialize roughness image
	prosper::util::ImageCreateInfo createInfoRoughness {};
	specularMap.GetImage()->GetCreateInfo(createInfoRoughness);
	createInfoRoughness.format = Anvil::Format::R8G8B8A8_UNORM;
	createInfoRoughness.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	createInfoRoughness.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	auto roughnessMap = prosper::util::create_image(dev,createInfoRoughness);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto roughnessTex = prosper::util::create_texture(dev,{},roughnessMap,&imgViewCreateInfo,&samplerCreateInfo);
	auto roughnessRt = prosper::util::create_render_target(dev,{roughnessTex},shaderSpecularToRoughness->GetRenderPass());

	// Specular to roughness
	if(prosper::util::record_begin_render_pass(**setupCmd,*roughnessRt) == true)
	{
		if(shaderSpecularToRoughness->BeginDraw(setupCmd) == true)
		{
			shaderSpecularToRoughness->Draw(*(*dsgSpecular)->get_descriptor_set(0u));
			shaderSpecularToRoughness->EndDraw();
		}
		prosper::util::record_end_render_pass(**setupCmd);
	}
	c_engine->FlushSetupCommandBuffer();
	return roughnessTex;
}

////////

void CUtilPBRConverter::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPBRConverterComponent>();
}
#pragma optimize("",on)
