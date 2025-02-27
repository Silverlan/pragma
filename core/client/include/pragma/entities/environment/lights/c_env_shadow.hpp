/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_SHADOW_HPP__
#define __C_ENV_SHADOW_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/rendering/render_queue.hpp"
#include <pragma/entities/components/base_entity_component.hpp>
#include <pragma/indexmap.h>

namespace prosper {
	class Framebuffer;
	class RenderPass;
	class RenderTarget;
	class Texture;
};
namespace pragma {
	class DLLCLIENT LightShadowRenderer {
	  public:
		enum class RenderState { NoRenderRequired = 0, RenderRequiredOnChange, RenderRequired };
		LightShadowRenderer(CLightComponent &l);
		~LightShadowRenderer();

		void BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo);
		void Render(const util::DrawSceneInfo &drawSceneInfo);
		bool DoesRenderQueueRequireBuilding() const;
		bool IsRenderQueueComplete() const;

		RenderState GetRenderState() const { return m_renderState; }
		void SetRenderState(RenderState renderState) { m_renderState = renderState; }
	  private:
		void UpdateSceneCallbacks();

		std::vector<std::shared_ptr<pragma::rendering::RenderQueue>> m_renderQueues {};
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

		CShadowComponent(BaseEntity &ent);
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
		virtual void InitializeLuaObject(lua_State *l) override;

		LightShadowRenderer &GetRenderer();
		const LightShadowRenderer &GetRenderer() const;

		void RenderShadows(const util::DrawSceneInfo &drawSceneInfo);
	  protected:
		void DestroyTextures();
		bool m_bDirty = true;
		Type m_type = CShadowComponent::Type::Generic;
		CShadowManagerComponent::RtHandle m_hRt = {};
		std::function<void(void)> m_onTexturesReloaded = nullptr;
		void InitializeDepthTextures(uint32_t size);

		std::unique_ptr<LightShadowRenderer> m_lightShadowRenderer = nullptr;
	};
};

#endif
