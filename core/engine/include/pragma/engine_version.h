#ifndef __ENGINE_VERSION_H__
#define __ENGINE_VERSION_H__

#include "pragma/definitions.h"
#include <sharedutils/utildefinitions.h>
#include <string>

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 4
#define ENGINE_VERSION_REVISION 2

namespace util
{
	struct DLLSHUTIL Version;
}

DLLENGINE util::Version &get_engine_version();
DLLENGINE std::string get_pretty_engine_version();

#endif