/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/rendering/c_render_context.hpp"
#include "pragma/rendering/render_apis.hpp"
#include <prosper_util.hpp>
#include <debug/prosper_debug.hpp>
#include <shader/prosper_shader.hpp>
#include <sharedutils/util_library.hpp>
#include <pragma/util/util_module.hpp>
#include <pragma/lua/lua_error_handling.hpp>
#include <pragma/logging.hpp>
#include <prosper_window.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

RenderContext::RenderContext() : m_monitor(nullptr), m_renderAPI {"vulkan"} {}
RenderContext::~RenderContext() { m_graphicsAPILib = nullptr; }
DLLNETWORK std::optional<std::string> g_customTitle;
void RenderContext::InitializeRenderAPI()
{
	auto &renderAPI = GetRenderAPI();
	auto getRenderApiPath = [](const std::string &renderAPI, std::string &outLocation, std::string &outModulePath) {
		outLocation = pragma::rendering::get_graphics_api_module_location(renderAPI);
		outModulePath = util::get_normalized_module_path(outLocation);
	};
	auto loadRenderApiModule = [this, &getRenderApiPath](const std::string &renderAPI, std::string &outErr) -> bool {
		std::string location;
		std::string modulePath;
		getRenderApiPath(renderAPI, location, modulePath);
		m_graphicsAPILib = util::load_library_module(modulePath, util::get_default_additional_library_search_directories(modulePath), {}, &outErr);
		return (m_graphicsAPILib != nullptr);
	};
	std::string err;
	if(loadRenderApiModule(renderAPI, err) == false) {
		Con::cwar << "Failed to load default render API '" << renderAPI << "': " << err << ". Falling back to alternatives..." << Con::endl;
		// Fallback
		SetRenderAPI("vulkan");
		if(loadRenderApiModule(renderAPI, err) == false) {
			SetRenderAPI("opengl");
			loadRenderApiModule(renderAPI, err);
		}
	}
	auto lib = m_graphicsAPILib;
	std::string location;
	std::string modulePath;
	getRenderApiPath(renderAPI, location, modulePath);

	//std::string err;
	if(lib != nullptr) {
		auto fInitRenderAPI = lib->FindSymbolAddress<bool (*)(const std::string &, bool, std::shared_ptr<prosper::IPrContext> &, std::string &)>("initialize_render_api");
		if(fInitRenderAPI == nullptr)
			err = "Symbol 'initialize_render_api' not found in library '" + location + "'!";
		else {
			std::string errMsg;
			auto title = g_customTitle.has_value() ? *g_customTitle : engine_info::get_name();
			auto success = fInitRenderAPI(title, false, m_renderContext, errMsg);
			if(success == false)
				err = errMsg;
		}
	}
	else
		err = "Module '" + modulePath + "' not found!";
	if(m_renderContext == nullptr)
		throw std::runtime_error {"Unable to load Vulkan implementation library for Prosper: " + err + "!"};

	m_renderContext->SetLogHandler(&pragma::log, &pragma::is_log_level_enabled);

	prosper::Callbacks callbacks {};
	callbacks.validationCallback = [this](prosper::DebugMessageSeverityFlags severityFlags, const std::string &message) { ValidationCallback(severityFlags, message); };
	callbacks.onWindowInitialized = [this]() { OnWindowInitialized(); };
	callbacks.onClose = [this]() { OnClose(); };
	callbacks.onResolutionChanged = [this](uint32_t w, uint32_t h) { OnResolutionChanged(w, h); };
	callbacks.drawFrame = [this]() { DrawFrame(); };
	m_renderContext->SetCallbacks(callbacks);
	if(umath::is_flag_set(m_stateFlags, StateFlags::GfxAPIValidationEnabled))
		m_renderContext->SetValidationEnabled(true);

	GetRenderContext().GetInitialWindowSettings().resizable = false;
	prosper::Shader::SetLogCallback([](prosper::Shader &shader, prosper::ShaderStage stage, const std::string &infoLog, const std::string &debugInfoLog) {
		std::stringstream msg;
		msg << "Unable to load shader '" << shader.GetIdentifier() << "':\r\n";
		msg << "Shader Stage: " << prosper::util::to_string(stage) << "\r\n";
		auto filePath = (stage != prosper::ShaderStage::Unknown) ? shader.GetStageSourceFilePath(stage) : std::optional<std::string> {};
		if(filePath.has_value())
			msg << "Shader Stage Filename: " << *filePath << "\r\n";
		msg << infoLog << "\r\n";
		msg << "\r\n";
		msg << debugInfoLog;
		spdlog::warn(msg.str());
	});
	prosper::debug::set_debug_validation_callback([](prosper::DebugReportObjectTypeEXT objectType, const std::string &msg) { spdlog::error("[prosper] {}", msg); });
	GLFW::initialize();
}
void RenderContext::Release()
{
	if(m_renderContext == nullptr)
		return;
	GetRenderContext().Close();
	m_renderContext = nullptr;
}
const prosper::IPrContext &RenderContext::GetRenderContext() const { return const_cast<RenderContext *>(this)->GetRenderContext(); }
prosper::IPrContext &RenderContext::GetRenderContext() { return *m_renderContext; }
prosper::ShaderManager &RenderContext::GetShaderManager() const { return GetRenderContext().GetShaderManager(); }
void RenderContext::RegisterShader(const std::string &identifier, const std::function<prosper::Shader *(prosper::IPrContext &, const std::string &)> &fFactory) { GetRenderContext().RegisterShader(identifier, fFactory); }
::util::WeakHandle<prosper::Shader> RenderContext::GetShader(const std::string &identifier) const { return GetRenderContext().GetShader(identifier); }

