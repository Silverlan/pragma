#ifndef __BROWSER_V8_HANDLER_HPP__
#define __BROWSER_V8_HANDLER_HPP__

#include <include/cef_v8.h>

namespace pragma
{
	class BrowserV8Handler
		: public CefV8Handler
	{
	public:
		BrowserV8Handler()=default;
		virtual bool Execute(
			const CefString &name,
			CefRefPtr<CefV8Value> object,
			const CefV8ValueList& arguments,
			CefRefPtr<CefV8Value>& retval,
			CefString& exception
		) override;
	private:
		IMPLEMENT_REFCOUNTING(BrowserV8Handler);
	};
};

#endif
