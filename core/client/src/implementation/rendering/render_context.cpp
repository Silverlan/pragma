// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.render_apis;
import :rendering.render_context;
import :engine;

using namespace pragma;

static spdlog::logger &LOGGER = register_logger("prosper");
static spdlog::logger &LOGGER_VALIDATION = register_logger("prosper_validation");

rendering::RenderContext::RenderContext() : m_monitor(nullptr), m_renderAPI {"vulkan"} {}
rendering::RenderContext::~RenderContext() {}
std::optional<std::string> g_customTitle;
extern bool g_cpuRendering;
void rendering::RenderContext::InitializeRenderAPI()
{
	auto &renderAPI = GetRenderAPI();
	auto getRenderApiPath = [](const std::string &renderAPI, std::string &outLocation, std::string &outModulePath) {
		outLocation = get_graphics_api_module_location(renderAPI);
		outModulePath = util::get_normalized_module_path(outLocation);
	};
	auto loadRenderApiModule = [this, &getRenderApiPath](const std::string &renderAPI, std::string &outErr) -> bool {
		std::string location;
		std::string modulePath;
		getRenderApiPath(renderAPI, location, modulePath);

		auto additionalSearchDirectories = util::get_default_additional_library_search_directories(modulePath);
		if(g_cpuRendering) {
			if(renderAPI == "vulkan") {
				if(fs::exists("modules/swiftshader/")) {
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
#ifdef __linux__
			// Unfortunately 'additionalSearchDirectories' does not work on Linux, so we'll have to load
			// the swiftshader library manually before the render module is loaded, otherwise the system
			// vulkan driver will be used instead of swiftshader.
			std::string absSwiftshaderPath;
			std::string relSwiftShaderPath = "modules/swiftshader/libvulkan.so.1";
			if(fs::find_absolute_path(relSwiftShaderPath, absSwiftshaderPath)) {
				std::string err;
				auto libVulkan = util::Library::Load(absSwiftshaderPath, {}, &err);
				if(libVulkan)
					libVulkan->SetDontFreeLibraryOnDestruct();
				else
					spdlog::error("Failed to load swiftshader library '{}': {}. This will likely cause issues.", absSwiftshaderPath, err);
			}
			else
				spdlog::error("Failed to locate swiftshader library {}! This will likely cause issues.", relSwiftShaderPath);
#endif
		}
		m_graphicsAPILib = util::load_library_module(modulePath, additionalSearchDirectories, {}, &outErr);
		return (m_graphicsAPILib != nullptr);
	};
	std::string err;
	if(loadRenderApiModule(renderAPI, err) == false) {
		Con::CWAR << "Failed to load default render API '" << renderAPI << "': " << err << ". Falling back to alternatives..." << Con::endl;
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

	m_renderContext->SetLogHandler(&log, &is_log_level_enabled);
	m_renderContext->SetProfilingHandler([](const char *taskName) { debug::start_profiling_task(taskName); }, []() { debug::end_profiling_task(); });

	prosper::Callbacks callbacks {};
	callbacks.validationCallback = [this](prosper::DebugMessageSeverityFlags severityFlags, const std::string &message) { ValidationCallback(severityFlags, message); };
	callbacks.onWindowInitialized = [this]() { OnWindowInitialized(); };
	callbacks.onClose = [this]() { OnClose(); };
	callbacks.onResolutionChanged = [this](uint32_t w, uint32_t h) { OnResolutionChanged(w, h); };
	callbacks.drawFrame = [this]() { DrawFrame(); };
	m_renderContext->SetCallbacks(callbacks);
	if(math::is_flag_set(m_stateFlags, StateFlags::GfxAPIValidationEnabled))
		m_renderContext->SetValidationEnabled(true);

	material::CMaterialManager::SetFlipTexturesVerticallyOnLoad(m_renderContext->ShouldFlipTexturesOnLoad());

	GetRenderContext().GetInitialWindowSettings().resizable = false;
	prosper::Shader::SetLogCallback([](prosper::Shader &shader, prosper::ShaderStage stage, const std::string &infoLog, const std::string &debugInfoLog) {
		std::stringstream msg;
		std::string eol;
#ifdef MSVC_COMPILER_FIX
		eol = "\r\n";
#else
		eol = spdlog::details::default_eol;
#endif
		msg << "Unable to load shader '" << shader.GetIdentifier() << "':" << eol;
		msg << "Shader Stage: " << prosper::util::to_string(stage) << eol;
		auto filePath = (stage != prosper::ShaderStage::Unknown) ? shader.GetStageSourceFilePath(stage) : std::optional<std::string> {};
		if(filePath.has_value())
			msg << "Shader Stage Filename: " << *filePath << eol;
		auto infoLogConv = infoLog;
		string::replace(infoLogConv, "\n", eol);
		msg << infoLogConv << eol;
		msg << eol;
		auto debugInfoLogConv = debugInfoLog;
		string::replace(debugInfoLogConv, "\n", eol);
		msg << debugInfoLogConv;
		LOGGER.warn(msg.str());
	});
	prosper::debug::set_debug_validation_callback([](prosper::DebugReportObjectTypeEXT objectType, const std::string &msg) { LOGGER_VALIDATION.error("{}", msg); });
	err.clear();
	if(!platform::initialize(err, get_cengine()->IsWindowless())) {
		LOGGER.critical("Failed to initialize GLFW: {}", err);
		throw std::runtime_error {"Failed to initialize GLFW: " + err};
	}

	if(GetRenderContext().IsValidationEnabled()) {
		// A VkImageStencilUsageCreateInfoEXT error is caused due to a bug in Anvil: https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/issues/153
		// We'll just ignore it for now, since it doesn't affect us in any way.
		// TODO: Remove this condition once the Anvil bug has been dealt with.
		SetValidationErrorDisabled("VUID-VkImageStencilUsageCreateInfo-stencilUsage-requiredbitmask", true);

		SetValidationErrorDisabled("VUID-VkMappedMemoryRange-size-01390", true);
	}
}
void rendering::RenderContext::Release()
{
	if(m_renderContext == nullptr)
		return;
	GetRenderContext().Close();
	m_renderContext = nullptr;

	if(m_graphicsAPILib) {
		auto *detach = m_graphicsAPILib->FindSymbolAddress<void (*)()>("pragma_detach");
		if(detach)
			detach();
	}
	m_graphicsAPILib = nullptr;
}
const prosper::IPrContext &rendering::RenderContext::GetRenderContext() const { return const_cast<RenderContext *>(this)->GetRenderContext(); }
prosper::IPrContext &rendering::RenderContext::GetRenderContext() { return *m_renderContext; }
prosper::ShaderManager &rendering::RenderContext::GetShaderManager() const { return GetRenderContext().GetShaderManager(); }
void rendering::RenderContext::RegisterShader(const std::string &identifier, const std::function<prosper::Shader *(prosper::IPrContext &, const std::string &)> &fFactory) { GetRenderContext().RegisterShader(identifier, fFactory); }
util::WeakHandle<prosper::Shader> rendering::RenderContext::GetShader(const std::string &identifier) const { return GetRenderContext().GetShader(identifier); }

prosper::Window &rendering::RenderContext::GetWindow() { return GetRenderContext().GetWindow(); }
platform::Window &rendering::RenderContext::GetGlfwWindow() { return *GetRenderContext().GetWindow(); }
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &rendering::RenderContext::GetSetupCommandBuffer() { return GetRenderContext().GetSetupCommandBuffer(); }
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &rendering::RenderContext::GetDrawCommandBuffer() const { return GetRenderContext().GetWindow().GetDrawCommandBuffer(); }
const std::shared_ptr<prosper::IPrimaryCommandBuffer> &rendering::RenderContext::GetDrawCommandBuffer(uint32_t swapchainIdx) const { return GetRenderContext().GetWindow().GetDrawCommandBuffer(swapchainIdx); }
void rendering::RenderContext::FlushSetupCommandBuffer() { GetRenderContext().FlushSetupCommandBuffer(); }
prosper::WindowSettings &rendering::RenderContext::GetInitialWindowSettings() { return GetRenderContext().GetInitialWindowSettings(); }
void rendering::RenderContext::SetValidationErrorDisabled(const std::string &id, bool disabled)
{
	if(!disabled) {
		auto it = m_disabledValidationErrors.find(id);
		if(it != m_disabledValidationErrors.end())
			m_disabledValidationErrors.erase(it);
		return;
	}
	m_disabledValidationErrors.insert(id);
}
bool rendering::RenderContext::IsValidationErrorDisabled(const std::string &id) const
{
	auto it = m_disabledValidationErrors.find(id);
	return (it != m_disabledValidationErrors.end());
}
void rendering::RenderContext::ValidationCallback(prosper::DebugMessageSeverityFlags severityFlags, const std::string &message)
{
	std::string strMsg = message;

	if(strMsg.find("No optimized version") != std::string::npos)
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

	if(math::is_flag_set(severityFlags, prosper::DebugMessageSeverityFlags::ErrorBit))
		LOGGER_VALIDATION.error(strMsg);
	else if(math::is_flag_set(severityFlags, prosper::DebugMessageSeverityFlags::WarningBit))
		LOGGER_VALIDATION.warn(strMsg);
	else if(math::is_flag_set(severityFlags, prosper::DebugMessageSeverityFlags::InfoBit))
		LOGGER_VALIDATION.info(strMsg);
	else if(math::is_flag_set(severityFlags, prosper::DebugMessageSeverityFlags::VerboseBit))
		LOGGER_VALIDATION.debug(strMsg);
	else
		LOGGER_VALIDATION.trace(strMsg);

	if(math::is_flag_set(severityFlags, prosper::DebugMessageSeverityFlags::WarningBit | prosper::DebugMessageSeverityFlags::ErrorBit)) {
		auto stackBacktraceString = debug::get_formatted_stack_backtrace_string();
		if(!stackBacktraceString.empty()) {
			string::replace(stackBacktraceString, "\n", std::string {util::LOG_NL});
			LOGGER_VALIDATION.debug("Backtrace: {}", stackBacktraceString);
		}
	}
	flush_loggers();
}

void rendering::RenderContext::OnClose() {}

void rendering::RenderContext::OnResolutionChanged(uint32_t w, uint32_t h) {}
void rendering::RenderContext::DrawFrameCore() {}
void rendering::RenderContext::OnWindowInitialized()
{
	// TODO: Remove this function
}

void rendering::RenderContext::DrawFrame() { GetRenderContext().DrawFrameCore(); }

void rendering::RenderContext::SetGfxAPIValidationEnabled(bool b)
{
	math::set_flag(m_stateFlags, StateFlags::GfxAPIValidationEnabled, b);
	if(b)
		spdlog::flush_on(spdlog::level::info); // Immediately flush all messages
}
void rendering::RenderContext::SetGfxDiagnosticsModeEnabled(bool b) { math::set_flag(m_stateFlags, StateFlags::GfxDiagnosticsModeEnabled, b); }
bool rendering::RenderContext::IsGfxAPIValidationEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::GfxAPIValidationEnabled); }
bool rendering::RenderContext::IsGfxDiagnosticsModeEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::GfxDiagnosticsModeEnabled); }

void rendering::RenderContext::SetRenderAPI(const std::string &renderAPI) { m_renderAPI = renderAPI; }
const std::string &rendering::RenderContext::GetRenderAPI() const { return m_renderAPI; }
