// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :rendering.render_stats;
import :engine;

pragma::rendering::RenderStats::RenderStats()
{
	auto numTimers = pragma::math::to_integral(RenderStage::GpuCount) + pragma::math::to_integral(RenderPassStats::Timer::GpuCount) * passes.size();
	queryPool = pragma::get_cengine()->GetRenderContext().CreateQueryPool(prosper::QueryType::Timestamp, numTimers * 2);
	if(queryPool) {
		stageTimes.SetGpuTimerStart(RenderStage::GpuStart);
		stageTimes.gpuTimers.resize(pragma::math::to_integral(RenderStage::GpuCount));
		for(auto &timer : stageTimes.gpuTimers)
			timer = queryPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
		for(auto &pass : passes) {
			pass->SetGpuTimerStart(RenderPassStats::Timer::GpuStart);
			pass->gpuTimers.resize(pragma::math::to_integral(RenderPassStats::Timer::GpuCount));
			for(auto &timer : pass->gpuTimers)
				timer = queryPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
		}
	}
}

pragma::rendering::RenderStats::~RenderStats()
{
	if(queryPool)
		pragma::get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(queryPool);
}
