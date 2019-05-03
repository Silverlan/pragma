#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/water/c_shader_water.hpp"
#include "pragma/c_water_object.hpp"
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderWater::DESCRIPTOR_SET_MATERIAL) ShaderWater::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // DuDv Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Normal Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderWater::DESCRIPTOR_SET_WATER) ShaderWater::DESCRIPTOR_SET_WATER = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Reflection Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Refraction Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Refraction Depth
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Water settings
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Water fog
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderWater::ShaderWater(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"world/vs_water","world/fs_water")
{
	SetBaseShader<ShaderTextured3D>();
}

std::shared_ptr<prosper::DescriptorSetGroup> ShaderWater::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	auto &dev = c_engine->GetDevice();
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_MATERIAL);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto descSet = (*descSetGroup)->get_descriptor_set(0u);

	auto *dudvMap = mat.GetTextureInfo("dudvmap");
	if(dudvMap != nullptr && dudvMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(dudvMap->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,umath::to_integral(MaterialBinding::DuDvMap));
	}

	auto *normalMap = mat.GetNormalMap();
	if(normalMap != nullptr && normalMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		if(texture->texture != nullptr)
			prosper::util::set_descriptor_set_binding_texture(*descSet,*texture->texture,umath::to_integral(MaterialBinding::NormalMap));
	}
	return descSetGroup;
}
bool ShaderWater::BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx,RecordFlags recordFlags)
{
	if(ShaderTextured3DBase::BeginDraw(cmdBuffer,clipPlane,pipelineIdx,recordFlags) == false)
		return false;
	decltype(PushConstants::enableReflection) enableReflection = {m_bReflectionEnabled == true ? 1u : 0u};
	return RecordPushConstants(enableReflection,sizeof(ShaderTextured3DBase::PushConstants) +offsetof(PushConstants,enableReflection));
}
void ShaderWater::SetReflectionEnabled(bool b) {m_bReflectionEnabled = b;}
bool ShaderWater::UpdateBindFogDensity()
{
	auto fogIntensity = 1.f;
	if(m_boundEntity.IsValid() == false || m_boundScene.expired() == true)
		return true;
	auto whWaterComponent = m_boundEntity->GetComponent<CWaterComponent>();
	if(whWaterComponent.expired())
		return true;
	auto &scene = *m_boundScene.lock();
	auto &cam = *scene.GetCamera();
	auto &pos = cam.GetPos();
	if(whWaterComponent->IsPointBelowWaterPlane(pos) == true)
		fogIntensity = 0.f;
	return RecordPushConstants(fogIntensity,sizeof(ShaderTextured3DBase::PushConstants) +offsetof(PushConstants,waterFogIntensity));
}

void ShaderWater::EndDraw()
{
	m_boundEntity = {};
	m_boundScene = {};
}

bool ShaderWater::BindSceneCamera(const Scene &scene,bool bView)
{
	auto r = ShaderTextured3DBase::BindSceneCamera(scene,bView);
	if(r == false)
		return false;
	auto &cam = *scene.GetCamera();
	auto m = cam.GetProjectionMatrix() *cam.GetViewMatrix();
	m_boundScene = const_cast<Scene&>(scene).shared_from_this();
	return UpdateBindFogDensity() &&
		RecordPushConstants(m,sizeof(ShaderTextured3DBase::PushConstants) +offsetof(PushConstants,reflectionVp));
}

bool ShaderWater::BindEntity(CBaseEntity &ent)
{
	auto whWaterComponent = ent.GetComponent<CWaterComponent>();
	if(whWaterComponent.expired())
		return false;
	if(ShaderTextured3DBase::BindEntity(ent) == false)
		return false;
	auto *descSetEffect = whWaterComponent->GetEffectDescriptorSet();
	if(descSetEffect == nullptr || whWaterComponent->IsWaterSceneValid() == false)
		return false;
	auto &waterScene = whWaterComponent->GetWaterScene();
	auto &sceneReflection = waterScene.sceneReflection;
//	auto &sceneRefraction = waterScene.sceneRefraction;
	//auto &rtReflection = sceneReflection->GetRenderTarget(); // prosper TODO
	//auto &rtRefraction = sceneRefraction->GetRenderTarget();

	//rtReflection->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO

	//rtRefraction->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	//sceneRefraction->GetDepthTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	m_boundEntity = ent.GetHandle();
	return UpdateBindFogDensity() && RecordBindDescriptorSet(*descSetEffect,DESCRIPTOR_SET_WATER.setIndex);
}

