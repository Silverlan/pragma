// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.lights.shadow;

export import :entities.components.game_shadow_manager;

export namespace pragma {
	class DLLCLIENT LightShadowRenderer {
	  public:
		enum class RenderState { NoRenderRequired = 0, RenderRequiredOnChange, RenderRequired };
		LightShadowRenderer(CLightComponent &l);
		~LightShadowRenderer();

		void BuildRenderQueues(const rendering::DrawSceneInfo &drawSceneInfo);
		void Render(const rendering::DrawSceneInfo &drawSceneInfo);
		bool DoesRenderQueueRequireBuilding() const;
		bool IsRenderQueueComplete() const;

		RenderState GetRenderState() const { return m_renderState; }
		void SetRenderState(RenderState renderState) { m_renderState = renderState; }
	  private:
		void UpdateSceneCallbacks();

		std::vector<std::shared_ptr<rendering::RenderQueue>> m_renderQueues {};
		std::atomic<bool> m_renderQueuesComplete = false;
		std::vector<CallbackHandle> m_sceneCallbacks {};
		CallbackHandle m_cbOnSceneFlagsChanged {};
		CallbackHandle m_cbPreRenderScenes {};
		ComponentHandle<CLightComponent> m_hLight {};
		RenderState m_renderState = RenderState::NoRenderRequired;
		bool m_requiresRenderQueueUpdate = false;
	};

	class DLLCLIENT CShadowComponent final : public BaseEntityComponent {
	  public:
		enum class Type : uint8_t { Generic = 1, Cube };
		static prosper::IDescriptorSet *GetDescriptorSet();

		CShadowComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		void ReloadDepthTextures();
		Type GetType() const;
		uint32_t GetLayerCount() const;
		void SetType(Type type);
		bool ShouldUpdateLayer(uint32_t layerId) const;
		void SetTextureReloadCallback(const std::function<void(void)> &f);

		prosper::RenderTarget *GetDepthRenderTarget() const;
		prosper::Texture *GetDepthTexture() const;
		prosper::IRenderPass *GetRenderPass() const;
		prosper::IFramebuffer *GetFramebuffer(uint32_t layerId = 0u);

		bool IsDirty() const;
		void SetDirty(bool dirty);

		std::weak_ptr<CShadowManagerComponent::RenderTarget> RequestRenderTarget();
		void FreeRenderTarget();
		bool HasRenderTarget() const;
		virtual void InitializeLuaObject(lua::State *l) override;

		LightShadowRenderer &GetRenderer();
		const LightShadowRenderer &GetRenderer() const;

		void RenderShadows(const rendering::DrawSceneInfo &drawSceneInfo);
	  protected:
		void DestroyTextures();
		bool m_bDirty = true;
		Type m_type = Type::Generic;
		CShadowManagerComponent::RtHandle m_hRt = {};
		std::function<void(void)> m_onTexturesReloaded = nullptr;
		void InitializeDepthTextures(uint32_t size);

		std::unique_ptr<LightShadowRenderer> m_lightShadowRenderer = nullptr;
	};
};
