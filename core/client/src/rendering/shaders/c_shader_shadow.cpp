#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include <prosper_util.hpp>
#include <pragma/model/vertex.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

static auto SHADOW_DEPTH_BIAS_CONSTANT = 1.25f;
static auto SHADOW_DEPTH_BIAS_SLOPE = 1.75f;

decltype(ShaderShadow::RENDER_PASS_DEPTH_FORMAT) ShaderShadow::RENDER_PASS_DEPTH_FORMAT = Anvil::Format::D32_SFLOAT;
decltype(ShaderShadow::VERTEX_BINDING_BONE_WEIGHT) ShaderShadow::VERTEX_BINDING_BONE_WEIGHT = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderEntity::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderShadow::VERTEX_BINDING_VERTEX) ShaderShadow::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX,sizeof(Vertex)};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_POSITION) ShaderShadow::VERTEX_ATTRIBUTE_POSITION = {ShaderEntity::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};

decltype(ShaderShadow::DESCRIPTOR_SET_INSTANCE) ShaderShadow::DESCRIPTOR_SET_INSTANCE = {&ShaderEntity::DESCRIPTOR_SET_INSTANCE};
ShaderShadow::ShaderShadow(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderEntity(context,identifier,vsShader,fsShader)
{}

ShaderShadow::ShaderShadow(prosper::Context &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow","shadow/fs_shadow")
{}
bool ShaderShadow::BindDepthMatrix(const Mat4 &depthMVP)
{
	return RecordPushConstants(depthMVP);
}
bool ShaderShadow::BindMaterial(CMaterial &mat)
{
	/*auto &descTexture = mat.GetDescriptorSetGroup(*this);
	if(descTexture == nullptr)
		return false;
	auto *data = mat.GetDataBlock();
	auto alphaDiscardThreshold = pragma::DefaultAlphaDiscardThreshold;
	if(data != nullptr)
		data->GetFloat("alpha_discard_threshold",&alphaDiscardThreshold);
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,1,&alphaDiscardThreshold);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DiffuseMap),layout,descTexture);
	return true;*/
	return false; // prosper TODO
}

bool ShaderShadow::BindEntity(CBaseEntity &ent,const Mat4 &depthMVP)
{
	if(ShaderEntity::BindEntity(ent) == false)
		return false;
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return false;
	auto entMvp = depthMVP *pRenderComponent->GetTransformationMatrix();
	return BindDepthMatrix(entMvp);
}
bool ShaderShadow::BindLight(CLightComponent &light)
{
	auto &ent = light.GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pRadiusComponent = ent.GetComponent<CRadiusComponent>();
	auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto lightPos = Vector4{pos.x,pos.y,pos.z,static_cast<float>(pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f)};
	return RecordPushConstants(lightPos,offsetof(PushConstants,lightPos));
}
void ShaderShadow::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadow>({{{
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::CLEAR,
			Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
		}
	}}},outRenderPass,pipelineIdx);
}
uint32_t ShaderShadow::GetRenderSettingsDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetCameraDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetInstanceDescriptorSetIndex() const{return DESCRIPTOR_SET_INSTANCE.setIndex;}
void ShaderShadow::GetVertexAnimationPushConstantInfo(uint32_t &offset) const {}
void ShaderShadow::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo,true);
	pipelineInfo.toggle_depth_writes(true);
	pipelineInfo.toggle_depth_test(true,Anvil::CompareOp::LESS_OR_EQUAL);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,Anvil::CullModeFlagBits::NONE);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::FRAGMENT_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);

	pipelineInfo.toggle_depth_bias(true,SHADOW_DEPTH_BIAS_CONSTANT,0.f,SHADOW_DEPTH_BIAS_SLOPE);
}

//////////////////

