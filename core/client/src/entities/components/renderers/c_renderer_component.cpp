/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_bloom_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_dof_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_fog_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_tone_mapping_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_fxaa_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

ComponentEventId CRendererComponent::EVENT_RELOAD_RENDER_TARGET = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_RELOAD_BLOOM_RENDER_TARGET = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_BEGIN_RENDERING = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_END_RENDERING = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_UPDATE_CAMERA_DATA = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_UPDATE_RENDER_SETTINGS = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_UPDATE_RENDERER_BUFFER = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_GET_SCENE_TEXTURE = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_GET_PRESENTATION_TEXTURE = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_GET_HDR_PRESENTATION_TEXTURE = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_RECORD_COMMAND_BUFFERS = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_RENDER = INVALID_COMPONENT_ID;
ComponentEventId CRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED = INVALID_COMPONENT_ID;
void CRendererComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_RELOAD_RENDER_TARGET = registerEvent("EVENT_RELOAD_RENDER_TARGET", ComponentEventInfo::Type::Explicit);
	EVENT_RELOAD_BLOOM_RENDER_TARGET = registerEvent("EVENT_RELOAD_BLOOM_RENDER_TARGET", ComponentEventInfo::Type::Explicit);
	EVENT_BEGIN_RENDERING = registerEvent("EVENT_BEGIN_RENDERING", ComponentEventInfo::Type::Explicit);
	EVENT_END_RENDERING = registerEvent("EVENT_END_RENDERING", ComponentEventInfo::Type::Explicit);
	EVENT_UPDATE_CAMERA_DATA = registerEvent("EVENT_UPDATE_CAMERA_DATA", ComponentEventInfo::Type::Explicit);
	EVENT_UPDATE_RENDER_SETTINGS = registerEvent("EVENT_UPDATE_RENDER_SETTINGS", ComponentEventInfo::Type::Explicit);
	EVENT_UPDATE_RENDERER_BUFFER = registerEvent("EVENT_UPDATE_RENDERER_BUFFER", ComponentEventInfo::Type::Explicit);
	EVENT_GET_SCENE_TEXTURE = registerEvent("EVENT_GET_SCENE_TEXTURE", ComponentEventInfo::Type::Explicit);
	EVENT_GET_PRESENTATION_TEXTURE = registerEvent("EVENT_GET_PRESENTATION_TEXTURE", ComponentEventInfo::Type::Explicit);
	EVENT_GET_HDR_PRESENTATION_TEXTURE = registerEvent("EVENT_GET_HDR_PRESENTATION_TEXTURE", ComponentEventInfo::Type::Explicit);
	EVENT_RECORD_COMMAND_BUFFERS = registerEvent("EVENT_RECORD_COMMAND_BUFFERS", ComponentEventInfo::Type::Explicit);
	EVENT_RENDER = registerEvent("EVENT_RENDER", ComponentEventInfo::Type::Explicit);
	EVENT_ON_RENDER_TARGET_RELOADED = registerEvent("EVENT_ON_RENDER_TARGET_RELOADED", ComponentEventInfo::Type::Explicit);
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
void CRendererComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CRendererComponent::UpdateRenderSettings() { InvokeEventCallbacks(EVENT_UPDATE_RENDER_SETTINGS); }

void CRendererComponent::UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	CEUpdateRendererBuffer evData {drawCmd};
	InvokeEventCallbacks(EVENT_UPDATE_RENDERER_BUFFER, evData);
}

void CRendererComponent::UpdateCameraData(pragma::CSceneComponent &scene, pragma::CameraData &cameraData)
{
	pragma::CEUpdateCameraData evData {scene, cameraData};
	InvokeEventCallbacks(EVENT_UPDATE_CAMERA_DATA, evData);
}

void CRendererComponent::RecordCommandBuffers(const util::DrawSceneInfo &drawSceneInfo)
{
	pragma::CEDrawSceneInfo evData {drawSceneInfo};
	InvokeEventCallbacks(EVENT_RECORD_COMMAND_BUFFERS, evData);
}
void CRendererComponent::Render(const util::DrawSceneInfo &drawSceneInfo)
{
	BeginRendering(drawSceneInfo);
	pragma::CERender evData {drawSceneInfo};
	InvokeEventCallbacks(EVENT_RENDER, evData);
	EndRendering();
}

prosper::Texture *CRendererComponent::GetSceneTexture()
{
	pragma::CEGetSceneTexture evData {};
	InvokeEventCallbacks(EVENT_GET_SCENE_TEXTURE, evData);
	return evData.resultTexture;
}
prosper::Texture *CRendererComponent::GetPresentationTexture()
{
	pragma::CEGetPresentationTexture evData {};
	InvokeEventCallbacks(EVENT_GET_PRESENTATION_TEXTURE, evData);
	return evData.resultTexture;
}
prosper::Texture *CRendererComponent::GetHDRPresentationTexture()
{
	pragma::CEGetHdrPresentationTexture evData {};
	InvokeEventCallbacks(EVENT_GET_HDR_PRESENTATION_TEXTURE, evData);
	return evData.resultTexture;
}

