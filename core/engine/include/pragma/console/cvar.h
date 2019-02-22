#ifndef __CVAR_H__
#define __CVAR_H__

#include <string>
#include "pragma/definitions.h"
#include <pragma/console/convarhandle.h>

DLLENGINE ConVarHandle GetConVar(std::string scmd);

#endif