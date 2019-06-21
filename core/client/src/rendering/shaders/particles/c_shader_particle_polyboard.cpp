#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <buffers/prosper_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderParticlePolyboard::VERTEX_BINDING_VERTEX) ShaderParticlePolyboard::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderParticlePolyboard::VERTEX_ATTRIBUTE_VERTEX) ShaderParticlePolyboard::VERTEX_ATTRIBUTE_VERTEX = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};
decltype(ShaderParticlePolyboard::VERTEX_ATTRIBUTE_COLOR) ShaderParticlePolyboard::VERTEX_ATTRIBUTE_COLOR = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32A32_SFLOAT};
ShaderParticlePolyboard::ShaderParticlePolyboard(prosper::Context &context,const std::string &identifier)
	: ShaderParticle2DBase(context,identifier,"particles/beam/vs_particle_polyboard","particles/beam/fs_particle_polyboard","particles/beam/gs_particle_polyboard")
{
	SetBaseShader<ShaderParticle>();
}

void ShaderParticlePolyboard::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_VERTEX);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_COLOR);

	pipelineInfo.set_primitive_topology(Anvil::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(GeometryPushConstants),Anvil::ShaderStageFlagBits::GEOMETRY_BIT);
	AttachPushConstantRange(pipelineInfo,sizeof(GeometryPushConstants),sizeof(FragmentPushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);

	RegisterDefaultGfxPipelineDescriptorSetGroups(pipelineInfo);
}

bool ShaderParticlePolyboard::Draw(const rendering::RasterizationRenderer &renderer,const pragma::CParticleSystemComponent &ps,prosper::Buffer &vertexBuffer,prosper::Buffer &indexBuffer,uint32_t numIndices,float radius,float curvature)
{
	if(BindParticleMaterial(renderer,ps) == false)
		return false;
	auto &cam = *renderer.GetScene().GetCamera();
	if(RecordPushConstants(GeometryPushConstants{
				cam.GetPos(),radius,curvature
			}) == false ||
		RecordPushConstants(FragmentPushConstants{
			static_cast<int32_t>(GetRenderFlags(ps)),
			static_cast<float>(c_engine->GetWindowWidth()),
			static_cast<float>(c_engine->GetWindowHeight())
		},sizeof(GeometryPushConstants)) == false
		)
		return false;
	return RecordBindVertexBuffer(vertexBuffer.GetAnvilBuffer()) && RecordBindIndexBuffer(indexBuffer.GetAnvilBuffer()) && RecordDrawIndexed(numIndices);
}

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "c_shader_particle_polyboard.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_side.h"
#include "pragma/rendering/uniformbinding.h"
#include <textureinfo.h>
#include <cmaterial.h>

using namespace Shader;

LINK_SHADER_TO_CLASS(ParticlePolyboard,particlepolyboard);

extern DLLCLIENT CGame *c_game;

ParticlePolyboard::ParticlePolyboard(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: ParticleBase(identifier,vsShader,fsShader,gsShader)
{
	m_bEnableLighting = true;
}

ParticlePolyboard::ParticlePolyboard()
	: ParticlePolyboard("particlepolyboard","particles/beam/vs_particle_polyboard","particles/beam/fs_particle_polyboard","particles/beam/gs_particle_polyboard")
{}

void ParticlePolyboard::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	Base::SetupPipeline(pipelineIdx,info);
	const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState)->setTopology(Anvil::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY);
}

void ParticlePolyboard::InitializeMaterial(Material *mat,bool bReload)
{
	ParticleBase::InitializeMaterial(mat,bReload);
}

void ParticlePolyboard::InitializeShaderPipelines(const Vulkan::Context &context)
{
	ParticleBase::InitializeShaderPipelines(context);
}

void ParticlePolyboard::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	ParticleBase::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.clear();
	pushConstants.push_back({Anvil::ShaderStageFlagBits::GEOMETRY_BIT,0,5});
	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,5,3});
}
void ParticlePolyboard::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Base::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(Binding::Vertex),
		sizeof(Vector3) +sizeof(Vector4),
		Anvil::VertexInputRate::VERTEX
	});

	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Vertex),
		static_cast<uint32_t>(Binding::Vertex),
		Anvil::Format::R32G32B32_SFLOAT,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Color),
		static_cast<uint32_t>(Binding::Color),
		Anvil::Format::R32G32B32A32_SFLOAT,sizeof(Vector3)
	});
}

void ParticlePolyboard::GenerateParticleMapDescriptorSet(Vulkan::DescriptorSet &texture) {GenerateDescriptorSet(static_cast<uint32_t>(DescSet::ParticleMap),texture);}
void ParticlePolyboard::GenerateAnimationDescriptorSet(Vulkan::DescriptorSet &anim) {GenerateDescriptorSet(static_cast<uint32_t>(DescSet::Animation),anim);}

