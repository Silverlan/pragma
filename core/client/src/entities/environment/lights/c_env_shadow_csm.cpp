/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/math/math_seb.h>
#include <pragma/math/e_frustum.h>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static const uint8_t LAYER_UPDATE_FREQUENCY = 3; // Frames

#define CSM_MAX_SHADOW_DISTANCE 10'000
#define CSM_SHADOW_SPLIT_FACTOR 0.9f

FrustumSplit::FrustumSplit() : neard(0.f), fard(0.f) {}

Frustum::Frustum() : radius(0.f)
{
	bounds[0] = Vector3(0.f, 0.f, 0.f);
	bounds[1] = Vector3(0.f, 0.f, 0.f);
	projection = umat::identity();
}

static void cmd_cl_render_shadow_pssm_split_count(NetworkState *, ConVar *, int, int val)
{
	if(c_game == NULL)
		return;
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CShadowCSMComponent>>();
	for(auto *ent : entIt) {
		auto hCsmC = ent->GetComponent<pragma::CShadowCSMComponent>();
		if(hCsmC.expired())
			continue;
		hCsmC->SetSplitCount(val);
		hCsmC->ReloadDepthTextures();
	}
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_pssm_split_count, cmd_cl_render_shadow_pssm_split_count);

static void cmd_render_csm_max_distance(NetworkState *, ConVar *, float, float val)
{
	if(c_game == NULL)
		return;
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CShadowCSMComponent>>();
	for(auto *ent : entIt) {
		auto hCsmC = ent->GetComponent<pragma::CShadowCSMComponent>();
		if(hCsmC.expired())
			continue;
		hCsmC->SetMaxDistance(val);
	}
}
REGISTER_CONVAR_CALLBACK_CL(render_csm_max_distance, cmd_render_csm_max_distance);

static CVar cvCascadeCount = GetClientConVar("cl_render_shadow_pssm_split_count");
static CVar cvRange = GetClientConVar("render_csm_max_distance");
CShadowCSMComponent::CShadowCSMComponent(BaseEntity &ent) : BaseEntityComponent {ent}, m_maxDistance {cvRange->GetFloat()}, m_layerUpdate {false}
{
	SetSplitCount(cvCascadeCount->GetInt());
	UpdateSplitDistances(2.f, GetMaxDistance());

	m_whShaderCsm = c_engine->GetShader("shadowcsm");
	m_whShaderCsmTransparent = c_engine->GetShader("shadowcsmtransparent");
}

void CShadowCSMComponent::Initialize() { BaseEntityComponent::Initialize(); }

void CShadowCSMComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

uint64_t CShadowCSMComponent::GetLastUpdateFrameId() const { return m_lastFrameUpdate; }
void CShadowCSMComponent::SetLastUpdateFrameId(uint64_t id) { m_lastFrameUpdate = id; }

//const Vulkan::DescriptorSet &CShadowCSMComponent::GetDescriptorSet() const {return m_descSet;} // prosper TODO

float CShadowCSMComponent::GetMaxDistance() { return m_maxDistance; }
void CShadowCSMComponent::SetMaxDistance(float dist)
{
	m_maxDistance = dist;
	UpdateSplitDistances(2.f, dist);
}

float *CShadowCSMComponent::GetSplitFarDistances() { return &m_fard.front(); }

unsigned int CShadowCSMComponent::GetSplitCount() { return m_numSplits; }
void CShadowCSMComponent::SetSplitCount(unsigned int numSplits)
{
	if(numSplits == 0)
		numSplits = 1;
	else if(numSplits > CShadowCSMComponent::MAX_CASCADE_COUNT)
		numSplits = CShadowCSMComponent::MAX_CASCADE_COUNT;
	m_numSplits = numSplits;
	m_layerCount = numSplits;

	auto &csmBuffer = c_game->GetGlobalRenderSettingsBufferData().csmBuffer;
	auto splitCount = static_cast<decltype(pragma::ShaderGameWorldLightingPass::CSMData::count)>(numSplits);
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(csmBuffer, offsetof(pragma::ShaderGameWorldLightingPass::CSMData, count), splitCount);

	m_frustums.resize(m_numSplits);
	m_fard.resize(m_numSplits);
	UpdateSplitDistances(2.f, GetMaxDistance());
}

Mat4 &CShadowCSMComponent::GetProjectionMatrix(unsigned int layer) { return m_frustums[layer].projection; }
Mat4 &CShadowCSMComponent::GetViewProjectionMatrix(unsigned int layer) { return m_frustums[layer].viewProjection; }

void CShadowCSMComponent::UpdateSplitDistances(float nd, float fd)
{
	const float splitWeight = 0.75f;
	float lambda = splitWeight;
	float ratio = fd / nd;
	m_frustums[0].split.neard = nd;
	unsigned int numCascades = GetSplitCount();
	for(unsigned int i = 1; i < numCascades; i++) {
		float si = i / float(numCascades);
		m_frustums[i].split.neard = lambda * (nd * powf(ratio, si)) + (1.f - lambda) * (nd + (fd - nd) * si);
		m_frustums[i - 1].split.fard = m_frustums[i].split.neard * 1.005f;
	}
	m_frustums[numCascades - 1].split.fard = fd;
}

