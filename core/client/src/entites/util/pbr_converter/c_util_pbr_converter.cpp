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
#include <sharedutils/util_library.hpp>
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/rendering/shaders/util/c_shader_specular_to_roughness.hpp"
#include "pragma/rendering/shaders/util/c_shader_extract_diffuse_ambient_occlusion.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <cmaterialmanager.h>

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
	for(auto &pair : m_scheduledModelUpdates)
	{
		if(pair.second.cbOnMaterialsLoaded.IsValid())
			pair.second.cbOnMaterialsLoaded.Remove();
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

void CPBRConverterComponent::GenerateAmbientOcclusionMaps(Model &mdl,uint32_t w,uint32_t h,uint32_t samples,bool rebuild)
{
	ScheduleModelUpdate(mdl,false,AmbientOcclusionInfo{w,h,samples,rebuild});
}

void CPBRConverterComponent::UpdateModel(Model &mdl,ModelUpdateInfo &updateInfo)
{
	if(updateInfo.updateMetalness)
		UpdateMetalness(mdl);
	if(updateInfo.updateAmbientOcclusion.has_value())
		UpdateAmbientOcclusion(mdl,*updateInfo.updateAmbientOcclusion);
	if(updateInfo.cbOnMaterialsLoaded.IsValid())
		updateInfo.cbOnMaterialsLoaded.Remove();
	auto it = m_scheduledModelUpdates.find(&mdl);
	if(it != m_scheduledModelUpdates.end())
		m_scheduledModelUpdates.erase(it);
}

void CPBRConverterComponent::ScheduleModelUpdate(Model &mdl,bool updateMetalness,std::optional<AmbientOcclusionInfo> updateAOInfo)
{
	auto itUpdateInfo = m_scheduledModelUpdates.find(&mdl);
	if(itUpdateInfo == m_scheduledModelUpdates.end())
		itUpdateInfo = m_scheduledModelUpdates.insert(std::make_pair(&mdl,ModelUpdateInfo{})).first;
	auto &updateInfo = itUpdateInfo->second;
	if(updateMetalness)
		updateInfo.updateMetalness = true;
	if(updateAOInfo.has_value())
		updateInfo.updateAmbientOcclusion = *updateAOInfo;
	auto cb = mdl.CallOnMaterialsLoaded([this,&mdl,&updateInfo]() {
		UpdateModel(mdl,updateInfo);
	});
	if(cb.IsValid())
		updateInfo.cbOnMaterialsLoaded = cb;
}

void CPBRConverterComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

	m_cbOnModelLoaded = c_game->AddCallback("OnModelLoaded",FunctionCallback<void,std::reference_wrapper<std::shared_ptr<Model>>>::Create([this](std::reference_wrapper<std::shared_ptr<Model>> mdl) {
		ScheduleModelUpdate(*mdl.get(),true);
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
	return (shader == "textured" || shader == "texturedalphatransition") && shader != "pbr" && shader != "pbr_blend";
}

bool CPBRConverterComponent::IsPBR(CMaterial &mat) const
{
	auto shader = mat.GetShaderIdentifier();
	ustring::to_lower(shader);
	return shader == "pbr" || shader == "pbr_blend";
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
	auto dataBlock = matTraditional.GetDataBlock();

	auto *matPbr = client->CreateMaterial(matTraditional.GetTextureInfo(Material::ALBEDO_MAP2_IDENTIFIER) ? "pbr_blend" : "pbr");
	auto &dataPbr = matPbr->GetDataBlock();

	auto matName = matTraditional.GetName();
	ufile::remove_extension_from_filename(matName);

	auto fAddGenericTexture = [&matTraditional,&dataPbr](const std::string &identifier,TextureInfo *texInfo) -> bool {
		if(texInfo && texInfo->texture && std::static_pointer_cast<Texture>(texInfo->texture)->HasValidVkTexture())
		{
			dataPbr->AddValue("texture",identifier,texInfo->name);
			return true;
		}
		return false;
	};
	auto fAddGenericTextureByIdentifier = [&matTraditional,&dataPbr](const std::string &name) -> bool {
		auto *map = matTraditional.GetTextureInfo(name);
		if(map && map->texture && std::static_pointer_cast<Texture>(map->texture)->HasValidVkTexture())
		{
			dataPbr->AddValue("texture",name,map->name);
			return true;
		}
		return false;
	};

	// TODO: Extract ambient occlusion from diffuse map, if possible
	fAddGenericTexture(Material::ALBEDO_MAP_IDENTIFIER,matTraditional.GetDiffuseMap()); // Albedo map
	fAddGenericTexture(Material::ALBEDO_MAP2_IDENTIFIER,matTraditional.GetTextureInfo(Material::ALBEDO_MAP2_IDENTIFIER)); // Albedo blend map
	fAddGenericTexture(Material::ALBEDO_MAP3_IDENTIFIER,matTraditional.GetTextureInfo(Material::ALBEDO_MAP3_IDENTIFIER)); // Albedo blend map 2
	fAddGenericTexture(Material::NORMAL_MAP_IDENTIFIER,matTraditional.GetNormalMap()); // Normal map
	fAddGenericTexture(Material::PARALLAX_MAP_IDENTIFIER,matTraditional.GetParallaxMap()); // Parallax map
	fAddGenericTexture(Material::AO_MAP_IDENTIFIER,matTraditional.GetAmbientOcclusionMap()); // Ambient occlusion map
	auto hasSpecularMap = fAddGenericTexture(Material::SPECULAR_MAP_IDENTIFIER,matTraditional.GetSpecularMap()); // Specular/Glossiness map
	fAddGenericTextureByIdentifier(Material::WRINKLE_STRETCH_MAP_IDENTIFIER); // Wrinkle stretch map
	fAddGenericTextureByIdentifier(Material::WRINKLE_COMPRESS_MAP_IDENTIFIER); // Wrinkle compress map
	fAddGenericTextureByIdentifier(Material::EXPONENT_MAP_IDENTIFIER); // Exponent map

	auto valSurfMat = dataBlock->GetDataValue("surfacematerial");
	auto surfMatName = valSurfMat ? valSurfMat->GetString() : "";
	auto *surfMat = valSurfMat ? c_game->GetSurfaceMaterial(surfMatName) : nullptr;

	if(surfMatName.empty() == false)
		dataPbr->AddValue("string","surfacematerial",surfMatName);

	auto &pbrInfo = surfMat->GetPBRInfo();
	if(surfMat && pbrInfo.subsurfaceMultiplier != 0.f)
	{
		dataPbr->AddValue("float","subsurface_multiplier",std::to_string(pbrInfo.subsurfaceMultiplier));
		dataPbr->AddValue("color","subsurface_color",pbrInfo.subsurfaceColor.ToString());
		dataPbr->AddValue("int","subsurface_method",std::to_string(umath::to_integral(pbrInfo.subsurfaceMethod)));
		dataPbr->AddValue("vector","subsurface_radius",std::to_string(pbrInfo.subsurfaceRadius.x) +" " +std::to_string(pbrInfo.subsurfaceRadius.y) +" " +std::to_string(pbrInfo.subsurfaceRadius.z));
	}

	// Roughness map
	// Obsolete, since Pragma now supports specular maps for PBR directly
	/*auto *specularMap = matTraditional.GetSpecularMap();
	if(
		specularMap && specularMap->texture && std::static_pointer_cast<Texture>(specularMap->texture)->HasValidVkTexture() &&
		std::static_pointer_cast<Texture>(specularMap->texture)->IsError() == false
	)
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
	else*/
	if(hasSpecularMap == false)
	{
		// Attempt to determine roughness by surface material
		if(surfMat)
		{
			auto &pbrInfo = surfMat->GetPBRInfo();
			if(pbrInfo.roughness == 0.f)
				dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,"pbr/smooth");
			else if(pbrInfo.roughness == 0.5f)
				dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,"pbr/rough_half");
			else if(pbrInfo.roughness == 1.f)
				dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,"pbr/rough");
			else
			{
				dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,"pbr/rough");
				dataPbr->AddValue("float","roughness_factor",std::to_string(pbrInfo.roughness));
			}
		}
		else
			dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,"pbr/rough_half"); // Generic roughness map with 50% roughness
	}
	//

	// Metalness map
	if(surfMat)
	{
		auto &pbrInfo = surfMat->GetPBRInfo();
		if(pbrInfo.metalness > 0.f)
		{
			if(pbrInfo.metalness == 1.f)
				dataPbr->AddValue("texture",Material::METALNESS_MAP_IDENTIFIER,"pbr/metal");
			else
			{
				dataPbr->AddValue("texture",Material::METALNESS_MAP_IDENTIFIER,"pbr/metal");
				dataPbr->AddValue("float","metalness_factor",std::to_string(pbrInfo.metalness));
			}
		}
	}

	// Note: If no surface material could be found in the material,
	// the model's surface material will be checked as well in 'GenerateGeometryBasedTextures'.
	//

	auto fCopyValue = [&dataPbr,&matTraditional](const std::string &name) -> bool {
		auto dataBlock = matTraditional.GetDataBlock();
		auto value = dataBlock ? dataBlock->GetDataValue(name) : nullptr;
		if(value == nullptr)
			return false;
		dataPbr->AddValue(value->GetTypeString(),name,value->GetString());
		return true;
	};

	// Emission map
	auto *glowMap = matTraditional.GetGlowMap();
	if(glowMap && glowMap->texture && std::static_pointer_cast<Texture>(glowMap->texture)->GetVkTexture())
	{
		dataPbr->AddValue("texture",Material::EMISSION_MAP_IDENTIFIER,glowMap->name);
		if(fCopyValue("glow_alpha_only") == false)
			dataPbr->AddValue("bool","glow_alpha_only","1");
		if(fCopyValue("glow_scale") == false)
			dataPbr->AddValue("float","glow_scale","1.0");
		if(fCopyValue("glow_blend_diffuse_mode") == false)
			dataPbr->AddValue("int","glow_blend_diffuse_mode","1");
		if(fCopyValue("glow_blend_diffuse_scale") == false)
			dataPbr->AddValue("float","glow_blend_diffuse_scale","3.0");
	}
	//

	fCopyValue("black_to_alpha");
	fCopyValue("phong_normal_alpha");
	fCopyValue("phong_intensity");
	fCopyValue("phong_shininess");
	fCopyValue("parallax_height_scale");
	fCopyValue("translucent");

	fCopyValue("metalness_factor");
	fCopyValue("roughness_factor");
	fCopyValue("specular_factor");
	fCopyValue("emission_factor");

	matPbr->UpdateTextures();
	// Overwrite old material with new PBR settings
	if(matPbr->Save(matTraditional.GetName(),"addons/converted/"))
		client->LoadMaterial(matName,true,true); // Reload material immediately
	static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().ClearUnused();
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
