/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LUA_DOC_HPP__
#define __LUA_DOC_HPP__

namespace pragma {namespace doc {class BaseCollectionObject;};};
namespace Lua
{
	namespace doc
	{
		DLLNETWORK bool load_documentation_file(const std::string &fileName);
		DLLNETWORK void print_documentation(const std::string &name);
		DLLNETWORK void find_candidates(const std::string &name,std::vector<const pragma::doc::BaseCollectionObject*> &outCandidates,uint32_t candidateLimit);
	};
};

#endif
