#include "stdafx_engine.h"
#include "pragma/util/profiling_stages.h"

std::string profiling_stage_to_string(ProfilingStage stage)
{
	switch(stage)
	{
		case ProfilingStage::EngineThink:
			return "[ENGINE] Think";
		case ProfilingStage::EngineTick:
			return "[ENGINE] Tick";
		case ProfilingStage::EngineDraw:
			return "[ENGINE] Draw";
		case ProfilingStage::PresentDraw:
			return "[ENGINE] Present Draw";
		case ProfilingStage::EngineLoop:
			return "[ENGINE] MainLoop";

		case ProfilingStage::ServerStateThink:
			return "[SERVER] Think";
		case ProfilingStage::ServerStateTick:
			return "[SERVER] Tick";
		case ProfilingStage::ServerStatePhysicsUpdate:
			return "[SERVER] Physics Update";
		case ProfilingStage::ServerStatePhysicsSimulate:
			return "[SERVER] Physics Simulate";
		case ProfilingStage::ServerStateSoundUpdate:
			return "[SERVER] Sound Update";
		case ProfilingStage::ServerStateEntityThink:
			return "[SERVER] Entity Think";

		case ProfilingStage::ClientStateThink:
			return "[CLIENT] Think";
		case ProfilingStage::ClientStateTick:
			return "[CLIENT] Tick";
		case ProfilingStage::ClientStateDraw:
			return "[CLIENT] Draw";
		case ProfilingStage::ClientStatePhysicsUpdate:
			return "[CLIENT] Physics Update";
		case ProfilingStage::ClientStatePhysicsSimulate:
			return "[CLIENT] Physics Simulate";
		case ProfilingStage::ClientStateSoundUpdate:
			return "[CLIENT] Sound Update";
		case ProfilingStage::ClientStateEntityThink:
			return "[CLIENT] Entity Think";

		case ProfilingStage::ClientGameOcclusionCulling:
			return "[CLIENT] Occlusion Culling";
		case ProfilingStage::ClientGameRenderShadows:
			return "[CLIENT] Render Shadows";
		case ProfilingStage::ClientGameRenderScene:
			return "[CLIENT] Render Scene";
		case ProfilingStage::ClientGameRenderPostProcessing:
			return "[CLIENT] Render Post-Processing";

		case ProfilingStage::GUIDraw:
			return "[GUI] Draw";
	};
	return "Unknown";
}