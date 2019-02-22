#ifndef __PXVISUALIZER_H__
#define __PXVISUALIZER_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include "pragma/physics/physxapi.h"
#include <vector>

class PxVisualizer;
struct DLLCLIENT PxVisualizerBuffer
{
	int renderMode;
	int count;
	unsigned int vertexBuffer;
	unsigned int colorBuffer;
	const void *target;
public:
	friend PxVisualizer;
	~PxVisualizerBuffer();
protected:
	PxVisualizerBuffer(const void *target,int mode);
};

class DLLCLIENT PxVisualizer
{
protected:
	static std::vector<PxVisualizerBuffer*> m_buffers;
	static PxVisualizerBuffer *GetBuffer(const void *ptr);
	static PxVisualizerBuffer *GetNewBuffer(const void *ptr,int mode);
	static void GetDebugColor(physx::PxU32 eCol,float *col);
	static void AddPoint(const physx::PxDebugPoint &point);
	static void AddLine(const physx::PxDebugLine &line);
	static void AddTriangle(const physx::PxDebugTriangle &triangle);
	static void AddText(const physx::PxDebugText &text);
public:
	static void RenderScene(const physx::PxRenderBuffer &pxRenderBuffer);
	static void ClearScene();
};
#endif

#endif