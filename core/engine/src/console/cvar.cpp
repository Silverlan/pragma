#include "stdafx_engine.h"
#include "pragma/console/cvar.h"
#include <pragma/console/convarhandle.h>
#include "pragma/engine.h"
#include <pragma/console/convars.h>

extern Engine *engine;
ConVarHandle GetConVar(std::string scmd)
{
	return engine->GetConVarHandle(scmd);
}