ShaderShadowSpot::ShaderShadowSpot(prosper::Context &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow","shadow/fs_shadow_spot")
{
	SetBaseShader<ShaderShadow>();
}

//////////////////

ShaderShadowCSM::ShaderShadowCSM(prosper::Context &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow_csm","")
{
	SetBaseShader<ShaderShadow>();
}
void ShaderShadowCSM::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadowCSM>({{{
		{
			RENDER_PASS_DEPTH_FORMAT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,
			Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
		}
	}}},outRenderPass,pipelineIdx);
}
 // prosper TODO
#if 0
#include "pragma/clientstate/clientstate.h"
#include "c_shader_shadow.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include <pragma/console/convars.h>
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_side.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/rendering/lighting/c_light_point.h"
#include "pragma/rendering/lighting/c_light_spot.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/lighting/c_light_directional.h"
#include <sharedutils/scope_guard.h>

using namespace Shader;

LINK_SHADER_TO_CLASS(Shadow,shadow);
LINK_SHADER_TO_CLASS(ShadowSpot,shadow_spot);
LINK_SHADER_TO_CLASS(ShadowTransparent,shadowtransparent);
LINK_SHADER_TO_CLASS(ShadowTransparentSpot,shadowtransparent_spot);
LINK_SHADER_TO_CLASS(ShadowCSM,shadowcsm);
LINK_SHADER_TO_CLASS(ShadowCSMTransparent,shadowcsmtransparent);
LINK_SHADER_TO_CLASS(ShadowCSMStatic,shadowcsmstatic);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

Shadow::MeshInfo::MeshInfo(Material *mat,CModelSubMesh *_mesh)
	: material(mat),mesh(_mesh)
{}

//////////////////////////

ShadowTransparent::ShadowTransparent(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Shadow(identifier,vsShader,fsShader,gsShader)
{}
ShadowTransparent::ShadowTransparent()
	: ShadowTransparent("ShadowTransparent","shadow/vs_shadow_transparent","shadow/fs_shadow_transparent")
{}

void ShadowTransparent::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Shadow::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,1
	});
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Diffuse Map
	}));
}

void ShadowTransparent::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Shadow::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::UV),
		umath::to_integral(Binding::UV),
		Anvil::Format::R32G32_SFLOAT,sizeof(Vector3)
	});
}

bool ShadowTransparent::BindMaterial(Vulkan::CommandBufferObject *cmdBuffer,Material *mat)
{
	auto &descTexture = static_cast<CMaterial*>(mat)->GetDescriptorSet();
	if(!descTexture.IsValid())
		return false;
	auto *data = mat->GetDataBlock();
	auto alphaDiscardThreshold = DEFAULT_ALPHA_DISCARD_THRESHOLD;
	if(data != nullptr)
		data->GetFloat("alpha_discard_threshold",&alphaDiscardThreshold);
	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,1,&alphaDiscardThreshold);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DiffuseMap),layout,descTexture);
	return true;
}

void ShadowTransparent::Render(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,uint32_t imgLayer,CLightBase::RenderPass rp,const std::unordered_map<CBaseEntity*,std::unique_ptr<std::vector<std::shared_ptr<MeshInfo>>>> &meshInfo)
{
	if(BeginDraw() == false)
		return;
	auto &layout = GetPipeline()->GetPipelineLayout();
	//auto &meshes = light->GetCulledMeshes(rp);
	auto *shadow = light->GetShadowMap();
	auto &depthMvp = light->GetTransformationMatrix(imgLayer);
	auto *tex = (light->GetType() != LightType::Directional) ? shadow->GetDepthTexture() : &static_cast<ShadowMapCasc*>(shadow)->GetDepthTexture(rp);
	auto &img = (*tex)->GetImage();
	//auto numLayers = shadow->GetLayerCount();
	auto w = img->GetWidth();
	auto h = img->GetHeight();
	cmdBuffer->SetViewport(w,h);
	cmdBuffer->SetScissor(w,h); // Required, but why? We don't have a dynamic scissor state...
	BindLight(cmdBuffer,light);

	auto &scene = c_game->GetRenderScene(); // Vulkan TODO
	BindScene(cmdBuffer,*scene);
	for(auto &pair : meshInfo)
	{
		auto *ent = pair.first;
		auto &meshes = pair.second;

		auto bWeighted = TexturedBase3D::BindEntity(cmdBuffer,ent);
		auto entMvp = depthMvp *(*ent->GetTransformationMatrix());
		cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,16,&entMvp);

		for(auto &matMesh : *meshes)
		{
			auto *mat = matMesh->material;
			auto &descTexture = static_cast<CMaterial*>(mat)->GetDescriptorSet();
			if(!descTexture.IsValid())
				continue;

			auto *data = mat->GetDataBlock();
			auto alphaDiscardThreshold = DEFAULT_ALPHA_DISCARD_THRESHOLD;
			if(data != nullptr)
				data->GetFloat("alpha_discard_threshold",&alphaDiscardThreshold);
			cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,1,&alphaDiscardThreshold);
			cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DiffuseMap),layout,descTexture);
			RenderMesh(cmdBuffer,matMesh->mesh,bWeighted);
		}
	}
	EndDraw();
}

