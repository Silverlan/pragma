#include "stdafx_client.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/physics/pxvisualizer.h"

#pragma message ("TODO: Add more console commands for visual debugging (See c_lua.cpp for physx enums)")

PxVisualizerBuffer::PxVisualizerBuffer(const void *t,int mode)
	: target(t),renderMode(mode)
{
	if(mode == GL_POINT)
		count = 1;
	else if(mode == GL_LINES)
		count = 2;
	else
		count = 3;
	vertexBuffer = OpenGL::GenerateBuffer();
	colorBuffer = OpenGL::GenerateBuffer();
}

PxVisualizerBuffer::~PxVisualizerBuffer()
{
	OpenGL::DeleteBuffer(vertexBuffer);
	OpenGL::DeleteBuffer(colorBuffer);
}

///////////////////////////////////////////

std::vector<PxVisualizerBuffer*> PxVisualizer::m_buffers;

void PxVisualizer::RenderScene(const physx::PxRenderBuffer &pxRenderBuffer)
{
	const physx::PxDebugPoint *points = pxRenderBuffer.getPoints();
	const physx::PxDebugLine *lines = pxRenderBuffer.getLines();
	const physx::PxDebugTriangle *triangles = pxRenderBuffer.getTriangles();
	const physx::PxDebugText *texts = pxRenderBuffer.getTexts();
	std::vector<const void*> ptrs;
	for(unsigned int i=0;i<pxRenderBuffer.getNbPoints();i++)
	{
		const physx::PxDebugPoint &point = points[i];
		ptrs.push_back(&point);
		AddPoint(point);
	}
	for(unsigned int i=0;i<pxRenderBuffer.getNbLines();i++)
	{
		const physx::PxDebugLine &line = lines[i];
		ptrs.push_back(&line);
		AddLine(line);
	}
	for(unsigned int i=0;i<pxRenderBuffer.getNbTriangles();i++)
	{
		const physx::PxDebugTriangle &triangle = triangles[i];
		ptrs.push_back(&triangle);
		AddTriangle(triangle);
	}
	for(unsigned int i=0;i<pxRenderBuffer.getNbTexts();i++)
	{
		const physx::PxDebugText &text = texts[i];
		ptrs.push_back(&text);
		AddText(text);
	}
	for(int i=m_buffers.size() -1;i>=0;i--)
	{
		PxVisualizerBuffer &buf = *m_buffers[i];
		bool bExists = false;
		for(unsigned int j=0;j<ptrs.size();j++)
		{
			if(ptrs[j] == buf.target)
			{
				bExists = true;
				break;
			}
		}
		if(bExists == false)
		{
#pragma message ("TODO: Find a faster way to do this!")
			delete m_buffers[i];
			m_buffers.erase(m_buffers.begin() +i);
		}
	}
	OpenGL::EnableVertexAttribArray(0);
	OpenGL::EnableVertexAttribArray(1);
	OpenGL::Disable(GL_TEXTURE_2D);
	for(unsigned int i=0;i<m_buffers.size();i++)
	{
		PxVisualizerBuffer &buf = *m_buffers[i];
		OpenGL::BindBuffer(buf.vertexBuffer);
		OpenGL::SetVertexAttribData(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			(void*)0
		);

		OpenGL::BindBuffer(buf.colorBuffer);
		OpenGL::SetVertexAttribData(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			(void*)0
		);
		OpenGL::DrawArrays(buf.renderMode,0,buf.count);
	}
	OpenGL::DisableVertexAttribArray(0);
	OpenGL::DisableVertexAttribArray(1);
	OpenGL::Enable(GL_TEXTURE_2D);
}

void PxVisualizer::AddPoint(const physx::PxDebugPoint &point)
{
	PxVisualizerBuffer *buf = GetNewBuffer(&point,GL_POINT);
	float color[3];
	GetDebugColor(point.color,&color[0]);
	OpenGL::BindBuffer(buf->colorBuffer);
	OpenGL::BindBufferData(
		sizeof(color),
		&color[0],
		GL_STATIC_DRAW
	);
	
	physx::PxVec3 vert = point.pos;
	OpenGL::BindBuffer(buf->vertexBuffer);
	OpenGL::BindBufferData(
		sizeof(point),
		&point,
		GL_STATIC_DRAW
	);
}

