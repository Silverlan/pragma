#include "browser_render_process_handler.hpp"
#include "browser_v8_handler.hpp"
#include "util_javascript.hpp"
#include "browser_load_handler.hpp"

namespace pragma
{
	extern std::vector<JavaScriptFunction> globalJavaScriptFunctions;
};

CefRefPtr<CefV8Handler> pragma::BrowserRenderProcessHandler::GetV8Handler() {return m_v8Handler;}
CefRefPtr<CefV8Context> pragma::BrowserRenderProcessHandler::GetContext() {return m_context;}

void pragma::BrowserRenderProcessHandler::OnWebKitInitialized()
{

}
CefRefPtr<CefLoadHandler> pragma::BrowserRenderProcessHandler::GetLoadHandler()
{
	if(m_loadHandler == nullptr)
		m_loadHandler = new BrowserLoadHandler();
	return m_loadHandler;
}
void pragma::BrowserRenderProcessHandler::OnContextCreated(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,CefRefPtr<CefV8Context> context)
{
	m_context = context;

	auto object = context->GetGlobal();
	m_v8Handler = new BrowserV8Handler();
	for(auto &jsf : globalJavaScriptFunctions)
	{
		auto func = CefV8Value::CreateFunction(jsf.name,m_v8Handler);

		object->SetValue(jsf.name,func,V8_PROPERTY_ATTRIBUTE_NONE);
	}
}