ShadowTransparentSpot::ShadowTransparentSpot()
	: ShadowTransparent("shadowtransparent_spot","shadow/vs_shadow_transparent","shadow/fs_shadow_transparent_spot")
{}

//////////////////////////

Shadow::Shadow(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: TexturedBase3D(identifier,vsShader,fsShader,gsShader)
{
	SetUseColorAttachments(false);
	SetUseBloomAttachment(false);
}

Shadow::Shadow()
	: Shadow("Shadow","shadow/vs_shadow","shadow/fs_shadow")
{}

void Shadow::BindScene(Vulkan::CommandBufferObject*,const Scene&,bool) {}

void Shadow::InitializeRenderPasses() {m_renderPasses = {m_context->GenerateRenderPass(Anvil::Format::D32_SFLOAT)};}
void Shadow::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	Base::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(Binding::Vertex),
		sizeof(Vertex),
		Anvil::VertexInputRate::VERTEX
	});
	vertexBindingDescriptions.push_back({
		static_cast<uint32_t>(Binding::BoneWeight),
		sizeof(VertexWeight),
		Anvil::VertexInputRate::VERTEX
	});

	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::Vertex),
		static_cast<uint32_t>(Binding::Vertex),
		Anvil::Format::R32G32B32_SFLOAT,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::BoneWeightId),
		static_cast<uint32_t>(Binding::BoneWeightId),
		vk::Format::eR32G32B32A32Sint,0
	});
	vertexAttributeDescriptions.push_back({
		static_cast<uint32_t>(Location::BoneWeight),
		static_cast<uint32_t>(Binding::BoneWeight),
		Anvil::Format::R32G32B32A32_SFLOAT,sizeof(Vector4i)
	});
}

void Shadow::SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info)
{
	TexturedBase3D::SetupPipeline(pipelineIdx,info);
	auto *rasterizationState = const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState);
	rasterizationState->cullMode = vk::CullModeFlagBits::eNone;
}

void Shadow::InitializeShaderPipelines(const Vulkan::Context &context)
{
	Base::InitializeShaderPipelines(context);
}

void Shadow::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::FRAGMENT_BIT,0,20
	});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Instance
		{Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,Anvil::ShaderStageFlagBits::VERTEX_BIT} // Bone Matrices
	}));
}

#if DEBUG_SHADOWS == 1
extern DLLCLIENT uint32_t s_shadowMeshCount;
extern DLLCLIENT uint32_t s_shadowIndexCount;
extern DLLCLIENT uint32_t s_shadowTriangleCount;
extern DLLCLIENT uint32_t s_shadowVertexCount;
#endif
void Shadow::RenderMesh(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh,bool bWeighted,const std::function<void(const Vulkan::CommandBufferObject*,uint32_t)> &fDraw)
{
#if DEBUG_SHADOWS == 1
	++s_shadowMeshCount;
	s_shadowIndexCount += mesh->GetTriangleVertexCount();
	s_shadowTriangleCount += mesh->GetTriangleCount();
	s_shadowVertexCount += mesh->GetVertexCount();
#endif
	auto &vkMesh = mesh->GetVKMesh();
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
		return;
	}

	//auto &context = *m_context.get();
	//auto &buf = context.GetSquareVertexBuffer();
	//auto numVerts = context.GetSquareVertexCount();
	
	if(bWeighted == true && vertexWeightBuffer.IsValid() == true)
	{
		cmdBuffer->BindVertexBuffer({
			vertexBuffer,
			vertexWeightBuffer
		});
	}
	else
		cmdBuffer->BindVertexBuffer({vertexBuffer,c_game->GetDummyVertexBuffer()});

	cmdBuffer->BindIndexBuffer(indexBuffer,vk::IndexType::eUint16);
	fDraw(cmdBuffer,mesh->GetTriangleVertexCount());
}

void Shadow::RenderMesh(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh,bool bWeighted)
{
	RenderMesh(cmdBuffer,mesh,bWeighted,[](const Vulkan::CommandBufferObject *drawCmd,uint32_t triangleVertCount) {
		drawCmd->DrawIndexed(triangleVertCount);
	});
}

