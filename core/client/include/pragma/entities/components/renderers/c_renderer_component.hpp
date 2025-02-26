/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_COMPONENT_HPP__
#define __C_RENDERER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured_uniform_data.hpp"
#include <pragma/entities/components/base_entity_component.hpp>
#include <unordered_set>

namespace pragma {
	struct DLLCLIENT PostProcessingEffectData {
		enum class Flags : uint32_t { None = 0, ToneMapped = 1 };
		std::string name;
		uint32_t weight;
		mutable CallbackHandle render;
		std::function<PostProcessingEffectData::Flags()> getFlags = nullptr;
	};
	class DLLCLIENT CRendererComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_RELOAD_RENDER_TARGET;
		static ComponentEventId EVENT_RELOAD_BLOOM_RENDER_TARGET;
		static ComponentEventId EVENT_BEGIN_RENDERING;
		static ComponentEventId EVENT_END_RENDERING;
		static ComponentEventId EVENT_UPDATE_CAMERA_DATA;
		static ComponentEventId EVENT_UPDATE_RENDER_SETTINGS;
		static ComponentEventId EVENT_UPDATE_RENDERER_BUFFER;
		static ComponentEventId EVENT_GET_SCENE_TEXTURE;
		static ComponentEventId EVENT_GET_PRESENTATION_TEXTURE;
		static ComponentEventId EVENT_GET_HDR_PRESENTATION_TEXTURE;
		static ComponentEventId EVENT_RECORD_COMMAND_BUFFERS;
		static ComponentEventId EVENT_RENDER;
		static ComponentEventId EVENT_ON_RENDER_TARGET_RELOADED;

		enum class StandardPostProcessingWeight : uint32_t { Fog = 100'000, MotionBlur = 200'000, DoF = 300'000, Bloom = 400'000, ToneMapping = 500'000, Fxaa = 600'000 };

		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CRendererComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		void RecordCommandBuffers(const util::DrawSceneInfo &drawSceneInfo);
		void Render(const util::DrawSceneInfo &drawSceneInfo);

		CallbackHandle AddPostProcessingEffect(const std::string &name, const std::function<void(const util::DrawSceneInfo &)> &render, uint32_t weight, const std::function<PostProcessingEffectData::Flags()> &fGetFlags = nullptr);
		void RemovePostProcessingEffect(const std::string &name);
		const std::vector<PostProcessingEffectData> &GetPostProcessingEffects() const;

		bool ReloadRenderTarget(pragma::CSceneComponent &scene, uint32_t width, uint32_t height);

		void EndRendering();
		void BeginRendering(const util::DrawSceneInfo &drawSceneInfo);
		bool ReloadBloomRenderTarget(uint32_t width);

		prosper::Texture *GetSceneTexture();
		prosper::Texture *GetPresentationTexture();
		prosper::Texture *GetHDRPresentationTexture();

		void UpdateRenderSettings();
		void UpdateCameraData(pragma::CSceneComponent &scene, pragma::CameraData &cameraData);
		void UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }
	  private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		std::vector<PostProcessingEffectData> m_postProcessingEffects;
	};

	struct DLLCLIENT CEReloadRenderTarget : public ComponentEvent {
		CEReloadRenderTarget(pragma::CSceneComponent &scene, uint32_t width, uint32_t height);
		virtual void PushArguments(lua_State *l) override;
		virtual void HandleReturnValues(lua_State *l) override;
		pragma::CSceneComponent &scene;
		uint32_t width;
		uint32_t height;

		bool resultSuccess = false;
	};

	struct DLLCLIENT CEReloadBloomRenderTarget : public ComponentEvent {
		CEReloadBloomRenderTarget(uint32_t width);
		virtual void PushArguments(lua_State *l) override;
		virtual void HandleReturnValues(lua_State *l) override;
		uint32_t width;

		bool resultSuccess = false;
	};

	struct DLLCLIENT CEBeginRendering : public ComponentEvent {
		CEBeginRendering(const util::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua_State *l) override;
		const util::DrawSceneInfo &drawSceneInfo;
	};

	struct DLLCLIENT CEUpdateCameraData : public ComponentEvent {
		CEUpdateCameraData(pragma::CSceneComponent &scene, pragma::CameraData &cameraData);
		virtual void PushArguments(lua_State *l) override {}
		pragma::CSceneComponent &scene;
		pragma::CameraData &cameraData;
	};

	struct DLLCLIENT CEGetSceneTexture : public ComponentEvent {
		CEGetSceneTexture() = default;
		virtual void PushArguments(lua_State *l) override {}
		virtual void HandleReturnValues(lua_State *l) override;

		prosper::Texture *resultTexture = nullptr;
	};

	struct DLLCLIENT CERender : public ComponentEvent {
		CERender(const util::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua_State *l) override;
		const util::DrawSceneInfo &drawSceneInfo;
	};

	struct DLLCLIENT CEOnRenderTargetReloaded : public ComponentEvent {
		CEOnRenderTargetReloaded(bool success);
		virtual void PushArguments(lua_State *l) override;
		bool success;
	};

	struct DLLCLIENT CEUpdateRendererBuffer : public ComponentEvent {
		CEUpdateRendererBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCommandBuffer);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<prosper::IPrimaryCommandBuffer> drawCommandBuffer;
	};
	using CEGetPresentationTexture = CEGetSceneTexture;
	using CEGetHdrPresentationTexture = CEGetSceneTexture;
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::PostProcessingEffectData::Flags)

#endif
