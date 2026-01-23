// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.gpu_profiler;
import :engine;

using namespace pragma::debug;

static auto cvTimerQueries = pragma::console::get_client_con_var("cl_gpu_timer_queries_enabled");

std::shared_ptr<GPUProfilingStage> GPUProfilingStage::Create(Profiler &profiler, std::thread::id tid, const std::string &name, prosper::PipelineStageFlags stage)
{
	auto result = ProfilingStage::Create<GPUProfilingStage>(profiler, tid, name);
	if(result == nullptr)
		return nullptr;
	result->m_stage = stage;
	return result;
}

GPUProfiler &GPUProfilingStage::GetProfiler() { return static_cast<GPUProfiler &>(ProfilingStage::GetProfiler()); }
GPUProfilingStage *GPUProfilingStage::GetParent() { return static_cast<GPUProfilingStage *>(ProfilingStage::GetParent()); }
const GPUSwapchainTimer &GPUProfilingStage::GetTimer() const { return const_cast<GPUProfilingStage *>(this)->GetTimer(); }
GPUSwapchainTimer &GPUProfilingStage::GetTimer() { return static_cast<GPUSwapchainTimer &>(ProfilingStage::GetTimer()); }
prosper::PipelineStageFlags GPUProfilingStage::GetPipelineStage() const { return m_stage; }

GPUProfilingStage::GPUProfilingStage(Profiler &profiler, std::thread::id tid, const std::string &name) : ProfilingStage {profiler, tid, name} { m_stage = prosper::PipelineStageFlags::BottomOfPipeBit; }
void GPUProfilingStage::InitializeTimer() { m_timer = GetProfiler().CreateTimer(m_stage); }

GPUProfiler::GPUProfiler() : m_timerQueryPool {nullptr}, m_statsQueryPool {nullptr} { InitializeQueries(); }
void GPUProfiler::Initialize()
{
	if(m_timerQueryPool == nullptr || m_statsQueryPool == nullptr)
		return;
	m_rootStage = GPUProfilingStage::Create(*this, {}, "root", prosper::PipelineStageFlags::None);
}
std::shared_ptr<pragma::debug::Timer> GPUProfiler::CreateTimer(prosper::PipelineStageFlags stage) { return GPUSwapchainTimer::Create(*m_timerQueryPool, *m_statsQueryPool, stage); }
void GPUProfiler::Reset()
{
	for(auto &wpStage : GetStages()) {
		if(wpStage.expired())
			continue;
		static_cast<GPUProfilingStage *>(wpStage.lock().get())->GetTimer().Reset();
	}
}
void GPUProfiler::InitializeQueries()
{
	if(m_timerQueryPool != nullptr || m_statsQueryPool != nullptr)
		return;
	auto swapchainImageCount = get_cengine()->GetRenderContext().GetPrimaryWindowSwapchainImageCount();
	const auto maxTimestampQueryCount = 200u; // Note: Every timer requires 2 timestamps
	const auto maxStatisticsQueryCount = 100u;
	m_timerQueryPool = get_cengine()->GetRenderContext().CreateQueryPool(prosper::QueryType::Timestamp, maxTimestampQueryCount);
	m_statsQueryPool = get_cengine()->GetRenderContext().CreateQueryPool(prosper::QueryPipelineStatisticFlags::InputAssemblyVerticesBit | prosper::QueryPipelineStatisticFlags::InputAssemblyPrimitivesBit | prosper::QueryPipelineStatisticFlags::VertexShaderInvocationsBit
	    | prosper::QueryPipelineStatisticFlags::GeometryShaderInvocationsBit | prosper::QueryPipelineStatisticFlags::GeometryShaderPrimitivesBit | prosper::QueryPipelineStatisticFlags::ClippingInvocationsBit | prosper::QueryPipelineStatisticFlags::ClippingPrimitivesBit
	    | prosper::QueryPipelineStatisticFlags::FragmentShaderInvocationsBit | prosper::QueryPipelineStatisticFlags::TessellationControlShaderPatchesBit | prosper::QueryPipelineStatisticFlags::TessellationEvaluationShaderInvocationsBit
	    | prosper::QueryPipelineStatisticFlags::ComputeShaderInvocationsBit,
	  maxStatisticsQueryCount);
}

/////////////////////////

static void cl_gpu_timer_queries_dump(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto extended = false;
	if(argv.empty() == false)
		extended = pragma::util::to_boolean(argv.at(0));
	Con::COUT << "-------- GPU-Profiler Query Results --------" << Con::endl;
	std::function<void(ProfilingStage &, const std::string &, bool)> fPrintResults = nullptr;
	fPrintResults = [&fPrintResults, extended](ProfilingStage &stage, const std::string &t, bool bRoot) {
		if(bRoot == false) {
			std::string sTime = "Pending";
			auto result = stage.GetResult();
			if(result && result->duration.has_value()) {
				auto t = pragma::util::clock::to_milliseconds(*result->duration);
				sTime = pragma::util::round_string(t, 2) + " ms";
				sTime += " (" + std::to_string(result->duration->count()) + " ns)";
			}
			Con::COUT << t << stage.GetName() << ": " << sTime;
			Con::COUT << " (" << prosper::util::to_string(static_cast<GPUProfilingStage &>(stage).GetPipelineStage()) << ")" << Con::endl;

			if(result && extended == true) {
				auto &statistics = static_cast<GPUProfilerResult &>(*result).statistics;
				if(statistics.has_value()) {
					auto &stats = *statistics;
					Con::COUT << t << "{" << Con::endl;
					Con::COUT << t << "\tInput Assembly Vertices: " << stats.inputAssemblyVertices << Con::endl;
					Con::COUT << t << "\tInput Assembly Primitives: " << stats.inputAssemblyPrimitives << Con::endl;
					Con::COUT << t << "\tVertex Shader Invocations: " << stats.vertexShaderInvocations << Con::endl;
					Con::COUT << t << "\tGeometry Shader Invocations: " << stats.geometryShaderInvocations << Con::endl;
					Con::COUT << t << "\tGeometry Shader Primitives: " << stats.geometryShaderPrimitives << Con::endl;
					Con::COUT << t << "\tClipping Invocations: " << stats.clippingInvocations << Con::endl;
					Con::COUT << t << "\tClipping Primitives: " << stats.clippingPrimitives << Con::endl;
					Con::COUT << t << "\tFragment Shader Invocations: " << stats.fragmentShaderInvocations << Con::endl;
					Con::COUT << t << "\tTessellation Control Shader Patches: " << stats.tessellationControlShaderPatches << Con::endl;
					Con::COUT << t << "\tTessellation Evaluation Shader Invocations: " << stats.tessellationEvaluationShaderInvocations << Con::endl;
					Con::COUT << t << "\tCompute Shader Invocations: " << stats.computeShaderInvocations << Con::endl;
					Con::COUT << t << "}" << Con::endl;
					Con::COUT << Con::endl;
				}
			}
		}
		for(auto &wpChild : stage.GetChildren()) {
			if(wpChild.expired())
				continue;
			fPrintResults(*wpChild.lock(), t + (bRoot ? "" : "\t"), false);
		}
	};
	auto &profiler = pragma::get_cengine()->GetGPUProfiler();
	fPrintResults(profiler.GetRootStage(), "", true);
	Con::COUT << "--------------------------------------------" << Con::endl;
}
namespace {
	auto UVN = pragma::console::client::register_command("cl_gpu_timer_queries_dump", &cl_gpu_timer_queries_dump, pragma::console::ConVarFlags::None, "Prints all timer query results to the console.");
}
