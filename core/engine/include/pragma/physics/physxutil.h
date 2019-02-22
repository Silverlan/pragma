#ifndef __PHYSXUTIL_H__
#define __PHYSXUTIL_H__

#include <vector>
#include <mathutil/glmutil.h>

namespace physx
{
	class PxConvexMesh;
	PxConvexMesh *CreateConvexMesh(std::vector<Vector3> &verts);
};

#endif