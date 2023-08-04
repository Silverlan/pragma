/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_PP_GLOW_COMPONENT_HPP__
#define __C_RENDERER_PP_GLOW_COMPONENT_HPP__

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_bloom_blur.hpp"
#include "pragma/rendering/controlled_blur_settings.hpp"

namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpGlowComponent final : public CRendererPpBaseComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		CRendererPpGlowComponent(BaseEntity &ent);

		void SetBlurRadius(uint32_t radius);
		void SetBlurSigma(double sigma);
		uint32_t GetBlurRadius() const;
		double GetBlurSigma() const;

		void SetBlurAmount(int32_t blurAmount);
		int32_t GetBlurAmount() const;

		prosper::Texture &GetGlowTexture();

		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override { return "glow"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::Bloom); }

		virtual void OnTick(double dt) override;
	  private:
		void SetPipelineDirty();
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		void InitializeRenderTarget();
		void RecordGlowPass(const util::DrawSceneInfo &drawSceneInfo);
		void ExecuteGlowPass(const util::DrawSceneInfo &drawSceneInfo);
		std::shared_ptr<prosper::RenderTarget> m_glowRt;

		std::shared_ptr<prosper::RenderTarget> m_blurRt;
		std::shared_ptr<prosper::BlurSet> m_blurSet = nullptr;

		ControlledBlurSettings m_controlledBlurSettings;

		std::shared_ptr<prosper::ISwapCommandBufferGroup> m_glowCommandBufferGroup = nullptr;
	};
};

#endif
