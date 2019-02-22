#include "stdafx_shared.h"
#include "pragma/physics/controllerhitdata.h"
#include "pragma/physics/physobj.h"

ControllerHitData::ControllerHitData()
	: physObj(NULL)
{
	Clear();
}

void ControllerHitData::Clear()
{
	if(physObj != NULL)
	{
		delete physObj;
		physObj = NULL;
	}
}

ControllerHitData::~ControllerHitData()
{
	Clear();
}