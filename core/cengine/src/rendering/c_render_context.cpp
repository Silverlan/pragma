/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_cengine.h"
#include "pragma/rendering/c_render_context.hpp"
#include <prosper_util.hpp>
#include <debug/prosper_debug.hpp>
#include <shader/prosper_shader.hpp>
#include <sharedutils/util_library.hpp>

using namespace pragma;
#pragma optimize("",off)
RenderContext::RenderContext()
	: m_bWindowedMode(true),m_monitor(nullptr),m_aspectRatio(1.f)
{
	// TODO
	std::string err;
	auto lib = util::Library::Load("E:\\projects\\pragma\\build_winx64\\output\\modules\\graphics\\vulkan\\prosper_vulkan.dll",{""},&err);
	if(lib != nullptr)
	{
		auto fInitRenderAPI = lib->FindSymbolAddress<bool(*)(const std::string&,bool,std::shared_ptr<prosper::IPrContext>&,std::string&)>("initialize_render_api");
		if(fInitRenderAPI == nullptr)
			err = "Symbol 'initialize_render_api' not found in Prosper library!";
		else
		{
			std::string errMsg;
			auto success = fInitRenderAPI(engine_info::get_name(),false,m_renderContext,errMsg);
			if(success == false)
				err = errMsg;
		}
	}
	if(m_renderContext == nullptr)
		throw std::runtime_error{"Unable to load Vulkan implementation library for Prosper: " +err +"!"};

	prosper::Callbacks callbacks {};
	callbacks.validationCallback = [this](prosper::DebugMessageSeverityFlags severityFlags,const char *message) {
		ValidationCallback(severityFlags,message);
	};
	callbacks.onWindowInitialized = [this]() {OnWindowInitialized();};
	callbacks.onClose = [this]() {OnClose();};
	callbacks.onResolutionChanged = [this](uint32_t w,uint32_t h) {OnResolutionChanged(w,h);};
	callbacks.drawFrame = [this](prosper::IPrimaryCommandBuffer &drawCmd,uint32_t swapchainImageIdx) {DrawFrame(drawCmd,swapchainImageIdx);};
	m_renderContext->SetCallbacks(callbacks);

	GetRenderContext().GetWindowCreationInfo().resizable = false;
	prosper::Shader::SetLogCallback([](prosper::Shader &shader,prosper::ShaderStage stage,const std::string &infoLog,const std::string &debugInfoLog) {
		Con::cwar<<"Unable to load shader '"<<shader.GetIdentifier()<<"':"<<Con::endl;
		Con::cwar<<"Shader Stage: "<<prosper::util::to_string(stage)<<Con::endl;
		Con::cwar<<infoLog<<Con::endl<<Con::endl;
		Con::cwar<<debugInfoLog<<Con::endl;
	});
	prosper::debug::set_debug_validation_callback([](prosper::DebugReportObjectTypeEXT objectType,const std::string &msg) {
		Con::cerr<<"[VK] "<<msg<<Con::endl;
	});
	GLFW::initialize();
}
RenderContext::~RenderContext()
{}
void RenderContext::Release()
{
	if(m_renderContext == nullptr)
		return;
	GetRenderContext().Close();
	m_renderContext = nullptr;
}
const prosper::IPrContext &RenderContext::GetRenderContext() const {return const_cast<RenderContext*>(this)->GetRenderContext();}
prosper::IPrContext &RenderContext::GetRenderContext() {return *m_renderContext;}
prosper::ShaderManager &RenderContext::GetShaderManager() const {return GetRenderContext().GetShaderManager();}
::util::WeakHandle<prosper::Shader> RenderContext::RegisterShader(const std::string &identifier,const std::function<prosper::Shader*(prosper::IPrContext&,const std::string&)> &fFactory)
{
	return GetRenderContext().RegisterShader(identifier,fFactory);
}
::util::WeakHandle<prosper::Shader> RenderContext::GetShader(const std::string &identifier) const
{
	return GetRenderContext().GetShader(identifier);
}