void Shadow::RenderMeshes(Vulkan::CommandBufferObject *cmdBuffer,uint32_t idxLayer,std::vector<std::shared_ptr<CLightBase::EntityInfo>> &meshes,const Mat4 &depthMvp,bool bPushConstants,std::unordered_map<CBaseEntity*,std::unique_ptr<std::vector<std::shared_ptr<MeshInfo>>>> *transparentMeshes)
{
	auto &scene = c_game->GetRenderScene(); // Vulkan TODO
	BindScene(cmdBuffer,*scene);
	auto &layout = GetPipeline()->GetPipelineLayout();
	CBaseEntity *entLast = nullptr;
	auto bWeighted = false;
	for(auto it=meshes.begin();it!=meshes.end();++it)
	{
		auto &info = *(*it);
		if(info.renderFlags == 0 || (info.renderFlags &(1<<idxLayer)))
		{
			auto *ent = static_cast<CBaseEntity*>(info.hEntity.get());
			//if(ent->IsWorld())
			//	continue;
			//if(ent->IsPlayer())
			//	std::cout<<"Player Shadow!"<<std::endl;
			// Push Constants
			if(ent != entLast) // TODO: Sort by entity? (Is that even worth it?)
			{
				ent->UpdateRenderData();
				entLast = ent;

				bWeighted = TexturedBase3D::BindEntity(cmdBuffer,ent); // Vulkan TODO: Entity Render Info (+Matrices), as well as bone matrices have to be updated!
				/*auto &renderBuffer = ent->GetRenderBuffer();
				auto &boneBuffer = ent->GetBoneBuffer();
				drawCmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::Instance),layout,descSetInstance,{
					static_cast<uint32_t>(renderBuffer->GetBaseOffset()), // Instance Buffer
					boneBuffer.IsValid() ? static_cast<uint32_t>(boneBuffer->GetBaseOffset()) : 0 // Bone Matrices
				});*/
				if(bPushConstants == true)
				{
					auto entMvp = depthMvp *(*ent->GetTransformationMatrix());
					cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,16,&entMvp);
				}
			}
			auto &mdl = ent->GetModel();
			auto &mats = mdl->GetMaterials();
			auto numMats = mats.size();
			for(auto &meshInfo : info.meshes)
			{
				if(meshInfo.renderFlags != 0 && !(meshInfo.renderFlags &(1<<idxLayer)))
					continue;
				auto *mesh = meshInfo.mesh;
				auto texId = mesh->GetTexture();
				if(texId < numMats)
				{
					auto &hMat = mats[texId];
					if(hMat.IsValid() && hMat->IsTranslucent())
					{
						if(transparentMeshes != nullptr)
						{
							auto it = transparentMeshes->find(ent);
							if(it == transparentMeshes->end())
								it = transparentMeshes->insert(std::pair<CBaseEntity*,std::unique_ptr<std::vector<std::shared_ptr<MeshInfo>>>>(ent,std::make_unique<std::vector<std::shared_ptr<MeshInfo>>>())).first;
							it->second->push_back(std::shared_ptr<MeshInfo>(new MeshInfo(hMat.get(),mesh)));
						}
						continue;
					}
				}
				RenderMesh(cmdBuffer,mesh,bWeighted);
			}
		}
	}
}
bool Shadow::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	return Base3D::BeginDraw(cmdBuffer,shaderPipeline); // Skip TexturedBase3D::BeginDraw
}
void Shadow::EndDraw(Vulkan::CommandBufferObject *cmdBuffer)
{
	Base3D::EndDraw(cmdBuffer); // Skip TexturedBase3D::EndDraw
}

void Shadow::BindLight(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light)
{
	auto *ranged = dynamic_cast<CLightRanged*>(light);
	if(ranged == nullptr)
		return;
	auto &pos = light->GetPosition();
	auto lightPos = Vector4{pos.x,pos.y,pos.z,static_cast<float>(ranged->GetDistance())};
	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,16,4,&lightPos);
}

void Shadow::Render(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,uint32_t imgLayer,CLightBase::RenderPass rp)
{
	auto *shadow = light->GetShadowMap();
	auto *tex = (light->GetType() != LightType::Directional) ? shadow->GetDepthTexture() : &static_cast<ShadowMapCasc*>(shadow)->GetDepthTexture(rp);
	auto &img = (*tex)->GetImage();
	Render(cmdBuffer,light,light->GetCulledMeshes(rp),img->GetWidth(),img->GetHeight(),imgLayer,rp);
}

