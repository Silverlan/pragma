// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.renderer_model;

import :client_state;
import :entities.components;
import :game;
import :rendering.shaders;

decltype(pragma::pts::CParticleRendererModel::s_rendererCount) pragma::pts::CParticleRendererModel::s_rendererCount = 0;
static std::shared_ptr<prosper::IBuffer> s_instanceBuffer = nullptr;
static std::shared_ptr<prosper::IBuffer> s_instanceBufferAnimated = nullptr;
static std::shared_ptr<prosper::IDescriptorSetGroup> s_instanceDescSetGroup = nullptr;
void pragma::pts::CParticleRendererModel::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleRenderer::Initialize(pSystem, values);
	m_rotationalBuffer.Initialize(pSystem);

	auto skin = 0u;
	m_shader = get_cengine()->GetShader("particlemodel");
	std::unordered_map<uint32_t, uint32_t> bodyGroups;
	std::string mdl;
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "model")
			mdl = pair.second;
		else if(key == "skin")
			skin = util::to_int(pair.second);
		else if(key == "animation") {
			if(pair.second.empty() == false)
				m_animation = pair.second;
		}
		else {
			auto sub = string::substr(key, 0, 9);
			if(sub == "bodygroup") {
				auto id = util::to_int(string::substr(key, 9));
				bodyGroups.at(id) = util::to_int(pair.second);
			}
		}
	}

	auto fCreateAnimatedComponent = [this, &pSystem]() { return pSystem.GetEntity().AddComponent<CAnimatedComponent>(true); };

	if(s_rendererCount++ == 0 && ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE.IsValid()) {
		rendering::InstanceData instanceData {umat::identity(), Vector4(1.f, 1.f, 1.f, 1.f), rendering::InstanceData::RenderFlags::None};
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.size = sizeof(instanceData);
		createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;

		s_instanceDescSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE);
		s_instanceBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &instanceData);
		s_instanceDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(*s_instanceBuffer, 0u);

		instanceData.renderFlags = rendering::InstanceData::RenderFlags::Weighted;
		s_instanceBufferAnimated = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &instanceData);
	}

	auto bAnimated = !m_animation.empty();
	if(bAnimated == false) // If not animated, we only need one model component
		m_particleComponents.push_back(ParticleModelComponent {fCreateAnimatedComponent(), nullptr});
	else // We need one model component per particle (expensive!!)
	{
		auto maxParticles = static_cast<ecs::CParticleSystemComponent &>(pSystem).GetMaxParticleCount();
		m_particleComponents.reserve(maxParticles);
		for(auto i = decltype(maxParticles) {0u}; i < maxParticles; ++i)
			m_particleComponents.push_back(ParticleModelComponent {fCreateAnimatedComponent(), nullptr});
	}
	if(ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE.IsValid()) {
		for(auto &ptComponent : m_particleComponents) {
			auto wpBoneBuffer = ptComponent.animatedComponent->GetBoneBuffer();
			if(wpBoneBuffer) {
				// If we are animated, we have to create a unique descriptor set
				ptComponent.instanceDescSetGroupAnimated = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE);
				ptComponent.instanceDescSetGroupAnimated->GetDescriptorSet()->SetBindingUniformBuffer(*s_instanceBufferAnimated, 0u);
				ptComponent.instanceDescSetGroupAnimated->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IBuffer &>(*wpBoneBuffer), math::to_integral(ShaderGameWorldLightingPass::InstanceBinding::BoneMatrices));
			}
		}
	}
}

pragma::pts::CParticleRendererModel::~CParticleRendererModel()
{
	if(--s_rendererCount == 0) {
		s_instanceDescSetGroup = nullptr;
		s_instanceBuffer = nullptr;
		s_instanceBufferAnimated = nullptr;
	}
}

pragma::ShaderParticleBase *pragma::pts::CParticleRendererModel::GetShader() const { return static_cast<ShaderParticleModel *>(m_shader.get()); }

pragma::pts::CParticleRendererModel::ParticleModelComponent &pragma::pts::CParticleRendererModel::GetParticleComponent(uint32_t particleIdx)
{
	if(particleIdx >= m_particleComponents.size())
		return m_particleComponents.front();
	return m_particleComponents.at(particleIdx);
}

void pragma::pts::CParticleRendererModel::OnParticleCreated(CParticle &particle)
{
	CParticleRenderer::OnParticleCreated(particle);
	if(m_animation.empty())
		return;
	auto &ptComponent = GetParticleComponent(particle.GetIndex());
	ptComponent.animatedComponent->PlayAnimation(m_animation, FPlayAnim::Reset);
}

bool pragma::pts::CParticleRendererModel::IsAnimated() const { return m_animation.empty() == false; }

