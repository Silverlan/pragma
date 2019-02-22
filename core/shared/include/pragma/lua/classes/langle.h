#ifndef __LANGLE_H__
#define __LANGLE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/math/angle/wvangle.h>
#include <pragma/math/angle/wvquaternion.h>
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>

namespace Lua
{
	namespace Angle
	{
		DLLNETWORK Vector3 Up(EulerAngles *ang);
		DLLNETWORK void Orientation(lua_State *l,EulerAngles *ang);
		DLLNETWORK void Copy(lua_State *l,EulerAngles *ang);
		DLLNETWORK void ToMatrix(lua_State *l,EulerAngles *ang);
		DLLNETWORK void ToQuaternion(lua_State *l,EulerAngles *ang,uint32_t rotationOrder);
		DLLNETWORK void Set(lua_State *l,EulerAngles *ang,float p,float y,float r);
	};
};

#endif