void Shadow::Render(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,std::vector<std::shared_ptr<CLightBase::EntityInfo>> &meshes,uint32_t w,uint32_t h,uint32_t imgLayer,CLightBase::RenderPass rp)
{
	if(BeginDraw() == false)
		return;
	auto *shadow = light->GetShadowMap();
	//auto numLayers = shadow->GetLayerCount();
	cmdBuffer->SetViewport(w,h);
	cmdBuffer->SetScissor(w,h); // Required, but why? We don't have a dynamic scissor state...
	BindLight(cmdBuffer,light);
	std::unordered_map<CBaseEntity*,std::unique_ptr<std::vector<std::shared_ptr<MeshInfo>>>> transparentMeshes;
	if(light->GetType() != LightType::Directional)
		RenderMeshes(cmdBuffer,imgLayer,meshes,light->GetTransformationMatrix(imgLayer),true,&transparentMeshes);
	else
		RenderMeshes(cmdBuffer,imgLayer,meshes,static_cast<ShadowMapCasc*>(shadow)->GetViewProjectionMatrix(imgLayer),true,&transparentMeshes);
	if(!transparentMeshes.empty())
	{
		if(hShaderTransparent.IsValid())
		{
			auto &shaderTransparent = static_cast<ShadowTransparent&>(*hShaderTransparent.get());
			shaderTransparent.Render(cmdBuffer,light,imgLayer,rp,transparentMeshes);
		}
	}
	EndDraw();
}

bool Shadow::BeginDrawTest(CLightBase *light,uint32_t w,uint32_t h)
{
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	if(BeginDraw(drawCmd) == false)
		return false;
	auto *shadow = light->GetShadowMap();
	//auto numLayers = shadow->GetLayerCount();
	drawCmd->SetViewport(w,h);
	drawCmd->SetScissor(w,h); // Required, but why? We don't have a dynamic scissor state...
	BindLight(drawCmd,light);

	// Bind camera
	auto &scene = c_game->GetRenderScene(); // Vulkan TODO
	BindScene(drawCmd,*scene);

	m_bWeighted = false;
	m_lightSource = light;
	return true;
}
void Shadow::EndDrawTest()
{
	m_bWeighted = false;
	m_lightSource = nullptr;
	m_renderFlags = 0;
	EndDraw();
}
void Shadow::BindDepthMatrix(Vulkan::CommandBufferObject *cmdBuffer,const Mat4 &mat)
{
	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,16,&mat);
}
bool Shadow::BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,const Mat4 &depthMvp)
{
	ent->UpdateRenderData();
	auto bWeighted = TexturedBase3D::BindEntity(cmdBuffer,ent);

	auto entMvp = depthMvp *(*ent->GetTransformationMatrix());
	BindDepthMatrix(cmdBuffer,entMvp);
	m_bWeighted = bWeighted;

	return bWeighted;
}
void Shadow::DrawTest(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh,const std::function<void(const Vulkan::CommandBufferObject*,uint32_t)> &fDraw)
{
	RenderMesh(cmdBuffer,mesh,false,fDraw);
}
void Shadow::DrawTest(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh)
{
	RenderMesh(cmdBuffer,mesh,m_bWeighted);
}

ShadowSpot::ShadowSpot()
	: Shadow("shadow_spot","shadow/vs_shadow","shadow/fs_shadow_spot")
{}

///////////////////////////////////

ShadowCSM::ShadowCSM(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Shadow(identifier,vsShader,fsShader,gsShader)
{}
ShadowCSM::ShadowCSM()
	: ShadowCSM("ShadowCSM","shadow/vs_shadow_csm","")
{}

void ShadowCSM::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Shadow::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants = {
		{Anvil::ShaderStageFlagBits::VERTEX_BIT,0,16}
	};
}

void ShadowCSM::BindLight(Vulkan::CommandBufferObject*,CLightBase*) {}

//////////////////////////

ShadowCSMTransparent::ShadowCSMTransparent()
	: ShadowCSM("ShadowCSMTransparent","shadow/vs_shadow_csm_transparent","shadow/fs_shadow_csm_transparent")
{}

void ShadowCSMTransparent::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	ShadowCSM::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,1
	});
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Diffuse Map
	}));
}

void ShadowCSMTransparent::InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions)
{
	ShadowCSM::InitializeVertexDescriptions(vertexBindingDescriptions,vertexAttributeDescriptions);
	vertexAttributeDescriptions.push_back({
		umath::to_integral(Location::UV),
		umath::to_integral(Binding::UV),
		Anvil::Format::R32G32_SFLOAT,sizeof(Vector3)
	});
}

