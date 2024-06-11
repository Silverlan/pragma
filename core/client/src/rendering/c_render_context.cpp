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
#include <sharedutils/util_debug.h>
#include <pragma/util/util_module.hpp>
#include <pragma/lua/lua_error_handling.hpp>
#include <pragma/logging.hpp>
#include <prosper_window.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
static spdlog::logger &LOGGER = pragma::register_logger("prosper");
static spdlog::logger &LOGGER_VALIDATION = pragma::register_logger("prosper_validation");

RenderContext::RenderContext() : m_monitor(nullptr), m_renderAPI {"vulkan"} {}
RenderContext::~RenderContext() { m_graphicsAPILib = nullptr; }
DLLNETWORK std::optional<std::string> g_customTitle;
extern bool g_cpuRendering;
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

		auto additionalSearchDirectories = util::get_default_additional_library_search_directories(modulePath);
		if(g_cpuRendering) {
			if(renderAPI == "vulkan") {
				if(filemanager::exists("modules/swiftshader/")) {
					auto p = util::Path::CreatePath(util::get_program_path());
					p += "modules/swiftshader/";
					additionalSearchDirectories.push_back(p.GetString());

					spdlog::info("-cpu_rendering option has been specified. SwiftShader will be used for rendering instead of Vulkan driver.");
				}
				else
					spdlog::error("-cpu_rendering option requires SwiftShader module, which is not installed! Ignoring option...");
			}
			else
				spdlog::error("-cpu_rendering option is only supported for Vulkan render API! Ignoring option...");
		}
		m_graphicsAPILib = util::load_library_module(modulePath, additionalSearchDirectories, {}, &outErr);
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
	if(m_renderContext == nullptr) {
		std::string msg = "Unable to load Vulkan implementation library for Prosper: " + err + "!";
		LOGGER.error(msg);
		throw std::runtime_error {msg};
	}

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
		LOGGER.warn(msg.str());
	});
	prosper::debug::set_debug_validation_callback([](prosper::DebugReportObjectTypeEXT objectType, const std::string &msg) { LOGGER_VALIDATION.error("{}", msg); });
	GLFW::initialize();

	if(GetRenderContext().IsValidationEnabled()) {
		// A VkImageStencilUsageCreateInfoEXT error is caused due to a bug in Anvil: https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/issues/153
		// We'll just ignore it for now, since it doesn't affect us in any way.
		// TODO: Remove this condition once the Anvil bug has been dealt with.
		SetValidationErrorDisabled("VUID-VkImageStencilUsageCreateInfo-stencilUsage-requiredbitmask", true);

		SetValidationErrorDisabled("VUID-VkMappedMemoryRange-size-01390", true);
	}
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

		LOGGER_VALIDATION.error(strMsg);
		if(std::this_thread::get_id() == c_engine->GetMainThreadId()) {
			// In many cases the error may have been caused by a Lua script, so we'll print
			// some information here about the current Lua call stack.
			auto *cl = c_engine->GetClientState();
			auto *game = cl ? static_cast<CGame *>(cl->GetGameState()) : nullptr;
			auto *l = game ? game->GetLuaState() : nullptr;
			if(l) {
				std::stringstream ss;
				if(Lua::get_callstack(l, ss))
					LOGGER_VALIDATION.error("Lua callstack: {}", ss.str());
			}
		}
#ifdef _WIN32
		auto stackBacktraceString = util::get_formatted_stack_backtrace_string();
		if(!stackBacktraceString.empty())
			LOGGER_VALIDATION.debug("Backtrace: {}", stackBacktraceString);
#endif
		pragma::flush_loggers();
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

void RenderContext::SetGfxAPIValidationEnabled(bool b)
{
	umath::set_flag(m_stateFlags, StateFlags::GfxAPIValidationEnabled, b);
	if(b)
		spdlog::flush_on(spdlog::level::info); // Immediately flush all messages
}
void RenderContext::SetRenderAPI(const std::string &renderAPI) { m_renderAPI = renderAPI; }
const std::string &RenderContext::GetRenderAPI() const { return m_renderAPI; }
