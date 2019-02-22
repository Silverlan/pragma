#include "stdafx_engine.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/physics/physxcallbacks.h"
#include "pragma/physics/physxutil.h"
#include "pragma/physics/physxapi.h"
#include "pragma/engine.h"
#include "pragma/physics/physxstream.h"

extern Engine *engine;

physx::PxConvexMesh *physx::CreateConvexMesh(std::vector<Vector3> &verts)
{
	physx::PxPhysics *physics = engine->GetPhysics();
	physx::PxCooking *cooking = engine->GetCookingLibrary();
	physx::PxConvexMeshDesc meshDesc;
	meshDesc.points.stride = sizeof(Vector3);
	meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
	meshDesc.vertexLimit = 256;
	meshDesc.points.count = verts.size();
	meshDesc.points.data = &verts[0];
	physx::MemoryOutputStream buf;
	if(cooking->cookConvexMesh(meshDesc,buf))
	{
		physx::MemoryInputData input(buf.getData(),buf.getSize());
		return physics->createConvexMesh(input);
	}
	return NULL;
}
#endif