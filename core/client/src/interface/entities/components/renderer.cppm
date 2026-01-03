// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.renderer;

export import :entities.components.scene;
export import :rendering.draw_scene_info;

export namespace pragma {
	struct DLLCLIENT PostProcessingEffectData {
		enum class Flags : uint32_t { None = 0, ToneMapped = 1 };
		std::string name;
		uint32_t weight;
		mutable CallbackHandle render;
		std::function<Flags()> getFlags = nullptr;
	};
	namespace cRendererComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_RELOAD_RENDER_TARGET;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_RELOAD_BLOOM_RENDER_TARGET;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_BEGIN_RENDERING;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_END_RENDERING;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_UPDATE_CAMERA_DATA;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_UPDATE_RENDER_SETTINGS;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_UPDATE_RENDERER_BUFFER;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_GET_SCENE_TEXTURE;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_GET_PRESENTATION_TEXTURE;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_GET_HDR_PRESENTATION_TEXTURE;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_RECORD_COMMAND_BUFFERS;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_RENDER;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RENDER_TARGET_RELOADED;
	}
	class DLLCLIENT CRendererComponent final : public BaseEntityComponent {
	  public:
		enum class StandardPostProcessingWeight : uint32_t { Fog = 100'000, MotionBlur = 200'000, DoF = 300'000, Bloom = 400'000, ToneMapping = 500'000, Fxaa = 600'000 };

		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CRendererComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;

		void RecordCommandBuffers(const rendering::DrawSceneInfo &drawSceneInfo);
		void Render(const rendering::DrawSceneInfo &drawSceneInfo);

		CallbackHandle AddPostProcessingEffect(const std::string &name, const std::function<void(const rendering::DrawSceneInfo &)> &render, uint32_t weight, const std::function<PostProcessingEffectData::Flags()> &fGetFlags = nullptr);
		void RemovePostProcessingEffect(const std::string &name);
		const std::vector<PostProcessingEffectData> &GetPostProcessingEffects() const;

		bool ReloadRenderTarget(CSceneComponent &scene, uint32_t width, uint32_t height);

		void EndRendering();
		void BeginRendering(const rendering::DrawSceneInfo &drawSceneInfo);
		bool ReloadBloomRenderTarget(uint32_t width);

		prosper::Texture *GetSceneTexture();
		prosper::Texture *GetPresentationTexture();
		prosper::Texture *GetHDRPresentationTexture();

		void UpdateRenderSettings();
		void UpdateCameraData(CSceneComponent &scene, CameraData &cameraData);
		void UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		uint32_t GetWidth() const { return m_width; }
		uint32_t GetHeight() const { return m_height; }
	  private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		std::vector<PostProcessingEffectData> m_postProcessingEffects;
	};

	struct DLLCLIENT CEReloadRenderTarget : public ComponentEvent {
		CEReloadRenderTarget(CSceneComponent &scene, uint32_t width, uint32_t height);
		virtual void PushArguments(lua::State *l) override;
		virtual void HandleReturnValues(lua::State *l) override;
		CSceneComponent &scene;
		uint32_t width;
		uint32_t height;

		bool resultSuccess = false;
	};

	struct DLLCLIENT CEReloadBloomRenderTarget : public ComponentEvent {
		CEReloadBloomRenderTarget(uint32_t width);
		virtual void PushArguments(lua::State *l) override;
		virtual void HandleReturnValues(lua::State *l) override;
		uint32_t width;

		bool resultSuccess = false;
	};

	struct DLLCLIENT CEBeginRendering : public ComponentEvent {
		CEBeginRendering(const rendering::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua::State *l) override;
		const rendering::DrawSceneInfo &drawSceneInfo;
	};

	struct DLLCLIENT CEUpdateCameraData : public ComponentEvent {
		CEUpdateCameraData(CSceneComponent &scene, CameraData &cameraData);
		virtual void PushArguments(lua::State *l) override {}
		CSceneComponent &scene;
		CameraData &cameraData;
	};

	struct DLLCLIENT CEGetSceneTexture : public ComponentEvent {
		CEGetSceneTexture() = default;
		virtual void PushArguments(lua::State *l) override {}
		virtual void HandleReturnValues(lua::State *l) override;

		prosper::Texture *resultTexture = nullptr;
	};

	struct DLLCLIENT CERender : public ComponentEvent {
		CERender(const rendering::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua::State *l) override;
		const rendering::DrawSceneInfo &drawSceneInfo;
	};

	struct DLLCLIENT CEOnRenderTargetReloaded : public ComponentEvent {
		CEOnRenderTargetReloaded(bool success);
		virtual void PushArguments(lua::State *l) override;
		bool success;
	};

	struct DLLCLIENT CEUpdateRendererBuffer : public ComponentEvent {
		CEUpdateRendererBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCommandBuffer);
		virtual void PushArguments(lua::State *l) override;
		std::shared_ptr<prosper::IPrimaryCommandBuffer> drawCommandBuffer;
	};
	using CEGetPresentationTexture = CEGetSceneTexture;
	using CEGetHdrPresentationTexture = CEGetSceneTexture;

	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::PostProcessingEffectData::Flags)
}
