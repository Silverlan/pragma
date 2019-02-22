#ifndef __WVQUATERNION_H__
#define __WVQUATERNION_H__
#include "pragma/definitions.h"
#include <mathutil/uquat.h>
#include "pragma/console/conout.h"

DLLENGINE Con::c_cout& operator<<(Con::c_cout &os,const Quat &q);

#endif