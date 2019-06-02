#ifndef __LUA_DOC_HPP__
#define __LUA_DOC_HPP__

namespace pragma {namespace doc {class BaseCollectionObject;};};
namespace Lua
{
	namespace doc
	{
		DLLENGINE bool load_documentation_file(const std::string &fileName);
		DLLENGINE void print_documentation(const std::string &name);
		DLLENGINE void find_candidates(const std::string &name,std::vector<const pragma::doc::BaseCollectionObject*> &outCandidates,uint32_t candidateLimit);
	};
};

#endif
