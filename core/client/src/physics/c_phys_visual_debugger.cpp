/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/physics/c_phys_visual_debugger.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

CPhysVisualDebugger::CPhysVisualDebugger()
{
	InitializeBuffers();
}

void CPhysVisualDebugger::Render(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam)
{
	auto vp = cam.GetProjectionMatrix() *cam.GetViewMatrix();
	auto m = umat::identity();
	auto &whDebugShader = c_game->GetGameShader(CGame::GameShader::DebugVertex);
	auto &shader = static_cast<pragma::ShaderDebugVertexColor&>(*whDebugShader.get());
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebugVertexColor::Pipeline::Line) == true)
	{
		drawCmd->RecordSetLineWidth(2.f);
		shader.Draw(*m_lineBuffer.buffer,*m_lineBuffer.colorBuffer,m_lineBuffer.instanceCount *decltype(m_lineBuffer)::VERTS_PER_INSTANCE,vp *m);
		shader.EndDraw();
	}
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebugVertexColor::Pipeline::Point) == true)
	{
		drawCmd->RecordSetLineWidth(2.f);
		shader.Draw(*m_pointBuffer.buffer,*m_lineBuffer.colorBuffer,m_pointBuffer.instanceCount *decltype(m_pointBuffer)::VERTS_PER_INSTANCE,vp *m);
		shader.EndDraw();
	}
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebugVertexColor::Pipeline::Triangle) == true)
	{
		drawCmd->RecordSetLineWidth(2.f);
		shader.Draw(*m_triangleBuffer.buffer,*m_lineBuffer.colorBuffer,m_triangleBuffer.instanceCount *decltype(m_triangleBuffer)::VERTS_PER_INSTANCE,vp *m);
		shader.EndDraw();
	}
}

void CPhysVisualDebugger::Reset()
{
	m_lineBuffer.Reset();
	m_pointBuffer.Reset();
	m_triangleBuffer.Reset();
}
void CPhysVisualDebugger::Flush()
{
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_lineBuffer.buffer,0,m_lineBuffer.GetDataSize(),m_lineBuffer.vertices.data());
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_pointBuffer.buffer,0,m_pointBuffer.GetDataSize(),m_pointBuffer.vertices.data());
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_triangleBuffer.buffer,0,m_triangleBuffer.GetDataSize(),m_triangleBuffer.vertices.data());

	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_lineBuffer.colorBuffer,0,m_lineBuffer.GetColorDataSize(),m_lineBuffer.vertexColors.data());
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_pointBuffer.colorBuffer,0,m_pointBuffer.GetColorDataSize(),m_pointBuffer.vertexColors.data());
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_triangleBuffer.colorBuffer,0,m_triangleBuffer.GetColorDataSize(),m_triangleBuffer.vertexColors.data());
}
void CPhysVisualDebugger::InitializeBuffers()
{
	constexpr auto totalBufferSize = decltype(m_lineBuffer)::BUFFER_SIZE +decltype(m_pointBuffer)::BUFFER_SIZE +decltype(m_triangleBuffer)::BUFFER_SIZE;
	constexpr auto totalBufferSizeMb = totalBufferSize /1024 /1024;

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = totalBufferSize;
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	m_debugBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo);
	m_debugBuffer->SetPermanentlyMapped(true);

	constexpr auto colorBufferSize = (decltype(m_lineBuffer)::VERTS_PER_INSTANCE *decltype(m_lineBuffer)::MAX_INSTANCES +
		decltype(m_pointBuffer)::VERTS_PER_INSTANCE *decltype(m_pointBuffer)::MAX_INSTANCES +
		decltype(m_triangleBuffer)::VERTS_PER_INSTANCE *decltype(m_triangleBuffer)::MAX_INSTANCES) *sizeof(Vector4);
	constexpr auto colorBufferSizeMb = colorBufferSize /1024 /1024;
	createInfo.size = colorBufferSize;

	m_colorBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo);
	m_colorBuffer->SetPermanentlyMapped(true);

	prosper::DeviceSize offset = 0;
	prosper::DeviceSize colorOffset = 0;

	m_lineBuffer.buffer = m_debugBuffer->CreateSubBuffer(offset,decltype(m_lineBuffer)::BUFFER_SIZE);
	m_lineBuffer.colorBuffer = m_colorBuffer->CreateSubBuffer(offset,decltype(m_lineBuffer)::COLOR_BUFFER_SIZE);
	offset += decltype(m_lineBuffer)::BUFFER_SIZE;
	colorOffset += decltype(m_lineBuffer)::COLOR_BUFFER_SIZE;

	m_pointBuffer.buffer = m_debugBuffer->CreateSubBuffer(offset,decltype(m_pointBuffer)::BUFFER_SIZE);
	m_pointBuffer.colorBuffer = m_colorBuffer->CreateSubBuffer(offset,decltype(m_pointBuffer)::COLOR_BUFFER_SIZE);
	offset += decltype(m_pointBuffer)::BUFFER_SIZE;
	colorOffset += decltype(m_pointBuffer)::COLOR_BUFFER_SIZE;

	m_triangleBuffer.buffer = m_debugBuffer->CreateSubBuffer(offset,decltype(m_triangleBuffer)::BUFFER_SIZE);
	m_triangleBuffer.colorBuffer = m_colorBuffer->CreateSubBuffer(offset,decltype(m_triangleBuffer)::COLOR_BUFFER_SIZE);
	offset += decltype(m_triangleBuffer)::BUFFER_SIZE;
	colorOffset += decltype(m_triangleBuffer)::COLOR_BUFFER_SIZE;
}

void CPhysVisualDebugger::DrawLine(const Vector3 &from,const Vector3 &to,const Color &fromColor,const Color &toColor)
{
	m_lineBuffer.AddInstance({from,to},{fromColor.ToVector4(),toColor.ToVector4()});
}
void CPhysVisualDebugger::DrawPoint(const Vector3 &pos,const Color &color)
{
	m_pointBuffer.AddInstance({pos},{color.ToVector4()});
}
void CPhysVisualDebugger::DrawTriangle(const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,const Color &c0,const Color &c1,const Color &c2)
{
	m_triangleBuffer.AddInstance({v0,v1,v2},{c0.ToVector4(),c1.ToVector4(),c2.ToVector4()});
}
void CPhysVisualDebugger::ReportErrorWarning(const std::string &str)
{
	Con::cwar<<"[PhysX] WARNING: "<<str<<Con::endl;
}
void CPhysVisualDebugger::DrawText(const std::string &str,const Vector3 &location,const Color &color,float size)
{
	// TODO
}
