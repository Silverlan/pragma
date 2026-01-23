// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.render_context;
export import pragma.platform;
export import pragma.prosper;

#undef CreateWindow

#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma::rendering {
	class DLLCLIENT RenderContext {
	  public:
		enum class StateFlags : uint8_t {
			None = 0u,
			GfxAPIValidationEnabled = 1u,
			GfxDiagnosticsModeEnabled = GfxAPIValidationEnabled << 1u,
		};
		RenderContext();
		virtual ~RenderContext();

		void Release();

		const prosper::IPrContext &GetRenderContext() const;
		prosper::IPrContext &GetRenderContext();
		prosper::ShaderManager &GetShaderManager() const;
		void RegisterShader(const std::string &identifier, const std::function<prosper::Shader *(prosper::IPrContext &, const std::string &)> &fFactory);
		util::WeakHandle<prosper::Shader> GetShader(const std::string &identifier) const;

		prosper::Window &GetWindow();
		platform::Window &GetGlfwWindow();
		const std::shared_ptr<prosper::IPrimaryCommandBuffer> &GetSetupCommandBuffer();
		const std::shared_ptr<prosper::IPrimaryCommandBuffer> &GetDrawCommandBuffer() const;
		const std::shared_ptr<prosper::IPrimaryCommandBuffer> &GetDrawCommandBuffer(uint32_t swapchainIdx) const;
		void FlushSetupCommandBuffer();

		prosper::WindowSettings &GetInitialWindowSettings();
		const prosper::WindowSettings &GetInitialWindowSettings() const { return const_cast<RenderContext *>(this)->GetInitialWindowSettings(); }

		void InitializeRenderAPI();
		void SetGfxAPIValidationEnabled(bool b);
		void SetGfxDiagnosticsModeEnabled(bool b);
		bool IsGfxAPIValidationEnabled() const;
		bool IsGfxDiagnosticsModeEnabled() const;
		void SetRenderAPI(const std::string &renderAPI);
		const std::string &GetRenderAPI() const;

		void SetValidationErrorDisabled(const std::string &id, bool disabled);
		bool IsValidationErrorDisabled(const std::string &id) const;
	  protected:
		virtual void OnClose();
		virtual void DrawFrameCore();
		virtual void OnWindowInitialized();
		virtual void OnResolutionChanged(uint32_t w, uint32_t h);
		virtual void DrawFrame();
		void ValidationCallback(prosper::DebugMessageSeverityFlags severityFlags, const std::string &message);
	  private:
		std::shared_ptr<prosper::IPrContext> m_renderContext = nullptr;
		std::unordered_set<std::string> m_disabledValidationErrors;
		StateFlags m_stateFlags = StateFlags::None;
		std::shared_ptr<util::Library> m_graphicsAPILib = nullptr;
		std::unique_ptr<platform::Monitor> m_monitor = nullptr;
		std::string m_renderAPI;
	};
	using namespace pragma::math::scoped_enum::bitwise;
}
export {
	REGISTER_ENUM_FLAGS(pragma::rendering::RenderContext::StateFlags)
};
#pragma warning(pop)
