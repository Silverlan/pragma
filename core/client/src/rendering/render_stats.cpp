// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/rendering/render_stats.hpp"
#include <queries/prosper_query_pool.hpp>
#include <queries/prosper_timer_query.hpp>

extern DLLCLIENT CEngine *c_engine;

RenderStats::RenderStats()
{
	auto numTimers = umath::to_integral(RenderStage::GpuCount) + umath::to_integral(RenderPassStats::Timer::GpuCount) * passes.size();
	queryPool = c_engine->GetRenderContext().CreateQueryPool(prosper::QueryType::Timestamp, numTimers * 2);
	if(queryPool) {
		stageTimes.SetGpuTimerStart(RenderStage::GpuStart);
		stageTimes.gpuTimers.resize(umath::to_integral(RenderStage::GpuCount));
		for(auto &timer : stageTimes.gpuTimers)
			timer = queryPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
		for(auto &pass : passes) {
			pass->SetGpuTimerStart(RenderPassStats::Timer::GpuStart);
			pass->gpuTimers.resize(umath::to_integral(RenderPassStats::Timer::GpuCount));
			for(auto &timer : pass->gpuTimers)
				timer = queryPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
		}
	}
}

RenderStats::~RenderStats()
{
	if(queryPool)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(queryPool);
}
