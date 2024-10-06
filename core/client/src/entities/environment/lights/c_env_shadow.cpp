/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/rendering/render_queue_instancer.hpp"
#include "pragma/rendering/render_queue_worker.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_framebuffer.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/console/c_cvar.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/math/intersection.h>
#include <image/prosper_render_target.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

CShadowComponent::CShadowComponent(BaseEntity &ent) : BaseEntityComponent {ent} {}

void CShadowComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	ReloadDepthTextures();
	auto lightC = GetEntity().GetComponent<CLightComponent>();
	if(lightC.valid())
		m_lightShadowRenderer = std::make_unique<LightShadowRenderer>(*lightC);
}

static void reload_all_shadow_maps()
{
	if(c_game == nullptr)
		return;
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CShadowComponent>>();
	for(auto *ent : entIt) {
		auto shadowC = ent->GetComponent<pragma::CShadowComponent>();
		shadowC->ReloadDepthTextures();
	}
}

static void cmd_render_shadow_quality(NetworkState *, const ConVar &, int, int quality)
{
	reload_all_shadow_maps();
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
}
REGISTER_CONVAR_CALLBACK_CL(render_shadow_quality, cmd_render_shadow_quality);

REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_dynamic, [](NetworkState *, const ConVar &, bool, bool) { reload_all_shadow_maps(); });

prosper::IDescriptorSet *CShadowComponent::GetDescriptorSet()
{
	auto *shadowManager = CShadowManagerComponent::GetShadowManager();
	return shadowManager ? shadowManager->GetDescriptorSet() : nullptr;
}

void CShadowComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	DestroyTextures();
}

void CShadowComponent::SetTextureReloadCallback(const std::function<void(void)> &f) { m_onTexturesReloaded = f; }

void CShadowComponent::DestroyTextures()
{
	if(m_hRt.expired())
		return;
	auto *shadowManager = CShadowManagerComponent::GetShadowManager();
	if(shadowManager)
		shadowManager->FreeRenderTarget(*m_hRt->lock());
}

CShadowComponent::Type CShadowComponent::GetType() const { return m_type; }
void CShadowComponent::SetType(Type type) { m_type = type; }
uint32_t CShadowComponent::GetLayerCount() const
{
	switch(m_type) {
	case Type::Generic:
		return 1;
	case Type::Cube:
		return 6;
	}
	return 0;
}

void CShadowComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

LightShadowRenderer &CShadowComponent::GetRenderer() { return *m_lightShadowRenderer; }
const LightShadowRenderer &CShadowComponent::GetRenderer() const { return const_cast<CShadowComponent *>(this)->GetRenderer(); }

static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
static CVar cvShadowQuality = GetClientConVar("render_shadow_quality");
void CShadowComponent::ReloadDepthTextures()
{
	//Scene::ClearLightCache();
	volatile util::ScopeGuard sg {[this]() {
		if(m_onTexturesReloaded == nullptr)
			return;
		m_onTexturesReloaded();
	}};
	if(cvShadowQuality->GetInt() <= 0)
		return;
	auto size = cvShadowmapSize->GetInt();
	if(size <= 0)
		return;
	InitializeDepthTextures(size);
}

void CShadowComponent::InitializeDepthTextures(uint32_t size) {}

