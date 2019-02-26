#ifndef __BROWSERRENDERPROCESSHANDLER_HPP__
#define __BROWSERRENDERPROCESSHANDLER_HPP__

#include <include/cef_base.h>
#include <include/cef_render_process_handler.h>

namespace pragma
{
	class BrowserRenderProcessHandler
		: public CefRenderProcessHandler
	{
	public:
		virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,CefRefPtr<CefV8Context> context) override;
		virtual void OnWebKitInitialized() override;
		virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;
		CefRefPtr<CefV8Handler> GetV8Handler();
		CefRefPtr<CefV8Context> GetContext();
	private:
		CefRefPtr<CefV8Handler> m_v8Handler = nullptr;
		CefRefPtr<CefV8Context> m_context = nullptr;
		CefRefPtr<CefLoadHandler> m_loadHandler = nullptr;
		IMPLEMENT_REFCOUNTING(BrowserRenderProcessHandler);
	};
};

#endif
