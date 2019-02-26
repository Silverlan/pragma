#include "browser_process.hpp"
#include "browser_render_process_handler.hpp"

pragma::BrowserProcess::BrowserProcess()
	: m_renderProcessHandler(new BrowserRenderProcessHandler())
{}

CefRefPtr<CefRenderProcessHandler> pragma::BrowserProcess::GetRenderProcessHandler() {return m_renderProcessHandler;}
