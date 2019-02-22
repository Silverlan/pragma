#ifndef __S_CVAR_H__
#define __S_CVAR_H__

#include <string>
#include "pragma/serverdefinitions.h"
#include "pragma/console/convarhandle.h"

DLLSERVER ConVarHandle GetServerConVar(std::string scmd);

#endif