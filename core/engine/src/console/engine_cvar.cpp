#include "stdafx_engine.h"
#include "pragma/console/engine_cvar.h"
#include <pragma/console/convarhandle.h>
#include "pragma/engine.h"

extern DLLENGINE Engine *engine;
ConVarHandle GetEngineConVar(std::string scmd)
{
	return engine->GetConVarHandle(scmd);
}