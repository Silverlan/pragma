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
namespace pragma
{
	class DLLCENGINE RenderContext
		: public prosper::Context
	{
	public:
		RenderContext();
		virtual ~RenderContext() override;
		
		void SetWindowedMode(bool b);
		void SetRefreshRate(uint32_t rate);
		void SetNoBorder(bool b);
		void SetResolution(const Vector2i &sz);
		void SetMonitor(GLFW::Monitor &monitor);
		void SetPresentMode(Anvil::PresentModeKHR presentMode);
		float GetAspectRatio() const;
		using prosper::Context::DrawFrame;
	protected:
		void UpdateWindow();
		virtual void DrawFrame() override;
		virtual void OnWindowInitialized() override;
		virtual VkBool32 ValidationCallback(
			Anvil::DebugMessageSeverityFlags severityFlags,
			const char *message
		) override;
	private:
		struct WindowChangeInfo
		{
			std::optional<bool> windowedMode = {};
			std::optional<uint32_t> refreshRate = {};
			std::optional<bool> decorated = {};
			std::optional<uint32_t> width = {};
			std::optional<uint32_t> height = {};
			std::optional<std::unique_ptr<GLFW::Monitor>> monitor = {};
			std::optional<Anvil::PresentModeKHR> presentMode = {};
		};
		WindowChangeInfo &ScheduleWindowReload();
		std::unique_ptr<WindowChangeInfo> m_scheduledWindowReloadInfo = nullptr;
		bool m_bWindowedMode = false;
		float m_aspectRatio = 1.f;
		std::unique_ptr<GLFW::Monitor> m_monitor = nullptr;
	};
}
#pragma warning(pop)

#endif
