#include "stdafx_engine.h"
#include "pragma/math/angle/wvangle.h"
#include "pragma/console/conout.h"

DLLENGINE Con::c_cout& operator<<(Con::c_cout &os,const EulerAngles &ang)
{
	os<<ang.p<<" "<<ang.y<<" "<<ang.r;
	return os;
}