void PxVisualizer::AddLine(const physx::PxDebugLine &line)
{
	PxVisualizerBuffer *buf = GetNewBuffer(&line,GL_LINES);
	float color[6];
	GetDebugColor(line.color0,&color[0]);
	GetDebugColor(line.color1,&color[3]);
	OpenGL::BindBuffer(buf->colorBuffer);
	OpenGL::BindBufferData(
		sizeof(color),
		&color[0],
		GL_STATIC_DRAW
	);
	
	physx::PxVec3 verts[2] = {line.pos0,line.pos1};
	OpenGL::BindBuffer(buf->vertexBuffer);
	OpenGL::BindBufferData(
		sizeof(verts),
		&verts[0],
		GL_STATIC_DRAW
	);
}

void PxVisualizer::AddTriangle(const physx::PxDebugTriangle &triangle)
{
	PxVisualizerBuffer *buf = GetNewBuffer(&triangle,GL_TRIANGLES);
	float color[9];
	GetDebugColor(triangle.color0,&color[0]);
	GetDebugColor(triangle.color1,&color[3]);
	GetDebugColor(triangle.color2,&color[6]);
	OpenGL::BindBuffer(buf->colorBuffer);
	OpenGL::BindBufferData(
		sizeof(color),
		&color[0],
		GL_STATIC_DRAW
	);
	
	physx::PxVec3 verts[3] = {triangle.pos0,triangle.pos1,triangle.pos2};
	OpenGL::BindBuffer(buf->vertexBuffer);
	OpenGL::BindBufferData(
		sizeof(verts),
		&verts[0],
		GL_STATIC_DRAW
	);
}

void PxVisualizer::AddText(const physx::PxDebugText &text)
{
#pragma message ("TODO: Add text-rendering!")
}

void PxVisualizer::ClearScene()
{
	for(unsigned int i=0;i<m_buffers.size();i++)
		delete m_buffers[i];
	m_buffers.clear();
}

PxVisualizerBuffer *PxVisualizer::GetBuffer(const void *ptr)
{
#pragma message ("TODO: Find a faster way to do this!")
	for(unsigned int i=0;i<m_buffers.size();i++)
	{
		PxVisualizerBuffer &buf = *m_buffers[i];
		if(buf.target == ptr)
			return &buf;
	}
	return NULL;
}

PxVisualizerBuffer *PxVisualizer::GetNewBuffer(const void *ptr,int mode)
{
	PxVisualizerBuffer *buf = GetBuffer(ptr);
	if(buf != NULL)
		return buf;
	m_buffers.push_back(new PxVisualizerBuffer(ptr,mode));
	return m_buffers.back();
}

void PxVisualizer::GetDebugColor(physx::PxU32 eCol,float *col)
{
	switch(eCol)
	{
	case physx::PxDebugColor::Enum::eARGB_BLACK:
		{
			col[0] = 0.f;
			col[1] = 0.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_RED:
		{
			col[0] = 1.f;
			col[1] = 0.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_GREEN:
		{
			col[0] = 0.f;
			col[1] = 1.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_BLUE:
		{
			col[0] = 0.f;
			col[1] = 0.f;
			col[2] = 1.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_YELLOW:
		{
			col[0] = 1.f;
			col[1] = 1.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_MAGENTA:
		{
			col[0] = 1.f;
			col[1] = 0.f;
			col[2] = 1.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_CYAN:
		{
			col[0] = 0.f;
			col[1] = 1.f;
			col[2] = 1.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_GREY:
		{
			col[0] = 0.5f;
			col[1] = 0.5f;
			col[2] = 0.5f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_DARKRED:
		{
			col[0] = 0.345f;
			col[1] = 0.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_DARKGREEN:
		{
			col[0] = 0.f;
			col[1] = 0.345f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_DARKBLUE:
		{
			col[0] = 0.f;
			col[1] = 0.f;
			col[2] = 0.345f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_WHITE:
	default:
		{
			col[0] = 1.f;
			col[1] = 1.f;
			col[2] = 1.f;
			break;
		}
	};
}
#endif