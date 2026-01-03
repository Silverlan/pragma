// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.bloom_pipeline_info;

export import std.compat;

export namespace pragma::rendering {
	struct BloomPipelineInfo {
		BloomPipelineInfo() = delete;
		BloomPipelineInfo(uint32_t radius, double sigma);
		const uint32_t radius;
		const double sigma;
		std::optional<uint32_t> pipelineIdx;
	};
};
