/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_RENDER_CONTEXT_HPP__
#define __C_RENDER_CONTEXT_HPP__

#include "pragma/c_enginedefinitions.h"
#include <prosper_context.hpp>
#include <iglfw/glfw_window.h>
#include <memory>
#include <optional>

#undef CreateWindow

#pragma warning(push)
#pragma warning(disable : 4251)
namespace util {class Library;};
namespace pragma
{
	class DLLCENGINE RenderContext
	{
	public:
		enum class StateFlags : uint8_t
		{
			None = 0u,
			WindowedMode = 1u,
			GfxAPIValidationEnabled = WindowedMode<<1u
		};
		RenderContext();
		virtual ~RenderContext();

		void Release();

		const prosper::IPrContext &GetRenderContext() const;
		prosper::IPrContext &GetRenderContext();
		prosper::ShaderManager &GetShaderManager() const;
		::util::WeakHandle<prosper::Shader> RegisterShader(const std::string &identifier,const std::function<prosper::Shader*(prosper::IPrContext&,const std::string&)> &fFactory);
		::util::WeakHandle<prosper::Shader> GetShader(const std::string &identifier) const;

		GLFW::Window &GetWindow();
		const std::shared_ptr<prosper::IPrimaryCommandBuffer> &GetSetupCommandBuffer();
		const std::shared_ptr<prosper::IPrimaryCommandBuffer> &GetDrawCommandBuffer() const;
		const std::shared_ptr<prosper::IPrimaryCommandBuffer> &GetDrawCommandBuffer(uint32_t swapchainIdx) const;
		void FlushSetupCommandBuffer();
		
		void InitializeRenderAPI();
		void SetGfxAPIValidationEnabled(bool b);
		void SetWindowedMode(bool b);
		void SetRefreshRate(uint32_t rate);
		void SetNoBorder(bool b);
		void SetResolution(const Vector2i &sz);
		void SetResolutionWidth(uint32_t w);
		void SetResolutionHeight(uint32_t h);
		void SetMonitor(GLFW::Monitor &monitor);
		void SetPresentMode(prosper::PresentModeKHR presentMode);
		float GetAspectRatio() const;
		void SetRenderAPI(const std::string &renderAPI);
		const std::string &GetRenderAPI() const;
	protected:
		void UpdateWindow();
		virtual void OnClose();
		virtual void DrawFrame();
		virtual void OnWindowInitialized();
		virtual void OnResolutionChanged(uint32_t w,uint32_t h);
		virtual void DrawFrame(prosper::IPrimaryCommandBuffer &drawCmd,uint32_t swapchainImageIdx);
		void ValidationCallback(
			prosper::DebugMessageSeverityFlags severityFlags,
			const std::string &message
		);
	private:
		struct WindowChangeInfo
		{
			std::optional<bool> windowedMode = {};
			std::optional<uint32_t> refreshRate = {};
			std::optional<bool> decorated = {};
			std::optional<uint32_t> width = {};
			std::optional<uint32_t> height = {};
			std::optional<std::unique_ptr<GLFW::Monitor>> monitor = {};
			std::optional<prosper::PresentModeKHR> presentMode = {};
		};
		WindowChangeInfo &ScheduleWindowReload();
		std::shared_ptr<prosper::IPrContext> m_renderContext = nullptr;
		std::unique_ptr<WindowChangeInfo> m_scheduledWindowReloadInfo = nullptr;
		StateFlags m_stateFlags = StateFlags::None;
		float m_aspectRatio = 1.f;
		std::shared_ptr<util::Library> m_graphicsAPILib = nullptr;
		std::unique_ptr<GLFW::Monitor> m_monitor = nullptr;
		std::string m_renderAPI = "vulkan";
	};
}
REGISTER_BASIC_BITWISE_OPERATORS(pragma::RenderContext::StateFlags)
#pragma warning(pop)

#endif
