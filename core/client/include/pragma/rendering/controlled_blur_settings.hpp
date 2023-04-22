/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#ifndef __CONTROLLED_BLUR_SETTINGS_HPP__
#define __CONTROLLED_BLUR_SETTINGS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_bloom_blur.hpp"
#include <memory>

namespace prosper {
	class BlurSet;
};

namespace pragma {
	struct BloomPipelineInfo;
	struct DLLCLIENT ControlledBlurSettings {
	  public:
		static constexpr uint32_t MAX_BLUR_RADIUS = 14;
		static constexpr double MAX_BLUR_SIGMA = 10.0;

		ControlledBlurSettings();
		bool IsValid() const;
		void RecordBlur(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmd, prosper::BlurSet &blurSet);
		void SetRadius(uint32_t radius);
		void SetSigma(double sigma);
		uint32_t GetRadius() const;
		double GetSigma() const;

		void SetShaderPipelineDirty();
		void UpdateShaderPipelines();

		void SetBlurAmount(int32_t blurAmount);
		int32_t GetBlurAmount() const;
	  private:
		uint32_t m_radius = ShaderPPBloomBlurBase::DEFAULT_RADIUS;
		double m_sigma = ShaderPPBloomBlurBase::DEFAULT_SIGMA;
		std::shared_ptr<BloomPipelineInfo> m_bloomPipelineInfoH = nullptr;
		std::shared_ptr<BloomPipelineInfo> m_bloomPipelineInfoV = nullptr;
		int32_t m_blurAmount = -1;
		bool m_pipelineDirty = false;
	};
};

#endif