Frustum *CShadowCSMComponent::GetFrustumSplit(unsigned int splitId)
{
	if(splitId >= m_numSplits)
		return nullptr;
	return &m_frustums[splitId];
}

bool CShadowCSMComponent::ShouldUpdateLayer(uint32_t layerId) const { return (layerId < m_layerUpdate.size() && m_layerUpdate[layerId] == true) ? true : false; }

uint32_t CShadowCSMComponent::GetLayerCount() const { return m_layerCount; }

void CShadowCSMComponent::SetFrustumUpdateCallback(const std::function<void(void)> &f) { m_onFrustumUpdated = f; }

static CVar cvUpdateFrequency = GetClientConVar("cl_render_shadow_update_frequency");
static CVar cvUpdateFrequencyOffset = GetClientConVar("cl_render_shadow_pssm_update_frequency_offset");
static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
void CShadowCSMComponent::UpdateFrustum(uint32_t splitId, pragma::CCameraComponent &cam, const Mat4 &matView, const Vector3 &dir)
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
	cam.GetFrustumPoints(frustumSplit.points, split.neard, split.fard, cam.GetFOVRad(), cam.GetAspectRatio(), entCam.GetPosition(), trCam ? trCam->GetForward() : uvec::FORWARD, trCam ? trCam->GetUp() : uvec::UP);

	cam.SetNearZ(zNear);
	cam.SetFarZ(zFar);

	auto &ftr = frustumSplit.points[static_cast<int>(FrustumPoint::FarTopRight)];
	auto &fbl = frustumSplit.points[static_cast<int>(FrustumPoint::FarBottomLeft)];
	auto &nbl = frustumSplit.points[static_cast<int>(FrustumPoint::NearBottomLeft)];
	auto &ntr = frustumSplit.points[static_cast<int>(FrustumPoint::NearTopRight)];

	std::vector<Vector3> trapezoid = {ftr, fbl, nbl, ntr};
	auto &center = frustumSplit.center;
	auto &radius = frustumSplit.radius;
	Seb::Calculate(trapezoid, center, radius);

	auto diameter = radius * 2.f;
	auto frustumTop = center - dir * diameter;
	auto perp = uvec::get_perpendicular(dir);
	auto matViewSplit = glm::lookAtRH(frustumTop, frustumTop + dir, uvec::get_perpendicular(perp));

	std::array<Vector3, 2> bounds = {-Vector3(diameter, diameter, diameter), Vector3(diameter, diameter, diameter)};
	frustumSplit.projection = glm::ortho(bounds.at(0).z, bounds.at(1).z, bounds.at(0).x, bounds.at(1).x, -bounds.at(1).y * 2.f, bounds.at(1).y * 2.f);
	auto scale = Vector3(1.f, -1.f, 1.f);
	frustumSplit.projection = glm::scale(frustumSplit.projection, scale);

	frustumSplit.viewProjection = frustumSplit.projection * matViewSplit;

	// Update Frustum OBB and AABB (Required for culling)
	uvec::to_min_max(bounds.at(0), bounds.at(1));
	auto &obbCenter = frustumSplit.obbCenter;
	obbCenter = (bounds.at(0) + bounds.at(1)) / 2.f;
	bounds.at(0) -= obbCenter;
	bounds.at(1) -= obbCenter;
	obbCenter += center; //frustumTop; // TODO: Is this correct?

	auto &obb = frustumSplit.obb;
	obb.min = bounds.at(0);
	obb.max = bounds.at(1);
	obb.rotation = uquat::create_look_rotation(dir, perp);

	auto &aabb = frustumSplit.aabb;
	bounding_volume::AABB::GetRotatedBounds(obb.min, obb.max, umat::create(obb.rotation), &aabb.min, &aabb.max);
	//

	m_vpMatrices.at(splitId) = m_frustums.at(splitId).viewProjection;
	m_fard.at(splitId) = m_frustums.at(splitId).split.fard;

	if(m_onFrustumUpdated != nullptr)
		m_onFrustumUpdated();

	auto &camProj = cam.GetProjectionMatrix();
	auto &splitDistance = m_pendingInfo.prevSplitDistances;

	auto &csmBuffer = c_game->GetGlobalRenderSettingsBufferData().csmBuffer;
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(csmBuffer, offsetof(pragma::ShaderGameWorldLightingPass::CSMData, VP), m_pendingInfo.prevVpMatrices.size() * sizeof(Mat4), m_pendingInfo.prevVpMatrices.data());
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(csmBuffer, offsetof(pragma::ShaderGameWorldLightingPass::CSMData, fard), splitDistance);

	// Calculate new split distances
	splitDistance[splitId] = 0.5f * (-frustumSplit.split.fard * camProj[2][2] + camProj[3][2]) / frustumSplit.split.fard + 0.5f;
}
void CShadowCSMComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CShadowCSMComponent::InitializeTextureSet(TextureSet &set, pragma::CLightComponent::ShadowMapType smType)
{
	auto wpShaderShadow = c_engine->GetShader("shadowcsm");
	if(wpShaderShadow.expired())
		return;
	auto size = cvShadowmapSize->GetInt();
	if(size <= 0)
		return;
	const auto layerCount = m_layerCount;

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = size;
	imgCreateInfo.height = size;
	imgCreateInfo.format = pragma::ShaderShadow::RENDER_PASS_DEPTH_FORMAT;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::DepthStencilAttachmentBit | prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	if(smType == pragma::CLightComponent::ShadowMapType::Static)
		imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;
	imgCreateInfo.layers = layerCount;
	auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);

	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	//samplerCreateInfo.compareEnable = true; // When enabled, causes strange behavior on Nvidia cards when doing texture lookups
	//samplerCreateInfo.compareOp = Anvil::CompareOp::LESS_OR_EQUAL;
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToBorder;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToBorder;
	samplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToBorder;
	samplerCreateInfo.borderColor = prosper::BorderColor::FloatOpaqueWhite;

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags |= prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer;
	auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	rtCreateInfo.useLayerFramebuffers = true;

	set.renderTarget = c_engine->GetRenderContext().CreateRenderTarget({tex}, static_cast<prosper::ShaderGraphics *>(wpShaderShadow.get())->GetRenderPass(), rtCreateInfo);
	set.renderTarget->SetDebugName("csm_rt");
}
void CShadowCSMComponent::UpdateFrustum(pragma::CCameraComponent &cam, const Mat4 &matView, const Vector3 &dir)
{
	auto numCascades = GetSplitCount();
	for(auto i = decltype(numCascades) {0}; i < numCascades; ++i)
		UpdateFrustum(i, cam, matView, dir);
}

