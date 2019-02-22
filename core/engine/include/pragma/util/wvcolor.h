#ifndef __COLOR_H_
#define __COLOR_H_
#include "pragma/definitions.h"
#include <mathutil/color.h>
#include "pragma/console/conout.h"
inline DLLENGINE Con::c_cout& operator<<(Con::c_cout &os,const Color &col)
{
	os<<col.r<<" "<<col.g<<" "<<col.b<<" "<<col.a;
	return os;
}
#endif