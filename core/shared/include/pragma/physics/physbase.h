#ifndef __PHYSBASE_H__
#define __PHYSBASE_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <pragma/physics/physapi.h>
#include <vector>

class PhysHandle;
class PhysEnv;
class DLLNETWORK PhysBase
{
public:
	friend PhysEnv;
protected:
	PhysBase(PhysEnv *env);

	PhysEnv *m_physEnv;
	PhysHandle *m_baseHandle;
public:
	virtual ~PhysBase();
	PhysHandle *CreateBaseHandle();
	PhysHandle GetBaseHandle();
};

DECLARE_BASE_HANDLE(DLLNETWORK,PhysBase,Phys);

#endif