prosper::Window &RenderContext::GetWindow() { return GetRenderContext().GetWindow(); }
GLFW::Window &RenderContext::GetGlfwWindow() { return *GetRenderContext().GetWindow(); }
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &RenderContext::GetSetupCommandBuffer() { return GetRenderContext().GetSetupCommandBuffer(); }
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &RenderContext::GetDrawCommandBuffer() const { return GetRenderContext().GetWindow().GetDrawCommandBuffer(); }
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &RenderContext::GetDrawCommandBuffer(uint32_t swapchainIdx) const { return GetRenderContext().GetWindow().GetDrawCommandBuffer(swapchainIdx); }
void RenderContext::FlushSetupCommandBuffer() { GetRenderContext().FlushSetupCommandBuffer(); }
prosper::WindowSettings &RenderContext::GetInitialWindowSettings() { return GetRenderContext().GetInitialWindowSettings(); }
void RenderContext::SetValidationErrorDisabled(const std::string &id, bool disabled)
{
	if(!disabled) {
		auto it = m_disabledValidationErrors.find(id);
		if(it != m_disabledValidationErrors.end())
			m_disabledValidationErrors.erase(it);
		return;
	}
	m_disabledValidationErrors.insert(id);
}
bool RenderContext::IsValidationErrorDisabled(const std::string &id) const
{
	auto it = m_disabledValidationErrors.find(id);
	return (it != m_disabledValidationErrors.end());
}
void RenderContext::ValidationCallback(prosper::DebugMessageSeverityFlags severityFlags, const std::string &message)
{
	if((severityFlags & (prosper::DebugMessageSeverityFlags::ErrorBit | prosper::DebugMessageSeverityFlags::WarningBit)) != prosper::DebugMessageSeverityFlags::None) {
		std::string strMsg = message;
		// A VkImageStencilUsageCreateInfoEXT error is caused due to a bug in Anvil: https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/issues/153
		// We'll just ignore it for now, since it doesn't affect us in any way.
		// TODO: Remove this condition once the Anvil bug has been dealt with.
		if(strMsg.find("value of stencilUsage must not be 0. The Vulkan spec states: stencilUsage must not be 0") != std::string::npos)
			return;

		auto p = strMsg.find("[ VUID-");
		if(p == std::string::npos)
			p = strMsg.find("[ UNASSIGNED");
		if(p != std::string::npos) {
			p += 2;
			auto pEnd = strMsg.find(" ]", p);
			auto id = strMsg.substr(p, (pEnd != std::string::npos) ? (pEnd - p) : std::numeric_limits<size_t>::max());
			if(IsValidationErrorDisabled(id))
				return;
		}

		Con::cerr << "[PR] " << strMsg << Con::endl;
		if(std::this_thread::get_id() == c_engine->GetMainThreadId()) {
			// In many cases the error may have been caused by a Lua script, so we'll print
			// some information here about the current Lua call stack.
			auto *cl = c_engine->GetClientState();
			auto *game = cl ? static_cast<CGame *>(cl->GetGameState()) : nullptr;
			auto *l = game ? game->GetLuaState() : nullptr;
			if(l) {
				std::stringstream ss;
				if(Lua::get_callstack(l, ss))
					Con::cerr << "Lua callstack: " << ss.str() << Con::endl;
			}
		}
	}
}

void RenderContext::OnClose() {}

void RenderContext::OnResolutionChanged(uint32_t w, uint32_t h) {}
void RenderContext::DrawFrameCore() {}
void RenderContext::OnWindowInitialized()
{
	// TODO: Remove this function
}

void RenderContext::DrawFrame() { GetRenderContext().DrawFrameCore(); }

void RenderContext::SetGfxAPIValidationEnabled(bool b) { umath::set_flag(m_stateFlags, StateFlags::GfxAPIValidationEnabled, b); }
void RenderContext::SetRenderAPI(const std::string &renderAPI) { m_renderAPI = renderAPI; }
const std::string &RenderContext::GetRenderAPI() const { return m_renderAPI; }
