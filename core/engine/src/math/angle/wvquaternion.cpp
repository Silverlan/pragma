#include "stdafx_engine.h"
#include "pragma/math/angle/wvquaternion.h"

DLLENGINE Con::c_cout & operator<<(Con::c_cout &os,const Quat &q)
{
	os<<q.w<<" "<<q.x<<" "<<q.y<<" "<<q.z;
	return os;
}