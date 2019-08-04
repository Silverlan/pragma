#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include <mathutil/umath.h>
#include <mathutil/umat.h>
#include <pragma/console/convars.h>
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/math/math_seb.h"
#include <pragma/model/modelmesh.h>
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/math/e_frustum.h>
#include <sharedutils/scope_guard.h>
#include <pragma/model/model.h>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
static const uint8_t LAYER_UPDATE_FREQUENCY = 3; // Frames

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

#define CSM_MAX_SHADOW_DISTANCE 10'000
#define CSM_SHADOW_SPLIT_FACTOR 0.9f

FrustumSplit::FrustumSplit()
	: neard(0.f),fard(0.f)
{}

Frustum::Frustum()
	: radius(0.f)
{
	bounds[0] = Vector3(0.f,0.f,0.f);
	bounds[1] = Vector3(0.f,0.f,0.f);
	projection = umat::identity();
}

static void cmd_cl_render_shadow_pssm_split_count(NetworkState*,ConVar*,int,int val)
{
	if(c_game == NULL)
		return;
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightComponent>>();
	for(auto *ent : entIt)
	{
		auto pLightBase = ent->GetComponent<pragma::CLightComponent>();
		auto type = LightType::Invalid;
		auto *pLight = pLightBase->GetLight(type);
		if(pLight != nullptr && type == LightType::Directional)
		{
			ShadowMap *shadow = pLightBase->GetShadowMap();
			if(shadow != NULL)
			{
				ShadowMapCasc *csm = static_cast<ShadowMapCasc*>(shadow);
				csm->SetSplitCount(val);
				csm->ReloadDepthTextures();
			}
		}
	}
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_pssm_split_count,cmd_cl_render_shadow_pssm_split_count);

static void cmd_render_csm_max_distance(NetworkState*,ConVar*,float,float val)
{
	if(c_game == NULL)
		return;
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightComponent>>();
	for(auto *ent : entIt)
	{
		auto pLightBase = ent->GetComponent<pragma::CLightComponent>();
		auto type = LightType::Invalid;
		auto *pLight = pLightBase->GetLight(type);
		if(pLight != nullptr && type == LightType::Directional)
		{
			ShadowMap *shadow = pLightBase->GetShadowMap();
			if(shadow != NULL)
			{
				ShadowMapCasc *csm = static_cast<ShadowMapCasc*>(shadow);
				csm->SetMaxDistance(val);
			}
		}
	}
}
REGISTER_CONVAR_CALLBACK_CL(render_csm_max_distance,cmd_render_csm_max_distance);

static CVar cvCascadeCount = GetClientConVar("cl_render_shadow_pssm_split_count");
static CVar cvRange = GetClientConVar("render_csm_max_distance");
ShadowMapCasc::ShadowMapCasc()
	: ShadowMap(cvCascadeCount->GetInt(),true),m_frustums(),m_fard(NULL),
	m_maxDistance(cvRange->GetFloat()),//m_descSet(nullptr),m_renderPassKeep(nullptr), // prosper TODO
	m_layerUpdate{false}
{
	SetSplitCount(cvCascadeCount->GetInt());
	UpdateSplitDistances(2.f,GetMaxDistance());

	m_whShaderCsm = c_engine->GetShader("shadowcsm");
	m_whShaderCsmTransparent = c_engine->GetShader("shadowcsmtransparent");

	auto hShader = c_engine->GetShader("textured");
	/*if(!hShader.IsValid())
		return;
	auto *shader = static_cast<Shader::Textured3D*>(hShader.get());
	auto *pipeline = shader->GetPipeline();
	if(pipeline != nullptr)
	{
		auto &layout = const_cast<Vulkan::ShaderPipeline*>(pipeline)->GetDescriptorSetLayout(umath::to_integral(Shader::TexturedBase3D::DescSet::CSMTextureArray));
		auto &pool = layout->GetContext().GetDescriptorPool(layout->GetDescriptorType());
		m_descSet = pool->CreateDescriptorSet(layout);
	}*/ // prosper TODO
}

