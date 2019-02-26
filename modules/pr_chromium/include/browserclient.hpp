#ifndef __BROWSERCLIENT_HPP__
#define __BROWSERCLIENT_HPP__

#include <include/cef_client.h>

class WebRenderHandler;
class WebBrowserClient
	: public CefClient
{
public:
	WebBrowserClient(WebRenderHandler *renderHandler);
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override;

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,CefProcessId source_process,CefRefPtr<CefProcessMessage> message) override;

	bool WasPageLoadedSuccessfully() const;
	bool HasPageLoadingStarted() const;
	
	void SetPageLoadedSuccessfully(bool b);
	void SetPageLoadingStarted(bool b);

	IMPLEMENT_REFCOUNTING(WebBrowserClient);
private:
	CefRefPtr<CefRenderHandler> m_renderHandler;
	bool m_bPageLoadedSuccessfully = false;
	bool m_bPageLoadingStarted = false;
};

#endif
