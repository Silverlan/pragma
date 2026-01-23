// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.renderer;
import :engine;
import :entities.components.pp_bloom;
import :entities.components.pp_dof;
import :entities.components.pp_fog;
import :entities.components.pp_fxaa;
import :entities.components.pp_tone_mapping;
import :scripting.lua.libraries.vulkan;

using namespace pragma;

ComponentEventId cRendererComponent::EVENT_RELOAD_RENDER_TARGET = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_RELOAD_BLOOM_RENDER_TARGET = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_BEGIN_RENDERING = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_END_RENDERING = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_UPDATE_CAMERA_DATA = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_UPDATE_RENDER_SETTINGS = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_UPDATE_RENDERER_BUFFER = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_GET_SCENE_TEXTURE = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_GET_PRESENTATION_TEXTURE = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_GET_HDR_PRESENTATION_TEXTURE = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_RECORD_COMMAND_BUFFERS = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_RENDER = INVALID_COMPONENT_ID;
ComponentEventId cRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED = INVALID_COMPONENT_ID;
void CRendererComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	cRendererComponent::EVENT_RELOAD_RENDER_TARGET = registerEvent("EVENT_RELOAD_RENDER_TARGET", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_RELOAD_BLOOM_RENDER_TARGET = registerEvent("EVENT_RELOAD_BLOOM_RENDER_TARGET", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_BEGIN_RENDERING = registerEvent("EVENT_BEGIN_RENDERING", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_END_RENDERING = registerEvent("EVENT_END_RENDERING", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_UPDATE_CAMERA_DATA = registerEvent("EVENT_UPDATE_CAMERA_DATA", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_UPDATE_RENDER_SETTINGS = registerEvent("EVENT_UPDATE_RENDER_SETTINGS", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_UPDATE_RENDERER_BUFFER = registerEvent("EVENT_UPDATE_RENDERER_BUFFER", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_GET_SCENE_TEXTURE = registerEvent("EVENT_GET_SCENE_TEXTURE", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_GET_PRESENTATION_TEXTURE = registerEvent("EVENT_GET_PRESENTATION_TEXTURE", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_GET_HDR_PRESENTATION_TEXTURE = registerEvent("EVENT_GET_HDR_PRESENTATION_TEXTURE", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_RECORD_COMMAND_BUFFERS = registerEvent("EVENT_RECORD_COMMAND_BUFFERS", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_RENDER = registerEvent("EVENT_RENDER", ComponentEventInfo::Type::Explicit);
	cRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED = registerEvent("EVENT_ON_RENDER_TARGET_RELOADED", ComponentEventInfo::Type::Explicit);
}

void CRendererComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<CRendererPpFogComponent>();
	GetEntity().AddComponent<CRendererPpDoFComponent>();
	GetEntity().AddComponent<CRendererPpBloomComponent>();
	GetEntity().AddComponent<CRendererPpToneMappingComponent>();
	GetEntity().AddComponent<CRendererPpFxaaComponent>();
}
void CRendererComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CRendererComponent::UpdateRenderSettings() { InvokeEventCallbacks(cRendererComponent::EVENT_UPDATE_RENDER_SETTINGS); }

void CRendererComponent::UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	CEUpdateRendererBuffer evData {drawCmd};
	InvokeEventCallbacks(cRendererComponent::EVENT_UPDATE_RENDERER_BUFFER, evData);
}

void CRendererComponent::UpdateCameraData(CSceneComponent &scene, CameraData &cameraData)
{
	CEUpdateCameraData evData {scene, cameraData};
	InvokeEventCallbacks(cRendererComponent::EVENT_UPDATE_CAMERA_DATA, evData);
}

void CRendererComponent::RecordCommandBuffers(const rendering::DrawSceneInfo &drawSceneInfo)
{
	CEDrawSceneInfo evData {drawSceneInfo};
	InvokeEventCallbacks(cRendererComponent::EVENT_RECORD_COMMAND_BUFFERS, evData);
}
void CRendererComponent::Render(const rendering::DrawSceneInfo &drawSceneInfo)
{
	BeginRendering(drawSceneInfo);
	CERender evData {drawSceneInfo};
	InvokeEventCallbacks(cRendererComponent::EVENT_RENDER, evData);
	EndRendering();
}

prosper::Texture *CRendererComponent::GetSceneTexture()
{
	CEGetSceneTexture evData {};
	InvokeEventCallbacks(cRendererComponent::EVENT_GET_SCENE_TEXTURE, evData);
	return evData.resultTexture;
}
prosper::Texture *CRendererComponent::GetPresentationTexture()
{
	CEGetPresentationTexture evData {};
	InvokeEventCallbacks(cRendererComponent::EVENT_GET_PRESENTATION_TEXTURE, evData);
	return evData.resultTexture;
}
prosper::Texture *CRendererComponent::GetHDRPresentationTexture()
{
	CEGetHdrPresentationTexture evData {};
	InvokeEventCallbacks(cRendererComponent::EVENT_GET_HDR_PRESENTATION_TEXTURE, evData);
	return evData.resultTexture;
}

bool CRendererComponent::ReloadRenderTarget(CSceneComponent &scene, uint32_t width, uint32_t height)
{
	auto oldWidth = m_width;
	auto oldHeight = m_height;
	m_width = width;
	m_height = height;
	CEReloadRenderTarget evData {scene, width, height};
	InvokeEventCallbacks(cRendererComponent::EVENT_RELOAD_RENDER_TARGET, evData);
	if(!evData.resultSuccess) {
		m_width = oldWidth;
		m_height = oldHeight;
	}
	CEOnRenderTargetReloaded evDataReloaded {evData.resultSuccess};
	InvokeEventCallbacks(cRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED, evDataReloaded);
	return evData.resultSuccess;
}

bool CRendererComponent::ReloadBloomRenderTarget(uint32_t width)
{
	CEReloadBloomRenderTarget evData {width};
	InvokeEventCallbacks(cRendererComponent::EVENT_RELOAD_BLOOM_RENDER_TARGET, evData);
	return evData.resultSuccess;
}

CallbackHandle CRendererComponent::AddPostProcessingEffect(const std::string &name, const std::function<void(const rendering::DrawSceneInfo &)> &render, uint32_t weight, const std::function<PostProcessingEffectData::Flags()> &fGetFlags)
{
	auto cb = FunctionCallback<void, const rendering::DrawSceneInfo &>::Create(render);
	PostProcessingEffectData effectData {};
	effectData.name = name;
	effectData.render = cb;
	effectData.weight = weight;
	effectData.getFlags = fGetFlags;
	for(auto it = m_postProcessingEffects.begin(); it != m_postProcessingEffects.end(); ++it) {
		auto &effectDataOther = *it;
		if(weight <= effectDataOther.weight) {
			m_postProcessingEffects.insert(it, effectData);
			return cb;
		}
	}
	m_postProcessingEffects.push_back(effectData);
	return cb;
}

void CRendererComponent::RemovePostProcessingEffect(const std::string &name)
{
	auto it = std::find_if(m_postProcessingEffects.begin(), m_postProcessingEffects.end(), [&name](const PostProcessingEffectData &data) { return data.name == name; });
	if(it == m_postProcessingEffects.end())
		return;
	m_postProcessingEffects.erase(it);
}

const std::vector<PostProcessingEffectData> &CRendererComponent::GetPostProcessingEffects() const { return m_postProcessingEffects; }

void CRendererComponent::EndRendering() { InvokeEventCallbacks(cRendererComponent::EVENT_END_RENDERING); }
void CRendererComponent::BeginRendering(const rendering::DrawSceneInfo &drawSceneInfo)
{
	const_cast<CSceneComponent *>(drawSceneInfo.scene.get())->UpdateBuffers(drawSceneInfo.commandBuffer);
	InvokeEventCallbacks(cRendererComponent::EVENT_BEGIN_RENDERING);
}

////////////

CEReloadRenderTarget::CEReloadRenderTarget(CSceneComponent &scene, uint32_t width, uint32_t height) : ComponentEvent {}, scene {scene}, width {width}, height {height} {}

void CEReloadRenderTarget::PushArguments(lua::State *l)
{
	scene.PushLuaObject(l);
	Lua::PushInt(l, width);
	Lua::PushInt(l, height);
}

void CEReloadRenderTarget::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		resultSuccess = Lua::CheckBool(l, -1);
}

////////////

CEBeginRendering::CEBeginRendering(const rendering::DrawSceneInfo &drawSceneInfo) : ComponentEvent {}, drawSceneInfo {drawSceneInfo} {}
void CEBeginRendering::PushArguments(lua::State *l) { Lua::Push<const rendering::DrawSceneInfo *>(l, &drawSceneInfo); }

////////////

CEReloadBloomRenderTarget::CEReloadBloomRenderTarget(uint32_t width) : ComponentEvent {}, width {width} {}
void CEReloadBloomRenderTarget::PushArguments(lua::State *l) { Lua::PushInt(l, width); }
void CEReloadBloomRenderTarget::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		resultSuccess = Lua::CheckBool(l, -1);
}

////////////

CEUpdateCameraData::CEUpdateCameraData(CSceneComponent &scene, CameraData &cameraData) : scene {scene}, cameraData {cameraData} {}

////////////

void CEGetSceneTexture::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		resultTexture = &Lua::Check<prosper::Texture>(l, -1);
}

////////////

CERender::CERender(const rendering::DrawSceneInfo &drawSceneInfo) : drawSceneInfo {drawSceneInfo} {}
void CERender::PushArguments(lua::State *l) { Lua::Push<const rendering::DrawSceneInfo *>(l, &drawSceneInfo); }

////////////

CEUpdateRendererBuffer::CEUpdateRendererBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCommandBuffer) : drawCommandBuffer {drawCommandBuffer} {}
void CEUpdateRendererBuffer::PushArguments(lua::State *l) { Lua::Push<std::shared_ptr<Lua::Vulkan::CommandBuffer>>(l, std::static_pointer_cast<Lua::Vulkan::CommandBuffer>(drawCommandBuffer)); }

////////////

CEOnRenderTargetReloaded::CEOnRenderTargetReloaded(bool success) : success {success} {}
void CEOnRenderTargetReloaded::PushArguments(lua::State *l) { Lua::PushBool(l, success); }