bool CRendererComponent::ReloadRenderTarget(pragma::CSceneComponent &scene, uint32_t width, uint32_t height)
{
	auto oldWidth = m_width;
	auto oldHeight = m_height;
	m_width = width;
	m_height = height;
	pragma::CEReloadRenderTarget evData {scene, width, height};
	InvokeEventCallbacks(EVENT_RELOAD_RENDER_TARGET, evData);
	if(!evData.resultSuccess) {
		m_width = oldWidth;
		m_height = oldHeight;
	}
	CEOnRenderTargetReloaded evDataReloaded {evData.resultSuccess};
	InvokeEventCallbacks(EVENT_ON_RENDER_TARGET_RELOADED, evDataReloaded);
	return evData.resultSuccess;
}

bool CRendererComponent::ReloadBloomRenderTarget(uint32_t width)
{
	pragma::CEReloadBloomRenderTarget evData {width};
	InvokeEventCallbacks(EVENT_RELOAD_BLOOM_RENDER_TARGET, evData);
	return evData.resultSuccess;
}

CallbackHandle CRendererComponent::AddPostProcessingEffect(const std::string &name, const std::function<void(const util::DrawSceneInfo &)> &render, uint32_t weight, const std::function<PostProcessingEffectData::Flags()> &fGetFlags)
{
	auto cb = FunctionCallback<void, const util::DrawSceneInfo &>::Create(render);
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

void CRendererComponent::EndRendering() { InvokeEventCallbacks(EVENT_END_RENDERING); }
void CRendererComponent::BeginRendering(const util::DrawSceneInfo &drawSceneInfo)
{
	const_cast<pragma::CSceneComponent *>(drawSceneInfo.scene.get())->UpdateBuffers(drawSceneInfo.commandBuffer);
	InvokeEventCallbacks(EVENT_BEGIN_RENDERING);
}

////////////

CEReloadRenderTarget::CEReloadRenderTarget(pragma::CSceneComponent &scene, uint32_t width, uint32_t height) : ComponentEvent {}, scene {scene}, width {width}, height {height} {}

void CEReloadRenderTarget::PushArguments(lua_State *l)
{
	scene.PushLuaObject(l);
	Lua::PushInt(l, width);
	Lua::PushInt(l, height);
}

void CEReloadRenderTarget::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		resultSuccess = Lua::CheckBool(l, -1);
}

////////////

CEBeginRendering::CEBeginRendering(const util::DrawSceneInfo &drawSceneInfo) : ComponentEvent {}, drawSceneInfo {drawSceneInfo} {}
void CEBeginRendering::PushArguments(lua_State *l) { Lua::Push<const util::DrawSceneInfo *>(l, &drawSceneInfo); }

////////////

CEReloadBloomRenderTarget::CEReloadBloomRenderTarget(uint32_t width) : ComponentEvent {}, width {width} {}
void CEReloadBloomRenderTarget::PushArguments(lua_State *l) { Lua::PushInt(l, width); }
void CEReloadBloomRenderTarget::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		resultSuccess = Lua::CheckBool(l, -1);
}

////////////

CEUpdateCameraData::CEUpdateCameraData(pragma::CSceneComponent &scene, pragma::CameraData &cameraData) : scene {scene}, cameraData {cameraData} {}

////////////

void CEGetSceneTexture::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		resultTexture = &Lua::Check<prosper::Texture>(l, -1);
}

////////////

CERender::CERender(const util::DrawSceneInfo &drawSceneInfo) : drawSceneInfo {drawSceneInfo} {}
void CERender::PushArguments(lua_State *l) { Lua::Push<const util::DrawSceneInfo *>(l, &drawSceneInfo); }

////////////

CEUpdateRendererBuffer::CEUpdateRendererBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCommandBuffer) : drawCommandBuffer {drawCommandBuffer} {}
void CEUpdateRendererBuffer::PushArguments(lua_State *l) { Lua::Push<std::shared_ptr<Lua::Vulkan::CommandBuffer>>(l, std::static_pointer_cast<Lua::Vulkan::CommandBuffer>(drawCommandBuffer)); }

////////////

CEOnRenderTargetReloaded::CEOnRenderTargetReloaded(bool success) : success {success} {}
void CEOnRenderTargetReloaded::PushArguments(lua_State *l) { Lua::PushBool(l, success); }
