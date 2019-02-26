#include "browserclient.hpp"
#include "renderer.hpp"

WebBrowserClient::WebBrowserClient(WebRenderHandler *renderHandler)
	: m_renderHandler(renderHandler)
{}
bool WebBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,CefProcessId source_process,CefRefPtr<CefProcessMessage> message)
{
	auto msgName = message->GetName().ToString();
	if(msgName == "LoadStart")
		SetPageLoadingStarted(true);
	else if(msgName == "LoadComplete")
		SetPageLoadedSuccessfully(true);
	else
		return false;
	return true;
}
CefRefPtr<CefRenderHandler> WebBrowserClient::GetRenderHandler() {return m_renderHandler;}

bool WebBrowserClient::HasPageLoadingStarted() const {return m_bPageLoadingStarted;}
bool WebBrowserClient::WasPageLoadedSuccessfully() const {return m_bPageLoadedSuccessfully;}

void WebBrowserClient::SetPageLoadedSuccessfully(bool b) {m_bPageLoadedSuccessfully = b;}
void WebBrowserClient::SetPageLoadingStarted(bool b) {m_bPageLoadingStarted = b;}
