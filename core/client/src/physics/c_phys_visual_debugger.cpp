#include "stdafx_client.h"
#include "pragma/physics/c_phys_visual_debugger.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
CPhysVisualDebugger::CPhysVisualDebugger()
{
	InitializeBuffers();
}

void CPhysVisualDebugger::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam)
{
	auto vp = cam.GetProjectionMatrix() *cam.GetViewMatrix();
	auto m = umat::identity();
	auto &whDebugShader = c_game->GetGameShader(CGame::GameShader::Debug);
	auto &shader = static_cast<pragma::ShaderDebug&>(*whDebugShader.get());
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebug::Pipeline::Line) == true)
	{
		(*drawCmd)->record_set_line_width(2.f);
		shader.Draw(**m_lineBuffer.buffer,m_lineBuffer.instanceCount *decltype(m_lineBuffer)::VERTS_PER_INSTANCE,vp *m);
		shader.EndDraw();
	}
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebug::Pipeline::Point) == true)
	{
		(*drawCmd)->record_set_line_width(2.f);
		shader.Draw(**m_pointBuffer.buffer,m_pointBuffer.instanceCount *decltype(m_pointBuffer)::VERTS_PER_INSTANCE,vp *m);
		shader.EndDraw();
	}
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebug::Pipeline::Triangle) == true)
	{
		(*drawCmd)->record_set_line_width(2.f);
		shader.Draw(**m_triangleBuffer.buffer,m_triangleBuffer.instanceCount *decltype(m_triangleBuffer)::VERTS_PER_INSTANCE,vp *m);
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
	c_engine->ScheduleRecordUpdateBuffer(m_lineBuffer.buffer,0,m_lineBuffer.GetDataSize(),m_lineBuffer.vertices.data());
	c_engine->ScheduleRecordUpdateBuffer(m_pointBuffer.buffer,0,m_pointBuffer.GetDataSize(),m_pointBuffer.vertices.data());
	c_engine->ScheduleRecordUpdateBuffer(m_triangleBuffer.buffer,0,m_triangleBuffer.GetDataSize(),m_triangleBuffer.vertices.data());
}
void CPhysVisualDebugger::InitializeBuffers()
{
	constexpr auto totalBufferSize = decltype(m_lineBuffer)::BUFFER_SIZE +decltype(m_pointBuffer)::BUFFER_SIZE +decltype(m_triangleBuffer)::BUFFER_SIZE;
	constexpr auto totalBufferSizeMb = totalBufferSize /1024 /1024;

	auto &dev = c_engine->GetDevice();
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = totalBufferSize;
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	m_debugBuffer = prosper::util::create_buffer(dev,createInfo);
	m_debugBuffer->SetPermanentlyMapped(true);
	vk::DeviceSize offset = 0;

	m_lineBuffer.buffer = prosper::util::create_sub_buffer(*m_debugBuffer,offset,decltype(m_lineBuffer)::BUFFER_SIZE);
	offset += decltype(m_lineBuffer)::BUFFER_SIZE;

	m_pointBuffer.buffer = prosper::util::create_sub_buffer(*m_debugBuffer,offset,decltype(m_pointBuffer)::BUFFER_SIZE);
	offset += decltype(m_pointBuffer)::BUFFER_SIZE;

	m_triangleBuffer.buffer = prosper::util::create_sub_buffer(*m_debugBuffer,offset,decltype(m_triangleBuffer)::BUFFER_SIZE);
	offset += decltype(m_triangleBuffer)::BUFFER_SIZE;
}

void CPhysVisualDebugger::DrawLine(const Vector3 &from,const Vector3 &to,const Color &fromColor,const Color &toColor)
{
	m_lineBuffer.AddInstance({from,to});

//	m_colorData.push_back(fromColor.ToVector4());
	//m_colorData.push_back(toColor.ToVector4());
}
void CPhysVisualDebugger::DrawPoint(const Vector3 &pos,const Color &color)
{
	m_pointBuffer.AddInstance({pos});
}
void CPhysVisualDebugger::DrawTriangle(const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,const Color &c0,const Color &c1,const Color &c2)
{
	m_triangleBuffer.AddInstance({v0,v1,v2});
}
void CPhysVisualDebugger::ReportErrorWarning(const std::string &str)
{
	Con::cwar<<"[PhysX] WARNING: "<<str<<Con::endl;
}
void CPhysVisualDebugger::DrawText(const std::string &str,const Vector3 &location,const Color &color,float size)
{
	// TODO
}
#pragma optimize("",on)
