#ifndef __WVANGLE_H__
#define __WVANGLE_H__
#include "pragma/definitions.h"
#include <mathutil/eulerangles.h>

namespace Con
{
	class c_cout;
	class c_cwar;
	class c_cerr;
	class c_crit;
	class c_csv;
	class c_ccl;
};
DLLENGINE Con::c_cout & operator<<(Con::c_cout &os,const EulerAngles &ang);

#endif