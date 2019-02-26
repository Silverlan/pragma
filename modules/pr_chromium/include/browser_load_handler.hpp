#ifndef __BROWSER_LOAD_HANDLER_HPP__
#define __BROWSER_LOAD_HANDLER_HPP__

#include <include/cef_load_handler.h>

namespace pragma
{
	class BrowserLoadHandler
		: public CefLoadHandler
	{
	public:
		virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,bool isLoading,bool canGoBack,bool canGoForward) override;
		virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,TransitionType transition_type) override;
		virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,int httpStatusCode) override;
		virtual void OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString& errorText,const CefString& failedUrl) override;
	private:
		IMPLEMENT_REFCOUNTING(BrowserLoadHandler);
	};
};

#endif