void ShaderWater::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::NONE);
	pipelineInfo.toggle_depth_writes(true); // Water is not part of render pre-pass, but we need the depth for post-processing
}

void ShaderWater::InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderTextured3DBase::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_WATER);
}

void ShaderWater::InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderTextured3DBase::PushConstants) +sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);
}

prosper::Shader::DescriptorSetInfo &ShaderWater::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "c_shader_water.h"
#include "pragma/model/c_side.h"
#include "cmaterialmanager.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/uniformbinding.h"
#include <texturemanager/texturemanager.h>
#include "textureinfo.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/c_water_object.hpp"

using namespace Shader;

LINK_SHADER_TO_CLASS(Water,water);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

Water::Water()
	: Textured3D("Water","world/vs_water","world/fs_water")
{}

void Water::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Textured3D::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,TexturedBase3D::PUSH_CONSTANT_COUNT,(sizeof(Mat4) +sizeof(float)) /sizeof(float) +1});//sizeof(Mat4) /sizeof(float) +sizeof(float) *3});
	//pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,16,sizeof(Mat4) /sizeof(float)});//sizeof(Mat4) /sizeof(float) +sizeof(float) *3});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}, // Reflection Map
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}, // Refraction Map
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}, // Refraction Depth 
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}, // Water settings
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Water fog
	}));
}

void Water::InitializeMaterialBindings(std::vector<Vulkan::DescriptorSetLayout::Binding> &bindings)
{
	bindings = {
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}, // Dudv Map
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Normal Map
	};
}

void Water::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Textured3D::SetupPipeline(pipelineIdx,info);
	const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState)->cullMode = vk::CullModeFlagBits::eNone;

	auto &depthStencilState = const_cast<vk::PipelineDepthStencilStateCreateInfo&>(*info.pDepthStencilState);
	depthStencilState.setDepthWriteEnable(true); // Water is not part of render pre-pass, but we need the depth for post-processing
}

void Water::InitializeMaterial(Material *mat,bool bReload)
{
	auto &descSet = InitializeDescriptorSet(mat,bReload);

	auto *dudvMap = mat->GetTextureInfo("dudvmap");
	if(dudvMap != nullptr && dudvMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(dudvMap->texture);
		descSet->Update(static_cast<uint32_t>(Binding::DudvMap),texture->vkTexture);
	}

	auto *normalMap = mat->GetTextureInfo("normalmap");
	if(normalMap != nullptr && normalMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(normalMap->texture);
		descSet->Update(static_cast<uint32_t>(Binding::NormalMap),texture->vkTexture);
	}
}

void Water::SetReflectionEnabled(bool b) {m_bReflectionEnabled = b;}

bool Water::BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,bool &bWeighted)
{
	auto r = Textured3D::BindEntity(cmdBuffer,ent,bWeighted);
	if(r == false)
		return r;
	auto *objWater = dynamic_cast<CWaterObject*>(ent);
	if(objWater == nullptr)
		return false;
	auto &waterScene = objWater->GetWaterScene();
	auto &sceneReflection = waterScene.sceneReflection;
//	auto &sceneRefraction = waterScene.sceneRefraction;
	auto &rtReflection = sceneReflection->GetRenderTarget();
	//auto &rtRefraction = sceneRefraction->GetRenderTarget();

	rtReflection->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

	//rtRefraction->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	//sceneRefraction->GetDepthTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::WaterEffects),layout,objWater->GetEffectDescriptorSet());
	auto fogIntensity = 1.f;
	if(m_boundScene != nullptr)
	{
		auto &cam = m_boundScene->GetCamera();
		auto &pos = cam->GetPos();
		if(objWater->IsPointBelowWaterPlane(pos) == true)
			fogIntensity = 0.f;
	}
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,TexturedBase3D::PUSH_CONSTANT_COUNT,1,&fogIntensity);
	return r;
}

bool Water::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	auto r = Textured3D::BeginDraw(cmdBuffer,shaderPipeline);
	if(r == false)
		return r;
	auto &cam = *c_game->GetRenderCamera();
	auto m = cam.GetProjectionMatrix() *cam.GetViewMatrix();
	DataStream ds(sizeof(uint32_t) +sizeof(Mat4));
	ds<<((m_bReflectionEnabled == true) ? 1u : 0u)<<m;
	cmdBuffer->PushConstants(GetPipeline()->GetPipelineLayout(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT,TexturedBase3D::PUSH_CONSTANT_COUNT +1,sizeof(Mat4) /sizeof(float) +1,ds->GetData());
	return r;
}
#endif
