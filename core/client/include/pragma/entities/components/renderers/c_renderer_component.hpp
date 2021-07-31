/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_COMPONENT_HPP__
#define __C_RENDERER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <unordered_set>

namespace pragma
{
	class DLLCLIENT CRendererComponent final
		: public BaseEntityComponent
	{
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
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CRendererComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;

		void RecordCommandBuffers(const util::DrawSceneInfo &drawSceneInfo);
		void Render(const util::DrawSceneInfo &drawSceneInfo);

		bool ReloadRenderTarget(pragma::CSceneComponent &scene,uint32_t width,uint32_t height);

		void EndRendering();
		void BeginRendering(const util::DrawSceneInfo &drawSceneInfo);
		bool ReloadBloomRenderTarget(uint32_t width);

		prosper::Texture *GetSceneTexture();
		prosper::Texture *GetPresentationTexture();
		prosper::Texture *GetHDRPresentationTexture();
		
		void UpdateRenderSettings();
		void UpdateCameraData(pragma::CSceneComponent &scene,pragma::CameraData &cameraData);
		void UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		uint32_t GetWidth() const {return m_width;}
		uint32_t GetHeight() const {return m_height;}
	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
	};

	struct DLLCLIENT CEReloadRenderTarget
		: public ComponentEvent
	{
		CEReloadRenderTarget(pragma::CSceneComponent &scene,uint32_t width,uint32_t height);
		virtual void PushArguments(lua_State *l) override;
		virtual void HandleReturnValues(lua_State *l) override;
		pragma::CSceneComponent &scene;
		uint32_t width;
		uint32_t height;

		bool resultSuccess = false;
	};

	struct DLLCLIENT CEReloadBloomRenderTarget
		: public ComponentEvent
	{
		CEReloadBloomRenderTarget(uint32_t width);
		virtual void PushArguments(lua_State *l) override;
		virtual void HandleReturnValues(lua_State *l) override;
		uint32_t width;

		bool resultSuccess = false;
	};

	struct DLLCLIENT CEBeginRendering
		: public ComponentEvent
	{
		CEBeginRendering(const util::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua_State *l) override;
		const util::DrawSceneInfo &drawSceneInfo;
	};

	struct DLLCLIENT CEUpdateCameraData
		: public ComponentEvent
	{
		CEUpdateCameraData(pragma::CSceneComponent &scene,pragma::CameraData &cameraData);
		virtual void PushArguments(lua_State *l) override {}
		pragma::CSceneComponent &scene;
		pragma::CameraData &cameraData;
	};

	struct DLLCLIENT CEGetSceneTexture
		: public ComponentEvent
	{
		CEGetSceneTexture()=default;
		virtual void PushArguments(lua_State *l) override {}
		virtual void HandleReturnValues(lua_State *l) override;
		
		prosper::Texture *resultTexture = nullptr;
	};

	struct DLLCLIENT CERender
		: public ComponentEvent
	{
		CERender(const util::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua_State *l) override;
		const util::DrawSceneInfo &drawSceneInfo;
	};

	struct DLLCLIENT CEUpdateRendererBuffer
		: public ComponentEvent
	{
		CEUpdateRendererBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCommandBuffer);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<prosper::IPrimaryCommandBuffer> drawCommandBuffer;
	};
	using CEGetPresentationTexture = CEGetSceneTexture;
	using CEGetHdrPresentationTexture = CEGetSceneTexture;
};

#endif
