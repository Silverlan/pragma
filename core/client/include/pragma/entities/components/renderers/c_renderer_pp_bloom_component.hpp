/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_PP_BLOOM_COMPONENT_HPP__
#define __C_RENDERER_PP_BLOOM_COMPONENT_HPP__

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_bloom_blur.hpp"

namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpBloomComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpBloomComponent(BaseEntity &ent);

		void SetBlurRadius(uint32_t radius);
		void SetBlurSigma(double sigma);
		uint32_t GetBlurRadius() const;
		double GetBlurSigma() const;

		void SetBloomThreshold(float threshold);
		float GetBloomThreshold() const;

		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override { return "bloom"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::Bloom); }

		virtual void OnTick(double dt) override;
	  private:
		void SetPipelineDirty();
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		uint32_t m_radius = ShaderPPBloomBlurBase::DEFAULT_RADIUS;
		double m_sigma = ShaderPPBloomBlurBase::DEFAULT_SIGMA;
		std::shared_ptr<BloomPipelineInfo> m_bloomPipelineInfoH = nullptr;
		std::shared_ptr<BloomPipelineInfo> m_bloomPipelineInfoV = nullptr;
		bool m_pipelineDirty = false;
		float m_bloomThreshold = 1.f;
	};
};

#endif