GLFW::Window &RenderContext::GetWindow() {return GetRenderContext().GetWindow();}
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &RenderContext::GetSetupCommandBuffer() {return GetRenderContext().GetSetupCommandBuffer();}
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &RenderContext::GetDrawCommandBuffer() const {return GetRenderContext().GetDrawCommandBuffer();}
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &RenderContext::GetDrawCommandBuffer(uint32_t swapchainIdx) const {return GetRenderContext().GetDrawCommandBuffer(swapchainIdx);}
void RenderContext::FlushSetupCommandBuffer() {GetRenderContext().FlushSetupCommandBuffer();}
void RenderContext::ValidationCallback(
	prosper::DebugMessageSeverityFlags severityFlags,
	const char *message
)
{
	if((severityFlags &prosper::DebugMessageSeverityFlags::ErrorBit) != prosper::DebugMessageSeverityFlags::None)
	{
		std::string strMsg = message;
		// A VkImageStencilUsageCreateInfoEXT error is caused due to a bug in Anvil: https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/issues/153
		// We'll just ignore it for now, since it doesn't affect us in any way.
		// TODO: Remove this condition once the Anvil bug has been dealt with.
		if(ustring::compare(strMsg.c_str(),"VkImageStencilUsageCreateInfoEXT",true,strlen("VkImageStencilUsageCreateInfoEXT")) == false)
		{
			prosper::debug::add_debug_object_information(strMsg);
			Con::cerr<<"[VK] "<<strMsg<<Con::endl;
		}
	}
}

void RenderContext::OnClose() {}

void RenderContext::OnResolutionChanged(uint32_t w,uint32_t h) {}
void RenderContext::DrawFrame(prosper::IPrimaryCommandBuffer &drawCmd,uint32_t swapchainImageIdx) {}
void RenderContext::OnWindowInitialized() {m_scheduledWindowReloadInfo = nullptr;}

void RenderContext::DrawFrame()
{
	UpdateWindow();
	GetRenderContext().DrawFrame();
}

void RenderContext::UpdateWindow()
{
	if(m_scheduledWindowReloadInfo == nullptr)
		return;
	GetRenderContext().WaitIdle();
	auto &creationInfo = GetRenderContext().GetWindowCreationInfo();
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
		GetRenderContext().SetPresentMode(*m_scheduledWindowReloadInfo->presentMode);
	m_scheduledWindowReloadInfo = nullptr;
	if(m_bWindowedMode == false)
	{
		if(creationInfo.monitor == nullptr)
			creationInfo.monitor = std::make_unique<GLFW::Monitor>(GLFW::get_primary_monitor());
	}
	else
		creationInfo.monitor = nullptr;
	GetRenderContext().ReloadWindow();
}
RenderContext::WindowChangeInfo &RenderContext::ScheduleWindowReload()
{
	if(m_scheduledWindowReloadInfo == nullptr)
		m_scheduledWindowReloadInfo = std::unique_ptr<WindowChangeInfo>(new WindowChangeInfo{});
	return *m_scheduledWindowReloadInfo;
}
void RenderContext::SetMonitor(GLFW::Monitor &monitor)
{
	auto &creationInfo = GetRenderContext().GetWindowCreationInfo();
	if(&monitor == creationInfo.monitor.get())
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.monitor = std::make_unique<GLFW::Monitor>(monitor);
}
void RenderContext::SetPresentMode(prosper::PresentModeKHR presentMode)
{
	if(presentMode == GetRenderContext().GetPresentMode())
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
	auto &creationInfo = GetRenderContext().GetWindowCreationInfo();
	if(rate == creationInfo.refreshRate)
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.refreshRate = rate;
}
void RenderContext::SetNoBorder(bool b)
{
	auto &creationInfo = GetRenderContext().GetWindowCreationInfo();
	if(!b == creationInfo.decorated)
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.decorated = !b;
}
void RenderContext::SetResolution(const Vector2i &sz)
{
	auto &creationInfo = GetRenderContext().GetWindowCreationInfo();
	if(sz.x == creationInfo.width && sz.y == creationInfo.height)
		return;
	auto &changeInfo = ScheduleWindowReload();
	changeInfo.width = sz.x;
	changeInfo.height = sz.y;
}
float RenderContext::GetAspectRatio() const {return m_aspectRatio;}
#pragma optimize("",on)