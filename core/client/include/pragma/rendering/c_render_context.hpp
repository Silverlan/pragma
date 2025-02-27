/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDER_CONTEXT_HPP__
#define __C_RENDER_CONTEXT_HPP__

#include "pragma/c_enginedefinitions.h"
#include <sharedutils/util_library.hpp>
#include <prosper_context.hpp>
#include <unordered_set>
#include <memory>
#include <optional>

import pragma.platform;

#undef CreateWindow

#pragma warning(push)
#pragma warning(disable : 4251)
namespace util {
	class Library;
};
namespace prosper {
	class Window;
	struct WindowSettings;
};
namespace pragma {
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
		::util::WeakHandle<prosper::Shader> GetShader(const std::string &identifier) const;

		prosper::Window &GetWindow();
		pragma::platform::Window &GetGlfwWindow();
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
		std::shared_ptr<::util::Library> m_graphicsAPILib = nullptr;
		std::unique_ptr<pragma::platform::Monitor> m_monitor = nullptr;
		std::string m_renderAPI;
	};
}
REGISTER_BASIC_BITWISE_OPERATORS(pragma::RenderContext::StateFlags)
#pragma warning(pop)

#endif