bool ParticlePolyboard::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	if(ParticleBase::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	/*auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = m_pipeline->GetPipelineLayout();

	// Bind static buffers
	auto &descTime = *m_pipeline->GetDescriptorBuffer(static_cast<uint32_t>(DescSet::Time));
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::Time),layout,descTime->GetDescriptorSet());*/
	return true;
}

void ParticlePolyboard::Draw(Camera &cam,CParticleSystem *particle,Bool,const Vulkan::Buffer &vertexBuffer,const Vulkan::Buffer &indexBuffer,uint32_t indexCount,float radius,float curvature)
{
	auto *mat = particle->GetMaterial();
	auto &descTexture = static_cast<CMaterial*>(mat)->GetDescriptorSet();
	if(!descTexture.IsValid())
		return;

	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();

	auto rot = particle->GetOrientation();
	//auto orType = particle->GetOrientationType();
	auto &v = cam.GetViewMatrix();
	auto vp = cam.GetProjectionMatrix() *v;
	auto w = context.GetWidth();
	auto h = context.GetHeight();
	//auto texIntensity = (bloom == true) ? particle->GetBloomScale() : 1.f;

	Vulkan::Std140LayoutBlockData gsPushConstants(5);
	gsPushConstants<<cam.GetPos();
	gsPushConstants<<radius<<curvature;

	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::GEOMETRY_BIT,static_cast<uint32_t>(gsPushConstants.GetCount()),gsPushConstants.GetData());

	auto renderFlags = GetRenderFlags(particle);
	Vulkan::Std140LayoutBlockData fsPushConstants(3);
	fsPushConstants<<static_cast<int32_t>(renderFlags);

	fsPushConstants<<static_cast<float>(w)<<static_cast<float>(h);
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,5,static_cast<uint32_t>(fsPushConstants.GetCount()),fsPushConstants.GetData());

	auto bAnimated = ((renderFlags &RenderFlags::Animated) != RenderFlags::None) ? true : false;
	if(bAnimated == true)
	{
		auto *descSet = particle->GetAnimationDescriptorSet();
		drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::Animation),layout,*descSet);
	}
	auto &scene = c_game->GetRenderScene();
	auto &tex = scene->GetRenderDepthBuffer();
	auto &descSetDepth = scene->GetRenderDepthDescriptorSet();
	tex->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::ParticleMap),layout,descTexture);
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::DepthMap),layout,descSetDepth);

	drawCmd->BindVertexBuffer(vertexBuffer);
	drawCmd->BindIndexBuffer(indexBuffer,vk::IndexType::eUint16);
	drawCmd->DrawIndexed(indexCount);
	/*auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = m_pipeline->GetPipelineLayout();

	auto *mat = particle->GetMaterial();
	auto &descTexture = static_cast<CMaterial*>(mat)->GetDescriptorSet();
	if(!descTexture.IsValid())
		return;

	auto &bufTime = *m_pipeline->GetDescriptorBuffer(umath::to_integral(DescSet::Time));
	auto &descSetTime = *bufTime->GetDescriptorSet(context.GetFrameSwapIndex());
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::Time),layout,descSetTime);

	auto bAnimated = particle->IsAnimated();
	Vulkan::Std140LayoutBlockData fsPushConstants(3);
	fsPushConstants<<bAnimated;
	if(bAnimated == true)
	{
		auto *descSet = particle->GetAnimationDescriptorSet();
		drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::Animation),layout,*descSet);
	}
	auto w = context.GetWidth();
	auto h = context.GetHeight();
	fsPushConstants<<static_cast<float>(w)<<static_cast<float>(h);
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,fsPushConstants.GetCount(),fsPushConstants.GetData());

	Vulkan::Std140LayoutBlockData gsPushConstants(21);
	gsPushConstants<<cam.GetProjectionMatrix() *cam.GetViewMatrix();
	gsPushConstants<<cam.GetPos();
	gsPushConstants<<radius<<curvature;

	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::GEOMETRY_BIT,gsPushConstants.GetCount(),gsPushConstants.GetData());

	auto &scene = c_game->GetRenderScene();
	auto &tex = scene->GetRenderDepthBuffer();
	auto &descSetDepth = scene->GetRenderDepthDescriptorSet();
	tex->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::ParticleMap),layout,descTexture);
	drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::DepthMap),layout,descSetDepth);

	drawCmd->BindVertexBuffer(vertexBuffer);
	drawCmd->BindIndexBuffer(indexBuffer,vk::IndexType::eUint16);
	drawCmd->DrawIndexed(indexCount);
	//drawCmd->DrawIndexed(0,indexCount,0,0,particle->GetParticleCount());*/
}
#endif