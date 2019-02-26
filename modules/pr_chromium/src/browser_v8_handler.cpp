#include "browser_v8_handler.hpp"
#include "util_javascript.hpp"
#include <iostream>

static CefRefPtr<CefV8Value> js_to_cef_value(const pragma::JSValue &jsVal)
{
	switch(jsVal.type)
	{
		case pragma::JSValueType::Null:
			return CefV8Value::CreateNull();
		case pragma::JSValueType::Bool:
			return CefV8Value::CreateBool(*static_cast<bool*>(jsVal.data.get()));
		case pragma::JSValueType::Int:
			return CefV8Value::CreateInt(*static_cast<int32_t*>(jsVal.data.get()));
		case pragma::JSValueType::Double:
			return CefV8Value::CreateDouble(*static_cast<double*>(jsVal.data.get()));
		case pragma::JSValueType::Date:
			return CefV8Value::CreateDate({});
		case pragma::JSValueType::String:
			return CefV8Value::CreateString(*static_cast<std::string*>(jsVal.data.get()));
		case pragma::JSValueType::Object:
		case pragma::JSValueType::Array:
		case pragma::JSValueType::Function:
		case pragma::JSValueType::Undefined:
		default:
			return CefV8Value::CreateUndefined();
	}
}

static pragma::JSValue cef_to_js_value(CefV8Value &v8Val)
{
	pragma::JSValue v{};
	if(v8Val.IsNull() == true)
	{
		v.type = pragma::JSValueType::Null;
		v.data = nullptr;
	}
	else if(v8Val.IsBool() == true)
	{
		v.type = pragma::JSValueType::Bool;
		v.data = std::make_shared<bool>(v8Val.GetBoolValue());
	}
	else if(v8Val.IsInt() == true)
	{
		v.type = pragma::JSValueType::Int;
		v.data = std::make_shared<int32_t>(v8Val.GetIntValue());
	}
	else if(v8Val.IsDouble() == true)
	{
		v.type = pragma::JSValueType::Double;
		v.data = std::make_shared<double>(v8Val.GetDoubleValue());
	}
	else if(v8Val.IsDate() == true)
	{
		v.type = pragma::JSValueType::Date;
		v.data = nullptr;
	}
	else if(v8Val.IsString() == true)
	{
		v.type = pragma::JSValueType::String;
		v.data = std::make_shared<std::string>(v8Val.GetStringValue());
	}
	else if(v8Val.IsObject() == true)
	{
		v.type = pragma::JSValueType::Object;
		v.data = nullptr;
	}
	else if(v8Val.IsArray() == true)
	{
		v.type = pragma::JSValueType::Array;
		v.data = nullptr;
	}
	else if(v8Val.IsFunction() == true)
	{
		v.type = pragma::JSValueType::Function;
		v.data = nullptr;
	}
	else
	{
		v.type = pragma::JSValueType::Undefined;
		v.data = nullptr;
	}
	return v;
}

namespace pragma
{
	extern std::vector<JavaScriptFunction> globalJavaScriptFunctions;
};
bool pragma::BrowserV8Handler::Execute(
	const CefString &name,
	CefRefPtr<CefV8Value> object,
	const CefV8ValueList& arguments,
	CefRefPtr<CefV8Value>& retval,
	CefString& exception
)
{
	for(auto &jsf : pragma::globalJavaScriptFunctions)
	{
		if(name == jsf.name)
		{
			std::vector<pragma::JSValue> jsArgs;
			jsArgs.reserve(arguments.size());
			for(auto &arg : arguments)
				jsArgs.push_back(cef_to_js_value(*arg));

			auto jsRetVal = jsf.callback(jsArgs);
			if(jsRetVal != nullptr)
				retval = js_to_cef_value(*jsRetVal);
			return true;
		}
	}
	return false;
}
