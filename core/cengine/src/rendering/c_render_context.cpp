#include "stdafx_cengine.h"
#include "pragma/rendering/c_render_context.hpp"
#include <prosper_util.hpp>
#include <debug/prosper_debug.hpp>
#include <shader/prosper_shader.hpp>

using namespace pragma;

#pragma optimize("",off)
RenderContext::RenderContext()
	: prosper::Context(engine_info::get_name(),false)
	,m_bWindowedMode(true),m_monitor(nullptr),m_aspectRatio(1.f)
{
	GetWindowCreationInfo().resizable = false;
	prosper::Shader::SetLogCallback([](prosper::Shader &shader,Anvil::ShaderStage stage,const std::string &infoLog,const std::string &debugInfoLog) {
		Con::cwar<<"Unable to load shader '"<<shader.GetIdentifier()<<"':"<<Con::endl;
		Con::cwar<<"Shader Stage: "<<prosper::util::to_string(stage)<<Con::endl;
		Con::cwar<<infoLog<<Con::endl<<Con::endl;
		Con::cwar<<debugInfoLog<<Con::endl;
	});
	prosper::debug::set_debug_validation_callback([](vk::DebugReportObjectTypeEXT objectType,const std::string &msg) {
		Con::cerr<<"[VK] "<<msg<<Con::endl;
	});
	GLFW::initialize();
}
RenderContext::~RenderContext()
{}
VkBool32 RenderContext::ValidationCallback(
	Anvil::DebugMessageSeverityFlags severityFlags,
	const char *message
)
{
	if((severityFlags &Anvil::DebugMessageSeverityFlagBits::ERROR_BIT) != Anvil::DebugMessageSeverityFlagBits::NONE)
	{
		std::string strMsg = message;
		prosper::debug::add_debug_object_information(strMsg);
		Con::cerr<<"[VK] "<<strMsg<<Con::endl;
	}
    return prosper::Context::ValidationCallback(severityFlags,message);
}

void RenderContext::OnWindowInitialized()
{
	prosper::Context::OnWindowInitialized();
	m_scheduledWindowReloadInfo = nullptr;
}

void RenderContext::DrawFrame()
{
	UpdateWindow();
	prosper::Context::DrawFrame();
}

void RenderContext::UpdateWindow()
{
	if(m_scheduledWindowReloadInfo == nullptr)
		return;
	WaitIdle();
	auto &creationInfo = GetWindowCreationInfo();
	if(m_scheduledWindowReloadInfo->windowedMode.has_value())
		m_bWindowedMode = *m_scheduledWindowReloadInfo->windowedMode;
	if(m_scheduledWindowReloadInfo->refreshRate.has_value())
		creationInfo.refreshRate = (*m_scheduledWindowReloadInfo->refreshRate != 0u) ? static_cast<int32_t>(*m_scheduledWindowReloadInfo->refreshRate) : GLFW_DONT_CARE;
	if(m_scheduledWindowReloadInfo->decorated.has_value())
		creationInfo.decorated = *m_scheduledWindowReloadInfo->decorated;
	if(m_scheduledWindowReloadInfo->width.has_value())
		creationInfo.width = *m_scheduledWindowReloadInfo->width;
	if(m_scheduledWindowReloadInfo->height.has_value())
		creationInfo.height = *m_scheduledWindowReloadInfo->height;
	m_aspectRatio = CFloat(creationInfo.width) /CFloat(creationInfo.height);
	if(m_scheduledWindowReloadInfo->monitor.has_value())
		creationInfo.monitor = std::move(*m_scheduledWindowReloadInfo->monitor);
	if(m_scheduledWindowReloadInfo->presentMode.has_value())
		prosper::Context::SetPresentMode(*m_scheduledWindowReloadInfo->presentMode);
	m_scheduledWindowReloadInfo = nullptr;
	if(m_bWindowedMode == false)
	{
		if(creationInfo.monitor == nullptr)
			creationInfo.monitor = std::make_unique<GLFW::Monitor>(GLFW::get_primary_monitor());
	}
	else
		creationInfo.monitor = nullptr;
	ReloadWindow();
}
RenderContext::WindowChangeInfo &RenderContext::ScheduleWindowReload()
{
	if(m_scheduledWindowReloadInfo == nullptr)
		m_scheduledWindowReloadInfo = std::unique_ptr<WindowChangeInfo>(new WindowChangeInfo{});
	return *m_scheduledWindowReloadInfo;
}
void RenderContext::SetMonitor(GLFW::Monitor &monitor)
{
	auto &creationInfo = GetWindowCreationInfo();
	if(&monitor == creationInfo.monitor.get())
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.monitor = std::make_unique<GLFW::Monitor>(monitor);
}
void RenderContext::SetPresentMode(Anvil::PresentModeKHR presentMode)
{
	if(presentMode == GetPresentMode())
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.presentMode = presentMode;
}
void RenderContext::SetWindowedMode(bool b)
{
	if(b == m_bWindowedMode)
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.windowedMode = b;
}
void RenderContext::SetRefreshRate(uint32_t rate)
{
	auto &creationInfo = GetWindowCreationInfo();
	if(rate == creationInfo.refreshRate)
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.refreshRate = rate;
}
void RenderContext::SetNoBorder(bool b)
{
	auto &creationInfo = GetWindowCreationInfo();
	if(!b == creationInfo.decorated)
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.decorated = !b;
}
void RenderContext::SetResolution(const Vector2i &sz)
{
	auto &creationInfo = GetWindowCreationInfo();
	if(sz.x == creationInfo.width && sz.y == creationInfo.height)
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.width = sz.x;
	changeInfo.height = sz.y;
}
float RenderContext::GetAspectRatio() const {return m_aspectRatio;}
#pragma optimize("",on)
