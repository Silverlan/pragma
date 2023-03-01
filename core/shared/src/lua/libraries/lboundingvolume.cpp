#include "stdafx_shared.h"
#include "pragma/lua/libraries/lboundingvolume.h"
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_mat4.h"
#include <pragma/math/intersection.h>
#include <mathutil/boundingvolume.h>

// See https://stackoverflow.com/a/58630206/2482983
static void Transform_Box(const Mat3 &M, const Vector3 &min, const Vector3 &max, Vector3 &outNewMin, Vector3 &outNewMax)
{
	float a, b;
	float Amin[3], Amax[3];
	float Bmin[3], Bmax[3];
	int i, j;

	/*Copy box A into a min array and a max array for easy reference.*/

	Amin[0] = (float)min.x;
	Amax[0] = (float)max.x;
	Amin[1] = (float)min.y;
	Amax[1] = (float)max.y;
	Amin[2] = (float)min.z;
	Amax[2] = (float)max.z;

	/* Take care of translation by beginning at T. */

	Vector3 T {};
	Bmin[0] = Bmax[0] = (float)T.x;
	Bmin[1] = Bmax[1] = (float)T.y;
	Bmin[2] = Bmax[2] = (float)T.z;

	/* Now find the extreme points by considering the product of the */
	/* min and max with each component of M.  */

	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++) {
			a = (float)(M[i][j] * Amin[j]);
			b = (float)(M[i][j] * Amax[j]);
			if(a < b)

			{
				Bmin[i] += a;
				Bmax[i] += b;
			}
			else {
				Bmin[i] += b;
				Bmax[i] += a;
			}
		}

	/* Copy the result into the new box. */

	outNewMin.x = Bmin[0];
	outNewMax.x = Bmax[0];
	outNewMin.y = Bmin[1];
	outNewMax.y = Bmax[1];
	outNewMin.z = Bmin[2];
	outNewMax.z = Bmax[2];
}

luabind::mult<Vector3, Vector3> Lua::boundingvolume::GetRotatedAABB(lua_State *l, const Vector3 &min, const Vector3 &max, const Mat3 &rot)
{
	Vector3 rmin;
	Vector3 rmax;
	Transform_Box(rot, min, max, rmin, rmax);
	return {l, rmin, rmax};
}
luabind::mult<Vector3, Vector3> Lua::boundingvolume::GetRotatedAABB(lua_State *l, const Vector3 &min, const Vector3 &max, const Quat &rot)
{
	Vector3 rmin;
	Vector3 rmax;
	Transform_Box(glm::toMat3(rot), min, max, rmin, rmax);
	return {l, rmin, rmax};
}
