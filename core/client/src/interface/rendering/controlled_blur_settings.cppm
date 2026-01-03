// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.controlled_blur_settings;

export import :rendering.bloom_pipeline_info;
export import pragma.prosper;

export namespace pragma::rendering {
	struct DLLCLIENT ControlledBlurSettings {
	  public:
		static constexpr uint32_t MAX_BLUR_RADIUS = 14;
		static constexpr double MAX_BLUR_SIGMA = 10.0;
		static constexpr uint32_t DEFAULT_RADIUS = 6;
		static constexpr double DEFAULT_SIGMA = 10;

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
		uint32_t m_radius = DEFAULT_RADIUS;
		double m_sigma = DEFAULT_SIGMA;
		std::shared_ptr<BloomPipelineInfo> m_bloomPipelineInfoH = nullptr;
		std::shared_ptr<BloomPipelineInfo> m_bloomPipelineInfoV = nullptr;
		int32_t m_blurAmount = -1;
		bool m_pipelineDirty = false;
	};
};