uint64_t ShadowMapCasc::GetLastUpdateFrameId() const {return m_lastFrameUpdate;}
void ShadowMapCasc::SetLastUpdateFrameId(uint64_t id) {m_lastFrameUpdate = id;}

//const Vulkan::DescriptorSet &ShadowMapCasc::GetDescriptorSet() const {return m_descSet;} // prosper TODO

float ShadowMapCasc::GetMaxDistance() {return m_maxDistance;}
void ShadowMapCasc::SetMaxDistance(float dist)
{
	m_maxDistance = dist;
	UpdateSplitDistances(2.f,dist);
}

float *ShadowMapCasc::GetSplitFarDistances() {return &m_fard.front();}

unsigned int ShadowMapCasc::GetSplitCount() {return m_numSplits;}
void ShadowMapCasc::SetSplitCount(unsigned int numSplits)
{
	if(numSplits == 0)
		numSplits = 1;
	else if(numSplits > ShadowMapCasc::MAX_CASCADE_COUNT)
		numSplits = ShadowMapCasc::MAX_CASCADE_COUNT;
	m_numSplits = numSplits;
	m_layerCount = numSplits;

	auto &csmBuffer = c_game->GetGlobalRenderSettingsBufferData().csmBuffer;
	auto splitCount = static_cast<decltype(pragma::ShaderTextured3DBase::CSMData::count)>(numSplits);
	c_engine->ScheduleRecordUpdateBuffer(csmBuffer,offsetof(pragma::ShaderTextured3DBase::CSMData,count),splitCount);

	m_frustums.resize(m_numSplits);
	m_fard.resize(m_numSplits);
	UpdateSplitDistances(2.f,GetMaxDistance());
}

Mat4 &ShadowMapCasc::GetProjectionMatrix(unsigned int layer) {return m_frustums[layer].projection;}
Mat4 &ShadowMapCasc::GetViewProjectionMatrix(unsigned int layer) {return m_frustums[layer].viewProjection;}

void ShadowMapCasc::UpdateSplitDistances(float nd,float fd)
{
	const float splitWeight = 0.75f;
	float lambda = splitWeight;
	float ratio = fd /nd;
	m_frustums[0].split.neard = nd;
	unsigned int numCascades = GetSplitCount();
	for(unsigned int i=1;i<numCascades;i++)
	{
		float si = i /float(numCascades);
		m_frustums[i].split.neard = lambda *(nd *powf(ratio,si))
			+(1.f -lambda) *(nd +(fd -nd) *si);
		m_frustums[i -1].split.fard = m_frustums[i].split.neard *1.005f;
	}
	m_frustums[numCascades -1].split.fard = fd;
}

Frustum *ShadowMapCasc::GetFrustumSplit(unsigned int splitId)
{
	if(splitId >= m_numSplits)
		return nullptr;
	return &m_frustums[splitId];
}

bool ShadowMapCasc::ShouldUpdateLayer(uint32_t layerId) const
{
	return (layerId < m_layerUpdate.size() && m_layerUpdate[layerId] == true) ? true : false;
}

void ShadowMapCasc::SetFrustumUpdateCallback(const std::function<void(void)> &f) {m_onFrustumUpdated = f;}