std::weak_ptr<CShadowManagerComponent::RenderTarget> CShadowComponent::RequestRenderTarget()
{
	// Note: Render target are assigned by priority, which is simply the frame id it was last used.
	// That means the render targets for shadow maps that haven't been used in a while have a lower priority
	// and are more likely to be re-assigned if a new one is requested.

	auto *shadowManager = CShadowManagerComponent::GetShadowManager();
	if(shadowManager == nullptr)
		return std::weak_ptr<CShadowManagerComponent::RenderTarget> {};
	auto priority = c_engine->GetRenderContext().GetLastFrameId();
	if(m_hRt.valid()) {
		// We'll keep our current render target; Just update the priority
		shadowManager->UpdatePriority(*m_hRt->lock(), priority);
		return *m_hRt;
	}
	GetRenderer().SetRenderState(LightShadowRenderer::RenderState::RenderRequired);
	SetDirty(true);
	m_hRt = shadowManager->RequestRenderTarget((GetType() == CShadowComponent::Type::Cube) ? CShadowManagerComponent::Type::Cube : CShadowManagerComponent::Type::Generic, cvShadowmapSize->GetInt(), priority);
	return m_hRt.valid() ? *m_hRt : std::weak_ptr<CShadowManagerComponent::RenderTarget> {};
}
void CShadowComponent::FreeRenderTarget() { DestroyTextures(); }

bool CShadowComponent::IsDirty() const { return m_bDirty; }
void CShadowComponent::SetDirty(bool dirty) { m_bDirty = dirty; }

prosper::RenderTarget *CShadowComponent::GetDepthRenderTarget() const
{
	static std::shared_ptr<prosper::RenderTarget> nptr = nullptr;
	if(m_hRt.expired())
		return nullptr;
	return m_hRt->lock()->renderTarget.get();
}
prosper::Texture *CShadowComponent::GetDepthTexture() const
{
	auto rt = GetDepthRenderTarget();
	return rt ? &rt->GetTexture() : nullptr;
}
prosper::IRenderPass *CShadowComponent::GetRenderPass() const
{
	auto rt = GetDepthRenderTarget();
	return rt ? &rt->GetRenderPass() : nullptr;
}
prosper::IFramebuffer *CShadowComponent::GetFramebuffer(uint32_t layerId)
{
	auto rt = GetDepthRenderTarget();
	auto *fb = rt ? rt->GetFramebuffer(layerId) : nullptr;
	return fb ? fb : nullptr;
}
bool CShadowComponent::HasRenderTarget() const { return m_hRt.valid(); }

bool CShadowComponent::ShouldUpdateLayer(uint32_t) const { return true; }

void CShadowComponent::RenderShadows(const util::DrawSceneInfo &drawSceneInfo)
{
	if(m_lightShadowRenderer == nullptr)
		return;
	m_lightShadowRenderer->Render(drawSceneInfo);
}

//////////////

LightShadowRenderer::LightShadowRenderer(CLightComponent &l) : m_hLight {l.GetHandle<CLightComponent>()}
{
	auto &ent = static_cast<CBaseEntity &>(l.GetEntity());
	m_cbPreRenderScenes = c_game->AddCallback("PreRenderScenes", FunctionCallback<void>::Create([this]() {
		m_requiresRenderQueueUpdate = true;
		m_renderState = RenderState::RenderRequiredOnChange;
		for(auto &renderQueue : m_renderQueues)
			renderQueue->Clear();
	}));
	m_cbOnSceneFlagsChanged = m_hLight->BindEventUnhandled(CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED, [this, &ent](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateSceneCallbacks(); });
	// TODO: Render shadows AFTER prepass and BEFORE lighting pass
	UpdateSceneCallbacks();
}

LightShadowRenderer::~LightShadowRenderer()
{
	for(auto &renderQueue : m_renderQueues)
		renderQueue->WaitForCompletion();
	m_renderQueues.clear();
	if(m_cbOnSceneFlagsChanged.IsValid())
		m_cbOnSceneFlagsChanged.Remove();
	if(m_cbPreRenderScenes.IsValid())
		m_cbPreRenderScenes.Remove();
	for(auto &hCb : m_sceneCallbacks) {
		if(hCb.IsValid())
			hCb.Remove();
	}
}

