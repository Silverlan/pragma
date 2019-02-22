#ifndef __ENGINE_CVAR_H__
#define __ENGINE_CVAR_H__

#include <string>
#include <pragma/console/convarhandle.h>
#include "pragma/definitions.h"
#include <pragma/console/convars.h>

DLLENGINE ConVarHandle GetEngineConVar(std::string scmd);

#endif