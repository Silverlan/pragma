#ifndef __CEFPROCESS_HPP__
#define __CEFPROCESS_HPP__

#include <include/cef_app.h>

namespace pragma
{
	class BrowserProcess
		: public CefApp
	{
	public:
		BrowserProcess();
		virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;
	private:
		IMPLEMENT_REFCOUNTING(BrowserProcess);
		CefRefPtr<CefRenderProcessHandler> m_renderProcessHandler = nullptr;
	};
};

#endif