void LightShadowRenderer::UpdateSceneCallbacks()
{
	if(m_hLight.expired())
		return;
	for(auto &hCb : m_sceneCallbacks) {
		if(hCb.IsValid())
			hCb.Remove();
	}
	m_sceneCallbacks.clear();
#if 0
	auto scenes = static_cast<CBaseEntity&>(m_hLight->GetEntity()).GetScenes();
	for(auto *scene : scenes)
	{
		scene->AddEventCallback(CSceneComponent::EVENT_ON_BUILD_RENDER_QUEUES,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			// TODO?
			/// TODO: Only if in list of culled light sources
			/// TODO: Seperate thread? (Or build AFTER prepass world)
			//BuildRenderQueues(static_cast<CEDrawSceneInfo&>(evData.get()).drawSceneInfo);
			return util::EventReply::Unhandled;
		});
		scene->AddEventCallback(CSceneComponent::EVENT_POST_RENDER_PREPASS,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			// TODO?
			/// TODO: Only if in list of culled light sources
			//Render(static_cast<CEDrawSceneInfo&>(evData.get()).drawSceneInfo);
			return util::EventReply::Unhandled;
		});
	}
#endif
}

static auto cvLodBias = GetClientConVar("cl_render_shadow_lod_bias");
static auto cvInstancingEnabled = GetClientConVar("render_instancing_enabled");
void LightShadowRenderer::BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo)
{
	if(m_hLight.expired())
		return;
	auto &light = *m_hLight;
	auto &ent = static_cast<CBaseEntity &>(light.GetEntity());
	auto &scene = *drawSceneInfo.scene;
	auto shadowC = m_hLight->GetShadowComponent();
	if(m_requiresRenderQueueUpdate == false || ent.IsInScene(scene) == false || shadowC == nullptr)
		return;

	auto *renderer = drawSceneInfo.scene->GetRenderer();
	auto hRasterizer = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	if(hRasterizer.expired())
		return;
	auto *rasterizer = hRasterizer.get();

	m_requiresRenderQueueUpdate = false;
	m_renderQueuesComplete = false;

	auto numLayers = shadowC->GetLayerCount();
	m_renderQueues.resize(numLayers);
	for(auto &renderQueue : m_renderQueues) {
		if(renderQueue == nullptr)
			renderQueue = rendering::RenderQueue::Create("shadow");
		renderQueue->Lock();
	}

	auto renderMask = drawSceneInfo.GetRenderMask(*c_game);
	// TODO: Use separate shadow queue builder thread
	auto lodBias = cvLodBias->GetInt();
	c_game->GetRenderQueueBuilder().Append(
	  [this, &drawSceneInfo, rasterizer, &scene, &light, &ent, lodBias, renderMask]() {
		  for(auto &renderQueue : m_renderQueues)
			  renderQueue->instanceSets.clear();
		  auto &mainRenderQueue = m_renderQueues.front();
		  auto &hCam = scene.GetActiveCamera();
		  if(hCam.valid()) {
			  // TODO: Directional lights?
			  auto &lightOrigin = ent.GetPosition();
			  auto radiusC = ent.GetComponent<CRadiusComponent>();
			  auto lightRadius = radiusC.valid() ? radiusC->GetRadius() : 0.f;
			  std::function<bool(const Vector3 &, const Vector3 &)> fShouldCull;
			  auto lightSpotC = ent.GetComponent<CLightSpotComponent>();
			  if(lightSpotC.valid()) {
				  auto coneDir = ent.GetForward();
				  auto coneAngle = umath::deg_to_rad(lightSpotC->GetOuterConeAngle() / 2.f);
				  fShouldCull = [&lightOrigin, lightRadius, coneDir, coneAngle](const Vector3 &min, const Vector3 &max) -> bool {
					  if(umath::intersection::aabb_sphere(min, max, lightOrigin, lightRadius) == false)
						  return true;
					  auto center = (min + max) / 2.f;
					  auto extents = (max - min) / 2.f;
					  auto radius = uvec::length(extents);
					  return !umath::intersection::sphere_cone(center, radius, lightOrigin, coneDir, coneAngle, lightRadius); // TODO: Frustum culling might be more efficient?
				  };
			  }
			  else
				  fShouldCull = [&lightOrigin, lightRadius](const Vector3 &min, const Vector3 &max) -> bool { return !umath::intersection::aabb_sphere(min, max, lightOrigin, lightRadius); };

			  auto &posCam = hCam->GetEntity().GetPosition();
			  auto vp = hCam->GetProjectionMatrix() * hCam->GetViewMatrix();
			  std::vector<util::BSPTree::Node *> bspLeafNodes;
			  EntityIterator entItWorld {*c_game};
			  entItWorld.AttachFilter<TEntityIteratorFilterComponent<pragma::CWorldComponent>>();
			  bspLeafNodes.reserve(entItWorld.GetCount());
			  for(auto *entWorld : entItWorld) {
				  if(SceneRenderDesc::ShouldConsiderEntity(*static_cast<CBaseEntity *>(entWorld), scene, drawSceneInfo.renderFlags, renderMask) == false)
					  continue;
				  auto worldC = entWorld->GetComponent<pragma::CWorldComponent>();
				  auto &bspTree = worldC->GetBSPTree();
				  auto *node = bspTree ? bspTree->FindLeafNode(posCam) : nullptr;
				  if(node == nullptr)
					  continue;
				  bspLeafNodes.push_back(node);

				  if(umath::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::Static) == false)
					  continue;

				  auto *renderC = static_cast<CBaseEntity &>(worldC->GetEntity()).GetRenderComponent();
				  if(renderC->ShouldDrawShadow() == false)
					  continue;
				  renderC->UpdateRenderDataMT(scene, *hCam, vp);

				  for(auto *renderQueue : {worldC->GetClusterRenderQueue(node->cluster), worldC->GetClusterRenderQueue(node->cluster, true)}) {
					  if(renderQueue == nullptr)
						  continue;
					  auto &pose = entWorld->GetPose();
					  for(auto i = decltype(renderQueue->queue.size()) {0u}; i < renderQueue->queue.size(); ++i) {
						  auto &item = renderQueue->queue.at(i);
						  if(SceneRenderDesc::ShouldCull(*renderC, item.mesh, fShouldCull))
							  continue;
						  mainRenderQueue->queue.push_back(item);
						  mainRenderQueue->sortedItemIndices.push_back(renderQueue->sortedItemIndices.at(i));
						  mainRenderQueue->sortedItemIndices.back().first = mainRenderQueue->queue.size() - 1;

						  auto &renderMeshes = renderC->GetRenderMeshes();
						  if(item.mesh >= renderMeshes.size())
							  continue;
						  auto pos = pose * renderMeshes.at(item.mesh)->GetCenter();
						  mainRenderQueue->queue.back().sortingKey.SetDistance(pos, *hCam);
					  }
				  }
			  }

			  auto fGetRenderQueue = [&mainRenderQueue](pragma::rendering::SceneRenderPass renderMode, bool translucent) -> pragma::rendering::RenderQueue * { return (renderMode == pragma::rendering::SceneRenderPass::World) ? mainRenderQueue.get() : nullptr; };
			  for(auto *pRenderComponent : pragma::CRenderComponent::GetEntitiesExemptFromOcclusionCulling()) {
				  if(SceneRenderDesc::ShouldConsiderEntity(static_cast<CBaseEntity &>(pRenderComponent->GetEntity()), scene, drawSceneInfo.renderFlags, renderMask) == false || pRenderComponent->ShouldDrawShadow() == false)
					  continue;
				  SceneRenderDesc::AddRenderMeshesToRenderQueue(rasterizer, drawSceneInfo.renderFlags, *pRenderComponent, fGetRenderQueue, scene, *hCam, vp, nullptr);
			  }

			  auto *culler = scene.FindOcclusionCuller();
			  if(culler) {
				  auto &dynOctree = culler->GetOcclusionOctree();
				  SceneRenderDesc::CollectRenderMeshesFromOctree(rasterizer, drawSceneInfo.renderFlags, drawSceneInfo.clipPlane.has_value(), dynOctree, scene, *hCam, vp, renderMask, fGetRenderQueue, fShouldCull, nullptr, nullptr, lodBias,
				    [](CBaseEntity &ent, const pragma::CSceneComponent &, RenderFlags) -> bool { return ent.GetRenderComponent()->ShouldDrawShadow(); });
			  }
		  }
	  },
	  [this, &drawSceneInfo, &ent]() {
		  c_game->GetRenderQueueWorkerManager().WaitForCompletion();
		  // Sorting is technically not necessary, we only use it to lower the number of material state changes (for translucent meshes)
		  auto &mainRenderQueue = m_renderQueues.front();
		  mainRenderQueue->Sort();

		  auto lightPointC = ent.GetComponent<CLightPointComponent>();
		  if(lightPointC.valid()) {
			  // The first render queue contains all shadow-casters within the light's radius,
			  // but we still need to cull for each cubemap direction. (The contents of the first
			  // render queue are copied into the others)
			  for(auto i = static_cast<int32_t>(m_renderQueues.size()) - 1; i >= 0; --i) {
				  auto &renderQueue = m_renderQueues.at(i);
				  if(i != 0) {
					  renderQueue->queue = mainRenderQueue->queue;
					  renderQueue->sortedItemIndices = mainRenderQueue->sortedItemIndices;
				  }

				  auto &planes = lightPointC->GetFrustumPlanes(static_cast<CubeMapSide>(i));
				  auto fShouldCull = [&planes](const Vector3 &min, const Vector3 &max) -> bool { return umath::intersection::aabb_in_plane_mesh(min, max, planes.begin(), planes.end()) == umath::intersection::Intersect::Outside; };
				  for(auto it = renderQueue->queue.begin(); it != renderQueue->queue.end();) {
					  auto &item = *it;
					  auto *ent = static_cast<CBaseEntity *>(c_game->GetEntityByLocalIndex(item.entity));
					  auto *renderC = ent->GetRenderComponent();
					  if(SceneRenderDesc::ShouldCull(*renderC, fShouldCull) == false || SceneRenderDesc::ShouldCull(*renderC, item.mesh, fShouldCull) == false) {
						  ++it;
						  continue;
					  }
					  renderQueue->sortedItemIndices.erase(renderQueue->sortedItemIndices.begin() + (it - renderQueue->queue.begin()));
					  it = renderQueue->queue.erase(it);
				  }
			  }
		  }

		// We'll generate a hash from the render queue and our transform matrix.
		// If the hash is the same as in the previous frame, we don't have to re-render.
		// TODO: This could be done more efficiently!
#if 0
		// TODO: What if object orientation changed?
		DataStream ds {};
		ds->SetOffset(0);
		auto &depthMVP = light.GetTransformationMatrix(0); // If first layer is the same as before, the others should be as well
		ds<<depthMVP;
		for(auto &item : m_renderQueue->sortedItemIndices)
			ds<<item.second;
		auto hash = util::murmur_hash3(ds->GetData(),ds->GetInternalSize(),0 /* seed */);
#endif
		  if(cvInstancingEnabled->GetBool()) {
			  for(auto &renderQueue : m_renderQueues) {
				  pragma::rendering::RenderQueueInstancer instancer {*renderQueue};
				  instancer.Process();
			  }
		  }

		  for(auto &renderQueue : m_renderQueues)
			  renderQueue->Unlock();
		  m_renderQueuesComplete = true;
	  });
}