void pragma::pts::CParticleRendererModel::PostSimulate(double tDelta)
{
	CParticleRenderer::PostSimulate(tDelta);
	if(m_particleComponents.front().animatedComponent.expired() || m_shader.expired() || s_instanceDescSetGroup == nullptr)
		return;
	Update();

	// Update animation
	if(IsAnimated() == false)
		return;
	auto &drawCmd = get_cengine()->GetDrawCommandBuffer();
	auto numRenderParticles = GetParticleSystem().GetRenderParticleCount();
	for(auto i = decltype(numRenderParticles) {0u}; i < numRenderParticles; ++i) {
		auto ptIdx = GetParticleSystem().TranslateBufferIndex(i);
		auto &ptComponent = GetParticleComponent(ptIdx);
		auto &animComponent = ptComponent.animatedComponent;
		if(animComponent.expired())
			continue;
		animComponent->MaintainAnimations(get_cgame()->DeltaTime());
		animComponent->UpdateBoneBuffer(*drawCmd);
	}
}

bool pragma::pts::CParticleRendererModel::Update()
{
	auto *cam = get_cgame()->GetRenderCamera<CCameraComponent>();
	if(m_rotationalBuffer.Update() == false || cam == nullptr)
		return false;
	// Update meshes
	auto &posCam = cam->GetEntity().GetPosition();
	auto &renderBounds = GetParticleSystem().GetRenderBounds();
	Vector3 p;
	math::geometry::closest_point_on_aabb_to_point(renderBounds.first, renderBounds.second, posCam, &p);
	auto dist = uvec::distance(posCam, p);

	auto bSuccessful = true;
	auto mdlComponent = GetParticleSystem().GetEntity().GetModelComponent();
	if(mdlComponent && mdlComponent->HasModel()) {
		auto &mdl = mdlComponent->GetModel();
		auto lod = get_cgame()->GetLOD(dist, mdl->GetLODCount());
		if(static_cast<CModelComponent &>(*mdlComponent).GetLOD() != lod)
			bSuccessful = false;
		static_cast<CModelComponent &>(*mdlComponent).UpdateLOD(lod);
	}
	return bSuccessful;
}

void pragma::pts::CParticleRendererModel::RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags)
{
#if 0
	if(m_shader.expired())
		return;
	auto *shader = static_cast<pragma::ShaderParticleModel*>(m_shader.get());
	auto animBuffer = GetParticleSystem().GetParticleAnimationBuffer();
	if(animBuffer == nullptr)
		animBuffer = pragma::get_cengine()->GetRenderContext().GetDummyBuffer();
	if(
		shader->BeginDraw(drawCmd,{},GetParticleSystem()) == false || 
		shader->BindParticleBuffers(*GetParticleSystem().GetParticleBuffer(),*m_rotationalBuffer.GetBuffer(),*animBuffer) == false || 
		shader->BindParticleSystem(GetParticleSystem()) == false
	)
		return;
	shader->BindLights(*renderer.GetLightSourceDescriptorSet());
	shader->BindSceneCamera(scene,renderer,(GetParticleSystem().GetSceneRenderPass() == pragma::rendering::SceneRenderPass::View) ? true : false);
	shader->BindRenderSettings(pragma::get_cgame()->GetGlobalRenderSettingsDescriptorSet());

	auto mdlComponent = GetParticleSystem().GetEntity().GetModelComponent();
	if(mdlComponent && mdlComponent->HasModel())
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
					auto *cSubMesh = static_cast<pragma::geometry::CModelSubMesh*>(subMesh.get());
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
					shader->BindMaterial(static_cast<material::CMaterial&>(*mat));
					shader->Draw(*static_cast<pragma::geometry::CModelSubMesh*>(subMesh.get()),bAnimated ? 1u : instanceCount,instanceIdx); // TODO: Bloom?
				}
			}
		}
	}
	shader->EndDraw();
#endif
}

void pragma::pts::CParticleRendererModel::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId)
{
	/*if(s_instanceDescSet == nullptr)
		return;
	static auto hShader = pragma::get_cengine()->GetShader("particlemodelshadow");
	if(!hShader.IsValid())
		return;
	auto *shadowMap = light->GetShadowMap();
	if(shadowMap == nullptr)
		return;
	Update();

	auto *depthTex = shadowMap->GetDepthTexture();
	auto w = (*depthTex)->GetWidth();
	auto h = (*depthTex)->GetHeight();
	auto &context = pragma::get_cengine()->GetRenderContext();
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
			auto *cSubMesh = static_cast<pragma::geometry::CModelSubMesh*>(subMesh.get());
			shader.DrawTest(cSubMesh,instanceCount);
		}
	}
	shader.EndDrawTest();*/ // prosper TODO
}
