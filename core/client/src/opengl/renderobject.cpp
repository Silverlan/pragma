#include "stdafx_client.h"
#include "pragma/rendering/rendersystem.h"

DEFINE_BASE_HANDLE(DLLCLIENT,RenderObject,RenderObject);

RenderObject::RenderObject(GLMesh *mesh)
	: m_handle(new PtrRenderObject(this)),m_mesh(mesh)
{
}

RenderObject::~RenderObject()
{
	m_handle.Invalidate();
}

RenderObjectHandle RenderObject::GetHandle() {return m_handle;}
RenderObjectHandle *RenderObject::CreateHandle() {return m_handle.Copy();}

void RenderObject::Remove()
{
	delete this;
}

RenderMode RenderObject::GetRenderMode() {return m_renderMode;}
void RenderObject::SetRenderMode(RenderMode renderMode) {m_renderMode = renderMode;}
