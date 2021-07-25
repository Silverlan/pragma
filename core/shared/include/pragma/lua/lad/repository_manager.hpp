/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_AUTO_DOC_REPOSITORY_MANAGER_HPP__
#define __LUA_AUTO_DOC_REPOSITORY_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include <vector>
#include <string>

namespace pragma::lua
{
	class RepositoryManager
	{
	public:
		static constexpr auto REPOSITORY_CHECKOUT_URL = "https://github.com/Silverlan/pragma.git";
		static constexpr auto REPOSITORY_URL = "https://github.com/Silverlan/pragma/blob/";
		static constexpr auto REPOSITORY_BRANCH = "master";
		static std::unique_ptr<RepositoryManager> Create(lua_State *l,std::string &outErr);
		bool LoadRepositoryReferences(std::string &outErr);
	private:
		RepositoryManager()=default;
		void ClearLocalRepositoryDir();
		static void CollectSourceCodeFiles(const std::string &sourceCodeLocation,std::vector<std::string> &outFiles);

		using GitClone = bool(*)(
			const std::string&,const std::string&,
			const std::vector<std::string>&,const std::string&,std::string&,
			std::string*
		);

		GitClone m_gitClone = nullptr;
		std::string m_repositoryRootDir = "core/";
		std::string m_tmpRepositoryLocation = "temp/plad_repo/";
		std::shared_ptr<util::Library> m_gitLib = nullptr;
	};
};

#endif
