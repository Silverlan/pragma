/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/renderers/c_particle_mod_model.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_model.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_model_shadow.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/scene/scene.h"
#include <pragma/math/intersection.h>
#include <pragma/model/model.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

REGISTER_PARTICLE_RENDERER(model,CParticleRendererModel);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;


decltype(CParticleRendererModel::s_rendererCount) CParticleRendererModel::s_rendererCount = 0;
static std::shared_ptr<prosper::IBuffer> s_instanceBuffer = nullptr;
static std::shared_ptr<prosper::IBuffer> s_instanceBufferAnimated = nullptr;
static std::shared_ptr<prosper::IDescriptorSetGroup> s_instanceDescSetGroup = nullptr;
void CParticleRendererModel::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleRenderer::Initialize(pSystem,values);
	m_rotationalBuffer.Initialize(pSystem);

	auto skin = 0u;
	m_shader = c_engine->GetShader("particlemodel");
	std::unordered_map<uint32_t,uint32_t> bodyGroups;
	std::string mdl;
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "model")
			mdl = pair.second;
		else if(key == "skin")
			skin = util::to_int(pair.second);
		else if(key == "animation")
		{
			if(pair.second.empty() == false)
				m_animation = pair.second;
		}
		else
		{
			auto sub = ustring::substr(key,0,9);
			if(sub == "bodygroup")
			{
				auto id = util::to_int(ustring::substr(key,9));
				bodyGroups.at(id) = util::to_int(pair.second);
			}
		}
	}

	auto fCreateAnimatedComponent = [this,&pSystem]() {
		return pSystem.GetEntity().AddComponent<pragma::CAnimatedComponent>(true);
	};

	if(s_rendererCount++ == 0 && pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE.IsValid())
	{
		pragma::ShaderEntity::InstanceData instanceData {umat::identity(),Vector4(1.f,1.f,1.f,1.f),pragma::ShaderEntity::InstanceData::RenderFlags::None};
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.size = sizeof(instanceData);
		createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;

		s_instanceDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE);
		s_instanceBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&instanceData);
		s_instanceDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(
			*s_instanceBuffer,0u
		);

		instanceData.renderFlags = pragma::ShaderEntity::InstanceData::RenderFlags::Weighted;
		s_instanceBufferAnimated = c_engine->GetRenderContext().CreateBuffer(createInfo,&instanceData);
	}

	auto bAnimated = !m_animation.empty();
	if(bAnimated == false) // If not animated, we only need one model component
		m_particleComponents.push_back(ParticleModelComponent{fCreateAnimatedComponent(),nullptr});
	else // We need one model component per particle (expensive!!)
	{
		auto maxParticles = pSystem.GetMaxParticleCount();
		m_particleComponents.reserve(maxParticles);
		for(auto i=decltype(maxParticles){0u};i<maxParticles;++i)
			m_particleComponents.push_back(ParticleModelComponent{fCreateAnimatedComponent(),nullptr});
	}
	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE.IsValid())
	{
		for(auto &ptComponent : m_particleComponents)
		{
			auto wpBoneBuffer = ptComponent.animatedComponent->GetBoneBuffer();
			if(wpBoneBuffer.expired() == false)
			{
				// If we are animated, we have to create a unique descriptor set
				ptComponent.instanceDescSetGroupAnimated = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE);
				ptComponent.instanceDescSetGroupAnimated->GetDescriptorSet()->SetBindingUniformBuffer(
					*s_instanceBufferAnimated,0u
				);
				ptComponent.instanceDescSetGroupAnimated->GetDescriptorSet()->SetBindingUniformBuffer(
					*wpBoneBuffer.lock(),umath::to_integral(pragma::ShaderTextured3DBase::InstanceBinding::BoneMatrices)
				);
			}
		}
	}
}

CParticleRendererModel::~CParticleRendererModel()
{
	if(--s_rendererCount == 0)
	{
		s_instanceDescSetGroup = nullptr;
		s_instanceBuffer = nullptr;
		s_instanceBufferAnimated = nullptr;
	}
}

pragma::ShaderParticleBase *CParticleRendererModel::GetShader() const
{
	return static_cast<pragma::ShaderParticleModel*>(m_shader.get());
}

CParticleRendererModel::ParticleModelComponent &CParticleRendererModel::GetParticleComponent(uint32_t particleIdx)
{
	if(particleIdx >= m_particleComponents.size())
		return m_particleComponents.front();
	return m_particleComponents.at(particleIdx);
}

void CParticleRendererModel::OnParticleCreated(CParticle &particle)
{
	CParticleRenderer::OnParticleCreated(particle);
	if(m_animation.empty())
		return;
	auto &ptComponent = GetParticleComponent(particle.GetIndex());
	ptComponent.animatedComponent->PlayAnimation(m_animation,pragma::FPlayAnim::Reset);
}

bool CParticleRendererModel::IsAnimated() const {return m_animation.empty() == false;}

void CParticleRendererModel::PostSimulate(double tDelta)
{
	CParticleRenderer::PostSimulate(tDelta);
	if(m_particleComponents.front().animatedComponent.expired() || m_shader.expired() || s_instanceDescSetGroup == nullptr)
		return;
	Update();

	// Update animation
	if(IsAnimated() == false)
		return;
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	auto numRenderParticles = GetParticleSystem().GetRenderParticleCount();
	for(auto i=decltype(numRenderParticles){0u};i<numRenderParticles;++i)
	{
		auto ptIdx = GetParticleSystem().TranslateBufferIndex(i);
		auto &ptComponent = GetParticleComponent(ptIdx);
		auto &animComponent = ptComponent.animatedComponent;
		if(animComponent.expired())
			continue;
		animComponent->MaintainAnimations(c_game->DeltaTime());
		animComponent->UpdateBoneBuffer(*drawCmd);
	}
}

