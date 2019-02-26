#ifndef __UTIL_JAVASCRIPT_HPP__
#define __UTIL_JAVASCRIPT_HPP__

#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace pragma
{
	enum class JSValueType : uint32_t
	{
		Undefined = 0,
		Null,
		Bool,
		Int,
		Double,
		Date,
		String,
		Object,
		Array,
		Function
	};

	struct JSValue
	{
		JSValueType type;
		std::shared_ptr<void> data = nullptr;
	};

	struct JavaScriptFunction
	{
		std::string name;
		std::function<std::unique_ptr<JSValue>(const std::vector<JSValue>&)> callback = nullptr;
	};
};

#endif
