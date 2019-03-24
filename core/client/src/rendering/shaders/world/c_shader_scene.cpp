#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderScene::DESCRIPTOR_SET_RENDER_SETTINGS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Debug
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Time
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // CSM Data
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderScene::DESCRIPTOR_SET_CAMERA) ShaderScene::DESCRIPTOR_SET_CAMERA = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Camera
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::GEOMETRY_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Render Settings
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::GEOMETRY_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // SSAO Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Light Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderScene::RENDER_PASS_FORMAT) ShaderScene::RENDER_PASS_FORMAT = Anvil::Format::R16G16B16A16_SFLOAT;
decltype(ShaderScene::RENDER_PASS_DEPTH_FORMAT) ShaderScene::RENDER_PASS_DEPTH_FORMAT = Anvil::Format::D32_SFLOAT;
decltype(ShaderScene::RENDER_PASS_SAMPLES) ShaderScene::RENDER_PASS_SAMPLES = Anvil::SampleCountFlagBits::_1_BIT;
void ShaderScene::SetRenderPassSampleCount(Anvil::SampleCountFlagBits samples) {RENDER_PASS_SAMPLES = samples;}
ShaderScene::ShaderScene(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Shader3DBase(context,identifier,vsShader,fsShader,gsShader)
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
bool ShaderScene::ShouldInitializePipeline(uint32_t pipelineIdx)
{
	if(static_cast<Pipeline>(pipelineIdx) != Pipeline::MultiSample)
		return true;
	return RENDER_PASS_SAMPLES != Anvil::SampleCountFlagBits::_1_BIT;
}
Anvil::SampleCountFlagBits ShaderScene::GetSampleCount(uint32_t pipelineIdx) const
{
	return (static_cast<Pipeline>(pipelineIdx) == Pipeline::MultiSample) ? RENDER_PASS_SAMPLES : Anvil::SampleCountFlagBits::_1_BIT;
}
void ShaderScene::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	auto sampleCount = GetSampleCount(pipelineIdx);
	prosper::util::RenderPassCreateInfo rpCreateInfo {{
		{
			RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::DONT_CARE,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
		},
		{ // Bloom Attachment
			RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::CLEAR,
			Anvil::AttachmentStoreOp::STORE,sampleCount,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
		},
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE /* depth values have already been written by prepass */,sampleCount,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	}};
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass{std::vector<std::size_t>{0ull,1ull},true});
	/*auto mainSubPassId = rpCreateInfo.subPasses.size() -1ull;

	// Particle sub-pass
	rpCreateInfo.subPasses.push_back(prosper::util::RenderPassCreateInfo::SubPass{std::vector<std::size_t>{0ull,1ull},true});
	auto particleSubPassId = rpCreateInfo.subPasses.size() -1ull;
	auto &subPass = rpCreateInfo.subPasses.back();
	subPass.dependencies.push_back({
		particleSubPassId,mainSubPassId,
		vk::PipelineStageFlagBits::eFragmentShader,vk::PipelineStageFlagBits::eFragmentShader,
		Anvil::AccessFlagBits::SHADER_WRITE_BIT | Anvil::AccessFlagBits::SHADER_READ_BIT,Anvil::AccessFlagBits::SHADER_WRITE_BIT
	});*/
	CreateCachedRenderPass<ShaderScene>(rpCreateInfo,outRenderPass,pipelineIdx);
}
bool ShaderScene::BindRenderSettings(Anvil::DescriptorSet &descSetRenderSettings)
{
	return RecordBindDescriptorSet(descSetRenderSettings,GetRenderSettingsDescriptorSetIndex());
}
bool ShaderScene::BindSceneCamera(const Scene &scene,bool bView)
{
	auto *descSet = (bView == true) ? scene.GetViewCameraDescriptorSet() : scene.GetCameraDescriptorSetGraphics();
	return RecordBindDescriptorSet(*descSet,GetCameraDescriptorSetIndex());
}

/////////////////////

decltype(ShaderSceneLit::DESCRIPTOR_SET_LIGHTS) ShaderSceneLit::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Light sources
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Visible light index buffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Shadow buffers
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderSceneLit::DESCRIPTOR_SET_CSM) ShaderSceneLit::DESCRIPTOR_SET_CSM = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Cascade Maps
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT,umath::to_integral(GameLimits::MaxCSMCascades)
		}
	}
};
decltype(ShaderSceneLit::DESCRIPTOR_SET_SHADOWS) ShaderSceneLit::DESCRIPTOR_SET_SHADOWS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Shadow Maps
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT,umath::to_integral(GameLimits::MaxActiveShadowMaps)
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Shadow Cube-Maps
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT,umath::to_integral(GameLimits::MaxActiveShadowCubeMaps)
		}
	}
};
ShaderSceneLit::ShaderSceneLit(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderScene(context,identifier,vsShader,fsShader,gsShader)
{}
bool ShaderSceneLit::BindLights(Anvil::DescriptorSet &descSetShadowMaps,Anvil::DescriptorSet &descSetLightSources)
{
	return RecordBindDescriptorSets({
		&descSetLightSources,&descSetShadowMaps,ShadowMap::GetDescriptorSet()
	},GetLightDescriptorSetIndex());
}
bool ShaderSceneLit::BindScene(const Scene &scene,bool bView)
{
	return BindSceneCamera(scene,bView) &&
		BindLights(*scene.GetCSMDescriptorSet(),*const_cast<Scene&>(scene).GetForwardPlusInstance().GetDescriptorSetGraphics());
}

/////////////////////

decltype(ShaderEntity::VERTEX_BINDING_BONE_WEIGHT) ShaderEntity::VERTEX_BINDING_BONE_WEIGHT = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {VERTEX_BINDING_BONE_WEIGHT,Anvil::Format::R32G32B32A32_SINT};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT = {VERTEX_BINDING_BONE_WEIGHT,Anvil::Format::R32G32B32A32_SFLOAT};

decltype(ShaderEntity::VERTEX_BINDING_VERTEX) ShaderEntity::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_POSITION) ShaderEntity::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_UV) ShaderEntity::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32_SFLOAT};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_NORMAL) ShaderEntity::VERTEX_ATTRIBUTE_NORMAL = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_TANGENT = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT) ShaderEntity::VERTEX_ATTRIBUTE_BI_TANGENT = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};

