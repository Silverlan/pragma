// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :physics.visual_debugger;

import :engine;
import :game;
import :rendering.shaders;

#undef DrawText

pragma::physics::CPhysVisualDebugger::CPhysVisualDebugger() { InitializeBuffers(); }

void pragma::physics::CPhysVisualDebugger::Render(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, CCameraComponent &cam)
{
	auto vp = cam.GetProjectionMatrix() * cam.GetViewMatrix();
	auto m = umat::identity();
	auto &whDebugShader = get_cgame()->GetGameShader(CGame::GameShader::DebugVertex);
	auto &shader = static_cast<ShaderDebugVertexColor &>(*whDebugShader.get());
	prosper::ShaderBindState bindState {*drawCmd};
	if(shader.RecordBeginDraw(bindState, ShaderDebugVertexColor::Pipeline::Line) == true) {
		drawCmd->RecordSetLineWidth(2.f);
		shader.RecordDraw(bindState, *m_lineBuffer.buffer, *m_lineBuffer.colorBuffer, m_lineBuffer.instanceCount * decltype(m_lineBuffer)::VERTS_PER_INSTANCE, vp * m);
		shader.RecordEndDraw(bindState);
	}
	if(shader.RecordBeginDraw(bindState, ShaderDebugVertexColor::Pipeline::Point) == true) {
		drawCmd->RecordSetLineWidth(2.f);
		shader.RecordDraw(bindState, *m_pointBuffer.buffer, *m_lineBuffer.colorBuffer, m_pointBuffer.instanceCount * decltype(m_pointBuffer)::VERTS_PER_INSTANCE, vp * m);
		shader.RecordEndDraw(bindState);
	}
	if(shader.RecordBeginDraw(bindState, ShaderDebugVertexColor::Pipeline::Triangle) == true) {
		drawCmd->RecordSetLineWidth(2.f);
		shader.RecordDraw(bindState, *m_triangleBuffer.buffer, *m_lineBuffer.colorBuffer, m_triangleBuffer.instanceCount * decltype(m_triangleBuffer)::VERTS_PER_INSTANCE, vp * m);
		shader.RecordEndDraw(bindState);
	}
}

void pragma::physics::CPhysVisualDebugger::Reset()
{
	m_lineBuffer.Reset();
	m_pointBuffer.Reset();
	m_triangleBuffer.Reset();
}
void pragma::physics::CPhysVisualDebugger::Flush()
{
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_lineBuffer.buffer, 0, m_lineBuffer.GetDataSize(), m_lineBuffer.vertices.data());
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_pointBuffer.buffer, 0, m_pointBuffer.GetDataSize(), m_pointBuffer.vertices.data());
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_triangleBuffer.buffer, 0, m_triangleBuffer.GetDataSize(), m_triangleBuffer.vertices.data());

	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_lineBuffer.colorBuffer, 0, m_lineBuffer.GetColorDataSize(), m_lineBuffer.vertexColors.data());
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_pointBuffer.colorBuffer, 0, m_pointBuffer.GetColorDataSize(), m_pointBuffer.vertexColors.data());
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_triangleBuffer.colorBuffer, 0, m_triangleBuffer.GetColorDataSize(), m_triangleBuffer.vertexColors.data());
}
void pragma::physics::CPhysVisualDebugger::InitializeBuffers()
{
	constexpr auto totalBufferSize = decltype(m_lineBuffer)::BUFFER_SIZE + decltype(m_pointBuffer)::BUFFER_SIZE + decltype(m_triangleBuffer)::BUFFER_SIZE;
	constexpr auto totalBufferSizeMb = totalBufferSize / 1024 / 1024;

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = totalBufferSize;
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	m_debugBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo);
	m_debugBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);

	constexpr auto colorBufferSize
	  = (decltype(m_lineBuffer)::VERTS_PER_INSTANCE * decltype(m_lineBuffer)::MAX_INSTANCES + decltype(m_pointBuffer)::VERTS_PER_INSTANCE * decltype(m_pointBuffer)::MAX_INSTANCES + decltype(m_triangleBuffer)::VERTS_PER_INSTANCE * decltype(m_triangleBuffer)::MAX_INSTANCES)
	  * sizeof(Vector4);
	constexpr auto colorBufferSizeMb = colorBufferSize / 1024 / 1024;
	createInfo.size = colorBufferSize;

	m_colorBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo);
	m_colorBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);

	prosper::DeviceSize offset = 0;
	prosper::DeviceSize colorOffset = 0;

	m_lineBuffer.buffer = m_debugBuffer->CreateSubBuffer(offset, decltype(m_lineBuffer)::BUFFER_SIZE);
	m_lineBuffer.colorBuffer = m_colorBuffer->CreateSubBuffer(offset, decltype(m_lineBuffer)::COLOR_BUFFER_SIZE);
	offset += decltype(m_lineBuffer)::BUFFER_SIZE;
	colorOffset += decltype(m_lineBuffer)::COLOR_BUFFER_SIZE;

	m_pointBuffer.buffer = m_debugBuffer->CreateSubBuffer(offset, decltype(m_pointBuffer)::BUFFER_SIZE);
	m_pointBuffer.colorBuffer = m_colorBuffer->CreateSubBuffer(offset, decltype(m_pointBuffer)::COLOR_BUFFER_SIZE);
	offset += decltype(m_pointBuffer)::BUFFER_SIZE;
	colorOffset += decltype(m_pointBuffer)::COLOR_BUFFER_SIZE;

	m_triangleBuffer.buffer = m_debugBuffer->CreateSubBuffer(offset, decltype(m_triangleBuffer)::BUFFER_SIZE);
	m_triangleBuffer.colorBuffer = m_colorBuffer->CreateSubBuffer(offset, decltype(m_triangleBuffer)::COLOR_BUFFER_SIZE);
	offset += decltype(m_triangleBuffer)::BUFFER_SIZE;
	colorOffset += decltype(m_triangleBuffer)::COLOR_BUFFER_SIZE;
}

void pragma::physics::CPhysVisualDebugger::DrawLine(const Vector3 &from, const Vector3 &to, const Color &fromColor, const Color &toColor) { m_lineBuffer.AddInstance({from, to}, {fromColor.ToVector4(), toColor.ToVector4()}); }
void pragma::physics::CPhysVisualDebugger::DrawPoint(const Vector3 &pos, const Color &color) { m_pointBuffer.AddInstance({pos}, {color.ToVector4()}); }
void pragma::physics::CPhysVisualDebugger::DrawTriangle(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Color &c0, const Color &c1, const Color &c2) { m_triangleBuffer.AddInstance({v0, v1, v2}, {c0.ToVector4(), c1.ToVector4(), c2.ToVector4()}); }
void pragma::physics::CPhysVisualDebugger::ReportErrorWarning(const std::string &str) { Con::CWAR << "[Phys] WARNING: " << str << Con::endl; }
void pragma::physics::CPhysVisualDebugger::DrawText(const std::string &str, const Vector3 &location, const Color &color, float size)
{
	// TODO
}
