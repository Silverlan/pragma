#ifndef __RENDEROBJECT_H__
#define __RENDEROBJECT_H__

#include "pragma/clientdefinitions.h"
#include <sharedutils/def_handle.h>
#include "pragma/rendering/c_rendermode.h"
#include <memory>

class DLLCLIENT RenderObject;
DECLARE_BASE_HANDLE(DLLCLIENT,RenderObject,RenderObject);

class RenderSystem;
class RenderList;
class RenderInstance;
class Material;
class GLMesh;
struct Camera;
class DLLCLIENT RenderObject
{
public:
	friend RenderSystem;
	friend RenderList;
	friend RenderInstance;
private:
	RenderObjectHandle m_handle;
	GLMesh *m_mesh;
	RenderMode m_renderMode;
protected:
	RenderObject(GLMesh *mesh);
	~RenderObject();
	RenderMode GetRenderMode();
	void SetRenderMode(RenderMode renderMode);
public:
	RenderObjectHandle GetHandle();
	RenderObjectHandle *CreateHandle();
	void Remove();
};

#endif