#ifndef __C_NWM_UTIL_H__
#define __C_NWM_UTIL_H__

#include "pragma/clientdefinitions.h"
#include <pragma/networking/nwm_util.h>

namespace nwm
{
	DLLCLIENT CBaseEntity *read_unique_entity(NetPacket &packet);
	DLLCLIENT CallbackHandle read_unique_entity(NetPacket &packet,const std::function<void(BaseEntity*)> &onCreated);
};

#endif