bool ShadowCSMTransparent::BindMaterial(Vulkan::CommandBufferObject *cmdBuffer,Material *mat)
{
	auto &descTexture = static_cast<CMaterial*>(mat)->GetDescriptorSet();
	if(!descTexture.IsValid())
		return false;
	auto *data = mat->GetDataBlock();
	auto alphaDiscardThreshold = DEFAULT_ALPHA_DISCARD_THRESHOLD;
	if(data != nullptr)
		data->GetFloat("alpha_discard_threshold",&alphaDiscardThreshold);
	auto &layout = GetPipeline()->GetPipelineLayout();
	cmdBuffer->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT,20,1,&alphaDiscardThreshold);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DiffuseMap),layout,descTexture);
	return true;
}

///////////////////////////////////

decltype(ShadowCSMStatic::MAX_CASCADE_COUNT) ShadowCSMStatic::MAX_CASCADE_COUNT = ShadowMapCasc::MAX_CASCADE_COUNT *10;
ShadowCSMStatic::ShadowCSMStatic()
	: ShadowCSM("ShadowCSMStatic","shadow/vs_shadow_csm_static","shadow/fs_shadow_csm")
{}

void ShadowCSMStatic::Render(CLightDirectional &light,std::vector<Vulkan::SwapCommandBuffer> &cmdBuffers,CLightBase::EntityInfo &info)
{
	auto *shadow = light.GetShadowMap();
	if(shadow == nullptr)
		return;
	auto *csm = static_cast<ShadowMapCasc*>(shadow);
	auto &tex = csm->GetDepthTexture(CLightBase::RenderPass::Dynamic);
	if(!tex.IsValid())
		return;
	auto &img = tex->GetImage();
	auto numLayers = shadow->GetLayerCount();
	auto &context = c_engine->GetRenderContext();

	auto &scene = c_game->GetRenderScene();
	auto w = img->GetWidth();
	auto h = img->GetHeight();
	auto &renderPass = csm->GetRenderPass(CLightBase::RenderPass::Dynamic);
	std::vector<std::shared_ptr<CLightBase::EntityInfo>> entInfos {};
	entInfos.push_back(std::shared_ptr<CLightBase::EntityInfo>(&info,[](CLightBase::EntityInfo *info) {}));
	cmdBuffers.reserve(numLayers);
	for(auto i=decltype(numLayers){0};i<numLayers;++i)
	{
		auto &framebuffer = csm->GetFramebuffer(CLightBase::RenderPass::Dynamic,i);
		auto cmds = Vulkan::SwapCommandBuffer::Create(context,vk::CommandBufferLevel::eSecondary);
		uint32_t swapIdx = 0;
		for(auto &cmd : cmds->GetCommandBuffers())
		{
			cmd->Begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse,renderPass,framebuffer,0);
				cmd->SetImageLayout(img,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				//img->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				//auto numLayers = shadow->GetLayerCount();
				if(BeginDraw(cmd) == true)
				{
					auto &descSet = *scene->GetCSMShadowDescriptorSet(i,swapIdx);
					auto &buf = *scene->GetCSMShadowBuffer(i,swapIdx);
					cmd->InsertBarrier(vk::BufferMemoryBarrier(
						vk::AccessFlagBits::eHostWrite,
						vk::AccessFlagBits::eUniformRead,
						VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED,
						buf->GetVkBuffer(),buf->GetBaseOffset(),buf->GetSize()
					));
					cmd->InsertBarrier(vk::BufferMemoryBarrier(
						vk::AccessFlagBits::eUniformRead,
						vk::AccessFlagBits::eHostWrite,
						VK_QUEUE_FAMILY_IGNORED,VK_QUEUE_FAMILY_IGNORED,
						buf->GetVkBuffer(),buf->GetBaseOffset(),buf->GetSize()
					));
					//BindLight(cmd,&light);
					cmd->SetViewport(w,h);
					cmd->SetScissor(w,h); // Required, but why? We don't have a dynamic scissor state...
					cmd->BindDescriptorSet(static_cast<uint32_t>(DescSet::DepthMatrix),GetPipeline()->GetPipelineLayout(),descSet);
					RenderMeshes(cmd,i,entInfos,static_cast<ShadowMapCasc*>(shadow)->GetViewProjectionMatrix(i),false);

					EndDraw(cmd);
				}
			cmd->End();
			++swapIdx;
		}
		cmdBuffers.push_back(cmds);
	}
}

void ShadowCSMStatic::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	ShadowCSM::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants = {};
	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::VERTEX_BIT} // Depth Matrix
	}));
}
#endif
