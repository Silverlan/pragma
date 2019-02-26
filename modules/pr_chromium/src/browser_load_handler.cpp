#include "browser_load_handler.hpp"
#include "browserclient.hpp"

void pragma::BrowserLoadHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,bool isLoading,bool canGoBack,bool canGoForward)
{
}
void pragma::BrowserLoadHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,TransitionType transition_type)
{
	browser->SendProcessMessage(PID_BROWSER,CefProcessMessage::Create("LoadStart"));
}
void pragma::BrowserLoadHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,int httpStatusCode)
{
	if(httpStatusCode == 200)
		browser->SendProcessMessage(PID_BROWSER,CefProcessMessage::Create("LoadComplete"));
}
void pragma::BrowserLoadHandler::OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString& errorText,const CefString& failedUrl)
{
}