bool CParticleRendererModel::Update()
{
	auto *cam = c_game->GetRenderCamera();
	if(m_rotationalBuffer.Update() == false || cam == nullptr)
		return false;
	// Update meshes
	auto &posCam = cam->GetEntity().GetPosition();
	auto &renderBounds = GetParticleSystem().GetRenderBounds();
	Vector3 p;
	Geometry::ClosestPointOnAABBToPoint(renderBounds.first,renderBounds.second,posCam,&p);
	auto dist = uvec::distance(posCam,p);

	auto bSuccessful = true;
	auto mdlComponent = GetParticleSystem().GetEntity().GetModelComponent();
	if(mdlComponent.valid() && mdlComponent->HasModel())
	{
		auto &mdl = mdlComponent->GetModel();
		auto lod = c_game->GetLOD(dist,mdl->GetLODCount());
		if(static_cast<pragma::CModelComponent&>(*mdlComponent).GetLOD() != lod)
			bSuccessful = false;
		static_cast<pragma::CModelComponent&>(*mdlComponent).UpdateLOD(lod);
	}
	return bSuccessful;
}

void CParticleRendererModel::Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags)
{
	if(m_shader.expired())
		return;
	auto *shader = static_cast<pragma::ShaderParticleModel*>(m_shader.get());
	auto animBuffer = GetParticleSystem().GetParticleAnimationBuffer();
	if(animBuffer == nullptr)
		animBuffer = c_engine->GetRenderContext().GetDummyBuffer();
	if(
		shader->BeginDraw(drawCmd,{},GetParticleSystem()) == false || 
		shader->BindParticleBuffers(*GetParticleSystem().GetParticleBuffer(),*m_rotationalBuffer.GetBuffer(),*animBuffer) == false || 
		shader->BindParticleSystem(GetParticleSystem()) == false
	)
		return;
	auto &descSetShadowmps = *renderer.GetCSMDescriptorSet();
	auto &descSetLightSources = *renderer.GetForwardPlusInstance().GetDescriptorSetGraphics();
	shader->BindLights(descSetShadowmps,descSetLightSources);
	shader->BindSceneCamera(renderer,(GetParticleSystem().GetRenderMode() == RenderMode::View) ? true : false);
	shader->BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());

	auto mdlComponent = GetParticleSystem().GetEntity().GetModelComponent();
	if(mdlComponent.valid() && mdlComponent->HasModel())
	{
		auto &mdl = *mdlComponent->GetModel();
		auto &materials = mdl.GetMaterials();
		auto *texGroup = mdl.GetTextureGroup(mdlComponent->GetSkin());
		if(texGroup == nullptr)
			texGroup = mdl.GetTextureGroup(0);

		auto *matPt = GetParticleSystem().GetMaterial();
		auto bAnimated = IsAnimated();
		auto instanceCount = GetParticleSystem().GetRenderParticleCount();
		auto numRenderPasses = bAnimated ? instanceCount : 1u;
		for(auto instanceIdx=decltype(numRenderPasses){0u};instanceIdx<numRenderPasses;++instanceIdx)
		{
			auto &ptComponent = GetParticleComponent(instanceIdx);
			auto &animComponent = ptComponent.animatedComponent;
			auto &descSet = animComponent.valid() ? ptComponent.instanceDescSetGroupAnimated : s_instanceDescSetGroup;
			if(shader->BindInstanceDescriptorSet(*descSet->GetDescriptorSet()) == false)
				continue;
			for(auto &mesh : static_cast<pragma::CModelComponent&>(*mdlComponent).GetLODMeshes())
			{
				for(auto &subMesh : mesh->GetSubMeshes())
				{
					auto *cSubMesh = static_cast<CModelSubMesh*>(subMesh.get());
					auto texId = mdl.GetMaterialIndex(*cSubMesh);
					auto *mat = matPt;
					if(mat == nullptr && texGroup != nullptr && texId.has_value() && *texId < texGroup->textures.size())
					{
						texId = texGroup->textures.at(*texId);
						if(texId < materials.size())
							mat = materials.at(*texId).get();
					}
					if(mat == nullptr)
						continue;
					shader->BindMaterial(static_cast<CMaterial&>(*mat));
					shader->Draw(*static_cast<CModelSubMesh*>(subMesh.get()),bAnimated ? 1u : instanceCount,instanceIdx); // TODO: Bloom?
				}
			}
		}
	}
	shader->EndDraw();
}

void CParticleRendererModel::RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId)
{
	/*if(s_instanceDescSet == nullptr)
		return;
	static auto hShader = c_engine->GetShader("particlemodelshadow");
	if(!hShader.IsValid())
		return;
	auto *shadowMap = light->GetShadowMap();
	if(shadowMap == nullptr)
		return;
	Update();

	auto *depthTex = shadowMap->GetDepthTexture();
	auto w = (*depthTex)->GetWidth();
	auto h = (*depthTex)->GetHeight();
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	auto &shader = static_cast<Shader::ParticleModelShadow&>(*hShader.get());
	if(shader.BeginDrawTest(*m_particleSystem->GetParticleBuffer(),m_rotBuffer,drawCmd,light,w,h) == false)
		return;
	shader.BindInstanceDescriptorSet(drawCmd,s_instanceDescSet,light->GetTransformationMatrix(layerId));
	auto instanceCount = m_particleSystem->GetRenderParticleCount();
	for(auto &mesh : m_lodMeshes)
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto *cSubMesh = static_cast<CModelSubMesh*>(subMesh.get());
			shader.DrawTest(cSubMesh,instanceCount);
		}
	}
	shader.EndDrawTest();*/ // prosper TODO
}
