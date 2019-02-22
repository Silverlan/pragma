#ifndef __LUA_DOC_HPP__
#define __LUA_DOC_HPP__

namespace Lua
{
	namespace doc
	{
		DLLENGINE bool load_documentation_file(const std::string &fileName);
		DLLENGINE void print_documentation(const std::string &name);
	};
};

#endif