static CVar cvUpdateFrequency = GetClientConVar("cl_render_shadow_update_frequency");
static CVar cvUpdateFrequencyOffset = GetClientConVar("cl_render_shadow_pssm_update_frequency_offset");
static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
void ShadowMapCasc::UpdateFrustum(uint32_t splitId,pragma::CCameraComponent &cam,const Mat4 &matView,const Vector3 &dir)
{
	auto &frustumSplit = m_frustums.at(splitId);
	m_pendingInfo.prevVpMatrices.at(splitId) = m_vpMatrices.at(splitId);
	m_pendingInfo.prevProjectionMatrices.at(splitId) = frustumSplit.projection;

	auto zNear = cam.GetNearZ();
	auto zFar = cam.GetFarZ();

	// Update points and planes
	frustumSplit.points.clear();
	frustumSplit.planes.clear();
	auto &split = frustumSplit.split;
	auto &entCam = cam.GetEntity();
	auto trCam = entCam.GetTransformComponent();
	cam.SetNearZ(split.neard);
	cam.SetFarZ(split.fard);
	cam.GetFrustumPlanes(frustumSplit.planes);
	cam.GetFrustumPoints(
		frustumSplit.points,
		split.neard,split.fard,
		cam.GetFOVRad(),cam.GetAspectRatio(),
		entCam.GetPosition(),
		trCam.valid() ? trCam->GetForward() : uvec::FORWARD,
		trCam.valid() ? trCam->GetUp() : uvec::UP
	);

	cam.SetNearZ(zNear);
	cam.SetFarZ(zFar);

	auto &ftr = frustumSplit.points[static_cast<int>(FrustumPoint::FarTopRight)];
	auto &fbl = frustumSplit.points[static_cast<int>(FrustumPoint::FarBottomLeft)];
	auto &nbl = frustumSplit.points[static_cast<int>(FrustumPoint::NearBottomLeft)];
	auto &ntr = frustumSplit.points[static_cast<int>(FrustumPoint::NearTopRight)];

	std::vector<Vector3> trapezoid = {ftr,fbl,nbl,ntr};
	auto &center = frustumSplit.center;
	auto &radius = frustumSplit.radius;
	Seb::Calculate(trapezoid,center,radius);

	auto diameter = radius *2.f;
	auto frustumTop = center -dir *diameter;
	auto perp = uvec::get_perpendicular(dir);
	auto matViewSplit = glm::lookAtRH(frustumTop,frustumTop +dir,uvec::get_perpendicular(perp));

	std::array<Vector3,2> bounds = {-Vector3(diameter,diameter,diameter),Vector3(diameter,diameter,diameter)};
	frustumSplit.projection = glm::ortho(bounds.at(0).z,bounds.at(1).z,bounds.at(0).x,bounds.at(1).x,-bounds.at(1).y *2.f,bounds.at(1).y *2.f);
	auto scale = Vector3(1.f,-1.f,1.f);
	frustumSplit.projection = glm::scale(frustumSplit.projection,scale);

	frustumSplit.viewProjection = frustumSplit.projection *matViewSplit;

	// Update Frustum OBB and AABB (Required for culling)
	uvec::to_min_max(bounds.at(0),bounds.at(1));
	auto &obbCenter = frustumSplit.obbCenter;
	obbCenter = (bounds.at(0) +bounds.at(1)) /2.f;
	bounds.at(0) -= obbCenter;
	bounds.at(1) -= obbCenter;
	obbCenter += center;//frustumTop; // TODO: Is this correct?

	auto &obb = frustumSplit.obb;
	obb.min = bounds.at(0);
	obb.max = bounds.at(1);
	obb.rotation = uquat::create_look_rotation(dir,perp);

	auto &aabb = frustumSplit.aabb;
	AABB::GetRotatedBounds(obb.min,obb.max,umat::create(obb.rotation),&aabb.min,&aabb.max);
	//

	m_vpMatrices.at(splitId) = m_frustums.at(splitId).viewProjection;
	m_fard.at(splitId) = m_frustums.at(splitId).split.fard;

	if(m_onFrustumUpdated != nullptr)
		m_onFrustumUpdated();
	
	auto &camProj = cam.GetProjectionMatrix();
	auto &splitDistance = m_pendingInfo.prevSplitDistances;

	auto &csmBuffer = c_game->GetGlobalRenderSettingsBufferData().csmBuffer;
	c_engine->ScheduleRecordUpdateBuffer(csmBuffer,offsetof(pragma::ShaderTextured3DBase::CSMData,VP),m_pendingInfo.prevVpMatrices.size() *sizeof(Mat4),m_pendingInfo.prevVpMatrices.data());
	c_engine->ScheduleRecordUpdateBuffer(csmBuffer,offsetof(pragma::ShaderTextured3DBase::CSMData,fard),splitDistance);

	// Calculate new split distances
	splitDistance[splitId] = 0.5f *(-frustumSplit.split.fard *camProj[2][2] +camProj[3][2]) /frustumSplit.split.fard +0.5f;
}
void ShadowMapCasc::InitializeTextureSet(TextureSet &set,pragma::CLightComponent::RenderPass rp)
{
	auto wpShaderShadow = c_engine->GetShader("shadowcsm");
	if(wpShaderShadow.expired())
		return;
	auto size = cvShadowmapSize->GetInt();
	if(size <= 0)
		return;
	const auto layerCount = m_layerCount;
	const auto type = GetType();

	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = size;
	imgCreateInfo.height = size;
	imgCreateInfo.format = pragma::ShaderShadow::RENDER_PASS_DEPTH_FORMAT;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	if(rp == pragma::CLightComponent::RenderPass::Static)
		imgCreateInfo.usage |= Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
	imgCreateInfo.layers = layerCount;
	if(type == ShadowMap::Type::Cube)
		imgCreateInfo.flags = prosper::util::ImageCreateInfo::Flags::Cubemap;
	auto img = prosper::util::create_image(dev,imgCreateInfo);

	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	//samplerCreateInfo.compareEnable = true; // When enabled, causes strange behavior on Nvidia cards when doing texture lookups
	//samplerCreateInfo.compareOp = Anvil::CompareOp::LESS_OR_EQUAL;
	samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.borderColor = Anvil::BorderColor::FLOAT_OPAQUE_WHITE;

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags |= prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer;
	auto tex = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);
	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	rtCreateInfo.useLayerFramebuffers = true;

	set.renderTarget = prosper::util::create_render_target(dev,{tex},static_cast<prosper::ShaderGraphics*>(wpShaderShadow.get())->GetRenderPass(),rtCreateInfo);
	set.renderTarget->SetDebugName("csm_rt");
}
void ShadowMapCasc::UpdateFrustum(pragma::CCameraComponent &cam,const Mat4 &matView,const Vector3 &dir)
{
	auto numCascades = GetSplitCount();
	for(auto i=decltype(numCascades){0};i<numCascades;++i)
		UpdateFrustum(i,cam,matView,dir);
}
/*const Vulkan::RenderPass &ShadowMapCasc::GetRenderPassKeep() const {return m_renderPassKeep;}
const Vulkan::Framebuffer &ShadowMapCasc::GetFramebufferKeep(uint32_t layer) const
{
	assert(layer < m_framebuffersKeep.size());
	return (layer < m_framebuffersKeep.size()) ? m_framebuffersKeep[layer] : m_framebuffersKeep.front();
}
const std::vector<Vulkan::Framebuffer> &ShadowMapCasc::GetFramebuffersKeep() const {return m_framebuffersKeep;}
*/ // prosper TODO
ShadowMap::Type ShadowMapCasc::GetType() {return ShadowMap::Type::Cascaded;}

