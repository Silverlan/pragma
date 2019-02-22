#ifndef __PROFILING_STAGES_H__
#define __PROFILING_STAGES_H__

#include "pragma/definitions.h"
#include <string>

enum class DLLENGINE ProfilingStage : uint32_t
{
	EngineThink = 0u,
	EngineTick,
	EngineDraw,
	EngineLoop,
	ServerStateThink,
	ServerStateTick,
	ServerStatePhysicsUpdate,
	ServerStatePhysicsSimulate,
	ServerStateSoundUpdate,
	ServerStateEntityThink,
	ClientStateThink,
	ClientStateTick,
	ClientStateDraw,
	ClientStatePhysicsUpdate,
	ClientStatePhysicsSimulate,
	ClientStateSoundUpdate,
	ClientStateEntityThink,
	GUIDraw,
	PresentDraw,
	ClientGameOcclusionCulling,
	ClientGameRenderShadows,
	ClientGameRenderScene,
	ClientGameRenderPostProcessing
};

DLLENGINE std::string profiling_stage_to_string(ProfilingStage stage);

#endif