bool LightShadowRenderer::DoesRenderQueueRequireBuilding() const { return m_requiresRenderQueueUpdate; }
bool LightShadowRenderer::IsRenderQueueComplete() const { return !m_requiresRenderQueueUpdate && m_renderQueuesComplete; }

void LightShadowRenderer::Render(const util::DrawSceneInfo &drawSceneInfo)
{
	if(m_renderState == RenderState::NoRenderRequired || m_hLight.expired())
		return;
	m_renderState = RenderState::NoRenderRequired;

	// TODO: Cleanup
	auto shadowC = m_hLight->GetShadowComponent();
	if(shadowC == nullptr || m_hLight->GetEffectiveShadowType() == pragma::BaseEnvLightComponent::ShadowType::None) // TODO: Clear render target?
		return;

	if(m_hLight->GetEntity().HasComponent<CLightDirectionalComponent>())
		return; // Directional lights are handled separately

	auto wpRt = shadowC->RequestRenderTarget();
	if(wpRt.expired() == true) {
		m_hLight->SetShadowMapIndex(std::numeric_limits<uint32_t>::max(), pragma::CLightComponent::ShadowMapType::Dynamic);
		return;
	}
	auto rt = wpRt.lock();
	m_hLight->SetShadowMapIndex(rt->index, pragma::CLightComponent::ShadowMapType::Dynamic);

	//auto &shader = (type != pragma::LightType::Spot) ? static_cast<pragma::ShaderShadow&>(*m_shader.get()) : static_cast<pragma::ShaderShadow&>(*m_shaderSpot.get());
	//pragma::ShaderShadowTransparent *shaderTransparent = nullptr;
	//if(type != pragma::LightType::Spot)
	//	shaderTransparent = static_cast<pragma::ShaderShadowTransparent*>(m_shaderTransparent.expired() == false ? m_shaderTransparent.get() : nullptr);
	//else
	//	shaderTransparent = static_cast<pragma::ShaderShadowTransparent*>(m_shaderSpotTransparent.expired() == false ? m_shaderSpotTransparent.get() : nullptr);

	// TODO
	auto *shader = static_cast<pragma::ShaderShadow *>(c_engine->GetShader("shadow").get());
	if(shader == nullptr)
		return;

	auto &light = *m_hLight;
	auto *smRt = shadowC->GetDepthRenderTarget();
	auto &tex = smRt->GetTexture();
	auto *scene = c_game->GetScene();
	auto *renderer = scene ? scene->GetRenderer() : nullptr;

	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &img = tex.GetImage();
	auto numLayers = shadowC->GetLayerCount();
	drawCmd->RecordImageBarrier(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

	for(auto layerId = decltype(numLayers) {0}; layerId < numLayers; ++layerId)
		CSceneComponent::UpdateRenderBuffers(drawCmd, *m_renderQueues.at(layerId), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::ShadowPass) : nullptr);

	auto pipeline = m_hLight->AreMorphTargetsInShadowsEnabled() ? ShaderShadow::Pipeline::WithMorphTargetAnimations : ShaderShadow::Pipeline::Default;

	util::RenderPassDrawInfo rpDrawInfo {drawSceneInfo, *drawSceneInfo.commandBuffer};
	rendering::DepthStageRenderProcessor shadowRenderProcessor {rpDrawInfo, {}};
	for(auto layerId = decltype(numLayers) {0}; layerId < numLayers; ++layerId) {
		auto *framebuffer = shadowC->GetFramebuffer(layerId);

		const prosper::ClearValue clearVal {prosper::ClearDepthStencilValue {1.f}};
		if(drawCmd->RecordBeginRenderPass(*smRt, layerId, prosper::IPrimaryCommandBuffer::RenderPassFlags::None, &clearVal) == false)
			continue;

		if(shadowRenderProcessor.BindShader(*shader, umath::to_integral(pipeline))) {
			shadowRenderProcessor.BindLight(*m_hLight, layerId);
			shadowRenderProcessor.Render(*m_renderQueues.at(layerId), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::ShadowPass) : nullptr);

			// TODO: Translucent render pass ?
			shadowRenderProcessor.UnbindShader();
		}

		// TODO: Particle shadows

		drawCmd->RecordEndRenderPass();

		prosper::util::ImageSubresourceRange range {layerId};
		drawCmd->RecordPostRenderPassImageBarrier(img, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, range);
	}
}