void ShadowMapCasc::RenderBatch(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CLightDirectionalComponent &light)
{
	auto pLightComponent = light.GetEntity().GetComponent<pragma::CLightComponent>();
	auto &rt = GetStaticPendingRenderTarget();
	if(m_whShaderCsm.expired() || pLightComponent.expired() || rt == nullptr)
		return;
	auto &shaderCsm = static_cast<pragma::ShaderShadowCSM&>(*m_whShaderCsm.get());
	auto *shaderCsmTransparent = m_whShaderCsmTransparent.expired() == false ? static_cast<pragma::ShaderShadowCSMTransparent*>(m_whShaderCsmTransparent.get()) : nullptr;
	auto &tex = rt->GetTexture();
	auto &img = tex->GetImage();
	auto &info = m_pendingInfo;
	auto rp = pragma::CLightComponent::RenderPass::Static;

	auto &renderPass = rt->GetRenderPass();
	auto numLayers = GetLayerCount();
	for(auto layer=decltype(numLayers){0};layer<numLayers;++layer)
	{
		//img->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,0,img->GetMipmapLevels(),layer,1); // Required for clearing the image
					
		//auto &framebuffer = shadowMap.GetFramebuffer(rp,i);
		auto &framebuffer = rt->GetFramebuffer(layer);

		auto bBlit = false;
		//img->SetInternalLayout(layout); // prosper TODO
		//img->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0,img->GetMipmapLevels(),layer,1); // prosper TODO

		prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,layer);

		prosper::util::record_begin_render_pass(*(*drawCmd),*rt,layer);
			auto &meshesInfo = info.meshes.at(layer);
			if(meshesInfo.entityMeshes.empty() == false)
			{
				//const uint32_t drawCountPerLayer = 4;
				static uint32_t drawCountPerLayer = 4;
				auto meshCount = drawCountPerLayer;
				auto layerFlag = 1<<layer;
				if(shaderCsm.BeginDraw(drawCmd) == true)
				{
					shaderCsm.BindLight(*pLightComponent);
					for(auto it=meshesInfo.entityMeshes.begin();it!=meshesInfo.entityMeshes.end();)
					{
						auto &info = *it;
						if(info.hEntity.IsValid() == false)
						{
							it = meshesInfo.entityMeshes.erase(it);
							continue;
						}
						auto *ent = static_cast<CBaseEntity*>(info.hEntity.get());
						auto &mdlComponent = ent->GetRenderComponent()->GetModelComponent();
						auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
						if(mdl == nullptr)
						{
							it = meshesInfo.entityMeshes.erase(it);
							continue;
						}

						// Bind entity to shader
						shaderCsm.BindEntity(*ent,GetViewProjectionMatrix(layer));

						while(meshCount > 0 && info.meshes.empty() == false)
						{
							auto &ptrMesh = info.meshes.front();
							if(ptrMesh.expired() == false)
							{
								uint32_t renderFlags = 0;
								auto mesh = ptrMesh.lock();
								if(info.bAlreadyPassed == true || pLightComponent->ShouldPass(*ent,*static_cast<CModelMesh*>(mesh.get()),renderFlags) == true && (renderFlags &layerFlag) != 0)
								{
									for(auto &subMesh : mesh->GetSubMeshes())
									{
										uint32_t renderFlags = 0;
										auto &cSubMesh = *static_cast<CModelSubMesh*>(subMesh.get());
										if(pLightComponent->ShouldPass(*mdl,cSubMesh) == true)
										{
											auto *mat = mdl->GetMaterial(cSubMesh.GetTexture());
											if(mat != nullptr && mat->IsTranslucent())
											{
												meshesInfo.translucentMeshes.push_back({});
												auto &translucent = meshesInfo.translucentMeshes.back();
												translucent.hEntity = info.hEntity;
												translucent.subMeshes.push(subMesh);
											}
											else
												shaderCsm.Draw(cSubMesh);
										}
									}
									--meshCount;
								}
							}
							info.meshes.pop();
						}
						/*
						auto &meshGroups = mdl->GetMeshGroups();
						auto &meshGroupId = info.meshGroupId;
						while(meshCount > 0 && meshGroupId < meshGroups.size())
						{
							auto &meshGroup = meshGroups.at(meshGroupId);
							auto &meshes = meshGroup->GetMeshes();
							while(meshCount > 0)
							{
								auto &meshId = info.meshId;
								if(meshId >= meshes.size())
								{
									++meshGroupId;
									meshId = 0;
									break;
								}
								auto &mesh = meshes.at(meshId);
								auto &subMeshId = info.subMeshId;
								uint32_t renderFlags = 0;
								if(light.ShouldPass(ent,static_cast<CModelMesh*>(mesh.get()),renderFlags) == true && (renderFlags &layerFlag) != 0)
								{
									auto &subMeshes = mesh->GetSubMeshes();
									while(meshCount > 0)
									{
										if(subMeshId >= subMeshes.size())
										{
											++meshId;
											subMeshId = 0;
											break;
										}
										auto &subMesh = subMeshes.at(subMeshId++);
										uint32_t renderFlags = 0;
										if(light.ShouldPass(*mdl,static_cast<CModelSubMesh*>(subMesh.get())) == true)
										{
											shaderCsm.DrawTest(drawCmd,static_cast<CModelSubMesh*>(subMesh.get()));
											--meshCount;
										}
									}
								}
								else
								{
									++meshId;
									subMeshId = 0;
								}
							}
						}*/
						if(meshCount == 0)
							break;
						it = meshesInfo.entityMeshes.erase(it);
					}
					shaderCsm.EndDraw();
				}

				// Render transparent meshes
				if(meshesInfo.translucentMeshes.empty() == false)
				{
					if(shaderCsmTransparent != nullptr)
					{
						CBaseEntity *prevEntity = nullptr;
						Material *prevMat = nullptr;
						if(shaderCsmTransparent->BeginDraw(drawCmd) == true)
						{
							shaderCsmTransparent->BindLight(*pLightComponent);
							for(auto &meshInfo : meshesInfo.translucentMeshes)
							{
								if(meshInfo.hEntity.IsValid() == false)
									continue;
								auto *ent = static_cast<CBaseEntity*>(meshInfo.hEntity.get());
								if(ent != prevEntity)
								{
									shaderCsmTransparent->BindEntity(*ent,GetViewProjectionMatrix(layer));
									prevEntity = ent;
								}
								auto &mdlComponent = ent->GetRenderComponent()->GetModelComponent();
								auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
								while(meshInfo.subMeshes.empty() == false)
								{
									auto &wpSubMesh = meshInfo.subMeshes.front();
									if(wpSubMesh.expired() == false)
									{
										auto *cSubMesh = static_cast<CModelSubMesh*>(wpSubMesh.lock().get());
										auto *mat = mdl->GetMaterial(cSubMesh->GetTexture());
										if(mat != nullptr)
										{
											if(mat == prevMat || shaderCsmTransparent->BindMaterial(static_cast<CMaterial&>(*mat)) == true)
											{
												shaderCsmTransparent->Draw(*cSubMesh);
												prevMat = mat;
											}
										}
									}
									meshInfo.subMeshes.pop();
								}
							}
							shaderCsmTransparent->EndDraw();
						}
					}
					meshesInfo.translucentMeshes.clear();
				}
			}
		prosper::util::record_end_render_pass(*(*drawCmd));
	}
	auto *cam = c_game->GetPrimaryCamera();
	auto camPos = cam ? cam->GetEntity().GetPosition() : Vector3{};
	for(auto layer=decltype(numLayers){0};layer<numLayers;++layer)
	{
		auto &meshesInfo = info.meshes.at(layer);
		if(meshesInfo.entityMeshes.empty() == false) // Not complete yet
			continue;
		// Layer is complete, reset data for next iteration
		auto &meshes = m_pendingInfo.meshes.at(layer);
		meshes.entityMeshes.resize(c_game->GetBaseEntities().size());
		auto i = decltype(m_pendingInfo.meshes.size()){0};
		std::vector<CWorld*> worldEnts;

		EntityIterator entIt{*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
		for(auto *ent : entIt)
		{
			auto pRenderComponent = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
			if(ent->IsInert() == false || pRenderComponent->ShouldDrawShadow(camPos) == false)
				continue;
			if(ent->IsWorld())
			{
				worldEnts.push_back(static_cast<CWorld*>(ent));
				continue;
			}
			uint32_t renderFlags = 0;
			if(pLightComponent->ShouldPass(static_cast<CBaseEntity&>(*ent),renderFlags) == true)
			{
				auto &info = meshes.entityMeshes.at(i++);
				info.hEntity = ent->GetHandle();
				info.meshes = {};
				info.bAlreadyPassed = false;
				auto &lodMeshes = pRenderComponent->GetLODMeshes(); // TODO: Which LOD meshes?
				for(auto &mesh : lodMeshes)
					info.meshes.push(mesh);
			}
		}

		for(auto *entWorld : worldEnts)
		{
			auto &info = meshes.entityMeshes.at(i++);
			info.hEntity = entWorld->GetHandle();
			info.meshes = {};
			info.bAlreadyPassed = false;
			auto pWorldComponent = entWorld->GetComponent<pragma::CWorldComponent>();
			auto meshTree = pWorldComponent.valid() ? pWorldComponent->GetMeshTree() : nullptr;
			if(meshTree != nullptr)
			{
				meshTree->IterateObjects([layer,&light](const OcclusionOctree<std::shared_ptr<ModelMesh>>::Node &node) -> bool {
					auto &bounds = node.GetWorldBounds();
					return light.ShouldPass(layer,bounds.first,bounds.second);
				},[layer,&light,&info](const std::shared_ptr<ModelMesh> &mesh) {
					Vector3 min,max;
					mesh->GetBounds(min,max);
					if(light.ShouldPass(layer,min,max) == false)
						return;
					info.meshes.push(mesh);
				});
			}
		}
		meshes.entityMeshes.resize(i);

		auto &dstImg = GetDepthTexture(rp)->GetImage();

		/*drawCmd->SetImageLayout(img,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,0u,1u,layer);
		img->SetInternalLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		drawCmd->SetImageLayout(dstImg,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,0u,1u,layer);
		dstImg->SetInternalLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);*/ // prosper TODO

		prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,layer);

		prosper::util::record_image_barrier(*(*drawCmd),*(*dstImg),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,layer);

		// Blit finished shadow map into our actual scene shadow map
		prosper::util::BlitInfo blitInfo {};
		blitInfo.srcSubresourceLayer.base_array_layer = blitInfo.dstSubresourceLayer.base_array_layer = layer;
		blitInfo.srcSubresourceLayer.layer_count = blitInfo.dstSubresourceLayer.layer_count = 1u;
		prosper::util::record_blit_image(*(*drawCmd),blitInfo,*(*img),*(*dstImg));

		prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,layer);

		prosper::util::record_image_barrier(*(*drawCmd),*(*dstImg),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,layer);

		//drawCmd->SetImageLayout(dstImg,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,0u,1u,layer); // prosper TODO
		//dstImg->SetInternalLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO

		// Update matrices and CSM-Buffer
		light.UpdateFrustum(layer);
		/*m_swapchainUpdatesRequired = (1<<(*buffer)->GetBufferCount()) -1;

		drawCmd->SetImageLayout(img,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,0u,1u,layer);
		img->SetInternalLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);*/ // prosper TODO

		prosper::util::ClearImageInfo clearImageInfo {};
		clearImageInfo.subresourceRange.baseArrayLayer = layer;
		clearImageInfo.subresourceRange.layerCount = 1u;
		prosper::util::record_clear_image(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,1.f,clearImageInfo); // Clear layer

		prosper::util::record_image_barrier(*(*drawCmd),*(*img),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,layer);

		//drawCmd->SetImageLayout(img,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,0u,1u,layer); // prosper TODO
		//img->SetInternalLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // prosper TODO
	}
}