decltype(ShaderEntity::VERTEX_BINDING_LIGHTMAP) ShaderEntity::VERTEX_BINDING_LIGHTMAP = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV) ShaderEntity::VERTEX_ATTRIBUTE_LIGHTMAP_UV = {VERTEX_BINDING_LIGHTMAP,Anvil::Format::R32G32_SFLOAT};
decltype(ShaderEntity::DESCRIPTOR_SET_INSTANCE) ShaderEntity::DESCRIPTOR_SET_INSTANCE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Instance
			Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Bone Matrices
			Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,
			Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Vertex Animations
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Vertex Animation Frame Data
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::VERTEX_BIT
		}
	}
};
ShaderEntity::ShaderEntity(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ShaderSceneLit(context,identifier,vsShader,fsShader,gsShader)
{}

bool ShaderEntity::BindEntity(CBaseEntity &ent)
{
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return false;
	m_boundEntity = &ent;
	//if(pRenderComponent->GetLastRenderFrame() != c_engine->GetLastFrameId())
	//	Con::cwar<<"WARNING: Entity buffer data for entity "<<ent.GetClass()<<" ("<<ent.GetIndex()<<") hasn't been updated for this frame, but entity is used in rendering! This may cause rendering issues!"<<Con::endl;
	return BindInstanceDescriptorSet(*pRenderComponent->GetRenderDescriptorSet());
}

void ShaderEntity::EndDraw()
{
	ShaderSceneLit::EndDraw();
	m_boundEntity = nullptr;
}

bool ShaderEntity::BindInstanceDescriptorSet(Anvil::DescriptorSet &descSet)
{
	return RecordBindDescriptorSet(descSet,GetInstanceDescriptorSetIndex(),{0u,0u});
}

bool ShaderEntity::BindVertexAnimationOffset(uint32_t offset)
{
	uint32_t pushConstantOffset;
	GetVertexAnimationPushConstantInfo(pushConstantOffset);
	return RecordPushConstants(sizeof(offset),&offset,pushConstantOffset);
}

bool ShaderEntity::BindScene(const Scene &scene,bool bView)
{
	return ShaderSceneLit::BindScene(scene,bView) &&
		BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());
}

bool ShaderEntity::Draw(CModelSubMesh &mesh,const std::function<bool(CModelSubMesh&)> &fDraw,bool bUseVertexWeightBuffer)
{
	auto numTriangleVertices = mesh.GetTriangleVertexCount();
	if(numTriangleVertices > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}
	auto &vkMesh = mesh.GetVKMesh();
	auto &vertexBuffer = vkMesh->GetVertexBuffer();
	auto &indexBuffer = vkMesh->GetIndexBuffer();
	auto &vertexWeightBuffer = vkMesh->GetVertexWeightBuffer();
	if(vkMesh == nullptr || vertexBuffer == nullptr || indexBuffer == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to render mesh with invalid ";
		if(vkMesh == nullptr)
			Con::cwar<<"VKMesh";
		else if(vertexBuffer == nullptr)
			Con::cwar<<"Vertex Buffer";
		else
			Con::cwar<<"Index Buffer";
		Con::cwar<<"! Skipping..."<<Con::endl;
		return false;
	}

	auto &scene = static_cast<CGame*>(c_engine->GetClientState()->GetGameState())->GetRenderScene();
	auto &cam = scene->GetCamera();
	auto mvp = cam->GetProjectionMatrix() *cam->GetViewMatrix();
	std::vector<Anvil::Buffer*> vertexBuffers;
	vertexBuffers.reserve(2u);
	if(bUseVertexWeightBuffer == true)
		vertexBuffers.push_back((vertexWeightBuffer != nullptr) ? &vertexWeightBuffer->GetAnvilBuffer() : &c_engine->GetDummyBuffer()->GetAnvilBuffer());
	vertexBuffers.push_back(&vertexBuffer->GetAnvilBuffer());
	return RecordBindVertexBuffers(vertexBuffers) &&
		RecordBindIndexBuffer(indexBuffer->GetAnvilBuffer(),Anvil::IndexType::UINT16) &&
		fDraw(mesh);
}

bool ShaderEntity::Draw(CModelSubMesh &mesh,bool bUseVertexWeightBuffer)
{
	return Draw(mesh,[this](CModelSubMesh &mesh) {
		return RecordDrawIndexed(mesh.GetTriangleVertexCount());
	},bUseVertexWeightBuffer);
}

bool ShaderEntity::Draw(CModelSubMesh &mesh) {return Draw(mesh,true);}