void CShadowCSMComponent::RenderBatch(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, pragma::CLightDirectionalComponent &light)
{
	// TODO
#if 0
	auto pLightComponent = light.GetEntity().GetComponent<pragma::CLightComponent>();
	auto *shadowScene = pLightComponent.valid() ? pLightComponent->FindShadowScene() : nullptr;
	auto &rt = GetStaticPendingRenderTarget();
	if(m_whShaderCsm.expired() || shadowScene == nullptr || rt == nullptr)
		return;
	auto &shaderCsm = static_cast<pragma::ShaderShadowCSM&>(*m_whShaderCsm.get());
	auto *shaderCsmTransparent = m_whShaderCsmTransparent.expired() == false ? static_cast<pragma::ShaderShadowCSMTransparent*>(m_whShaderCsmTransparent.get()) : nullptr;
	auto &tex = rt->GetTexture();
	auto &img = tex.GetImage();
	auto &info = m_pendingInfo;
	auto smType = pragma::CLightComponent::ShadowMapType::Static;

	auto &renderPass = rt->GetRenderPass();
	auto numLayers = GetLayerCount();
	for(auto layer=decltype(numLayers){0};layer<numLayers;++layer)
	{
		auto &framebuffer = *rt->GetFramebuffer(layer);

		auto bBlit = false;
		drawCmd->RecordImageBarrier(img,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal,layer);

		drawCmd->RecordBeginRenderPass(*rt,layer);
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
										auto matIdx = mdl->GetMaterialIndex(cSubMesh);
										auto *mat = matIdx.has_value() ? mdl->GetMaterial(*matIdx) : nullptr;
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
									auto matIdx = mdl->GetMaterialIndex(*cSubMesh);
									auto *mat = matIdx.has_value() ? mdl->GetMaterial(*matIdx) : nullptr;
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
		drawCmd->RecordEndRenderPass();
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
			if(ent->IsInert() == false || static_cast<CBaseEntity*>(ent)->IsInScene(*shadowScene) == false || pRenderComponent->ShouldDrawShadow(camPos) == false)
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

		auto &dstImg = GetDepthTexture(smType)->GetImage();

		drawCmd->RecordImageBarrier(img,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferSrcOptimal,layer);

		drawCmd->RecordImageBarrier(dstImg,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal,layer);

		// Blit finished shadow map into our actual scene shadow map
		prosper::util::BlitInfo blitInfo {};
		blitInfo.srcSubresourceLayer.baseArrayLayer = blitInfo.dstSubresourceLayer.baseArrayLayer = layer;
		blitInfo.srcSubresourceLayer.layerCount = blitInfo.dstSubresourceLayer.layerCount = 1u;
		drawCmd->RecordBlitImage(blitInfo,img,dstImg);

		drawCmd->RecordImageBarrier(img,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::TransferDstOptimal,layer);

		drawCmd->RecordImageBarrier(dstImg,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal,layer);

		// Update matrices and CSM-Buffer
		light.UpdateFrustum(layer);

		prosper::util::ClearImageInfo clearImageInfo {};
		clearImageInfo.subresourceRange.baseArrayLayer = layer;
		clearImageInfo.subresourceRange.layerCount = 1u;
		drawCmd->RecordClearImage(img,prosper::ImageLayout::TransferDstOptimal,1.f,clearImageInfo); // Clear layer

		drawCmd->RecordImageBarrier(img,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal,layer);
	}
#endif
}
