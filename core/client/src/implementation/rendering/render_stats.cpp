// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.render_stats;
import :engine;

pragma::rendering::RenderStats::RenderStats()
{
	auto numTimers = math::to_integral(RenderStage::GpuCount) + math::to_integral(RenderPassStats::Timer::GpuCount) * passes.size();
	queryPool = get_cengine()->GetRenderContext().CreateQueryPool(prosper::QueryType::Timestamp, numTimers * 2);
	if(queryPool) {
		stageTimes.SetGpuTimerStart(RenderStage::GpuStart);
		stageTimes.gpuTimers.resize(math::to_integral(RenderStage::GpuCount));
		for(auto &timer : stageTimes.gpuTimers)
			timer = queryPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
		for(auto &pass : passes) {
			pass->SetGpuTimerStart(RenderPassStats::Timer::GpuStart);
			pass->gpuTimers.resize(math::to_integral(RenderPassStats::Timer::GpuCount));
			for(auto &timer : pass->gpuTimers)
				timer = queryPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
		}
	}
}

pragma::rendering::RenderStats::~RenderStats()
{
	if(queryPool)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(queryPool);
}
