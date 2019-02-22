#ifndef __C_LISTENER_HANDLE_HPP__
#define __C_LISTENER_HANDLE_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/baseentity_handle.h>

class DLLCLIENT CListener;
DECLARE_DERIVED_CHILD_HANDLE(DLLCLIENT,Entity,BaseEntity,Entity,CListener,Listener);

#endif
