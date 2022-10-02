/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#ifdef _MSC_VER
#include "stdafx_shared.h"
#include "pragma/lua/lad/repository_manager.hpp"
#include <sharedutils/util_library.hpp>
#include <filesystem>

using namespace pragma::lua;

std::unique_ptr<RepositoryManager> RepositoryManager::Create(lua_State *l,std::string &outErr)
{
	constexpr auto MODULE_NAME = "git/pr_git";
	auto manager = std::unique_ptr<RepositoryManager>{new RepositoryManager{}};
	manager->m_gitLib = pragma::get_engine()->GetNetworkState(l)->InitializeLibrary(MODULE_NAME);
	if(!manager->m_gitLib)
	{
		outErr = "Unable to load module '" +std::string{MODULE_NAME} +"'!";
		return nullptr;
	}
	constexpr auto GIT_CLONE_FUNC_NAME = "pr_git_clone";
	manager->m_gitClone = manager->m_gitLib->FindSymbolAddress<GitClone>(GIT_CLONE_FUNC_NAME);
	if(!manager->m_gitClone)
	{
		outErr = "Could not locate symbol '" +std::string{GIT_CLONE_FUNC_NAME} +"' in module '" +std::string{MODULE_NAME} +"'!";
		return nullptr;
	}
	return manager->m_gitClone ? std::move(manager) : nullptr;
}

void RepositoryManager::CollectSourceCodeFiles(const std::string &sourceCodeLocation,std::vector<std::string> &outFiles)
{
	auto offset = outFiles.size();
	FileManager::FindSystemFiles((sourceCodeLocation +"*.cpp").c_str(),&outFiles,nullptr,true);
	for(auto i=offset;i<outFiles.size();++i)
		outFiles[i] = sourceCodeLocation +outFiles[i];

	std::vector<std::string> dirs;
	FileManager::FindSystemFiles((sourceCodeLocation +"*").c_str(),nullptr,&dirs,true);
	for(auto &dir : dirs)
		CollectSourceCodeFiles(sourceCodeLocation +dir +'/',outFiles);
}

void RepositoryManager::ClearLocalRepositoryDir()
{
	// Git-files are read-only by default; We'll have to change permission flags to remove them
	auto absRepositoryDir = FileManager::GetProgramPath() +'/' +m_tmpRepositoryLocation;
	try
	{
		for(auto &path : std::filesystem::recursive_directory_iterator(absRepositoryDir +".git"))
		{
			try {
				std::filesystem::permissions(path,std::filesystem::perms::all);
			}
			catch (std::exception& e) {
			}           
		}
	}
	catch(const std::filesystem::filesystem_error &err)
	{
	}
	filemanager::remove_directory(m_tmpRepositoryLocation);
}

bool RepositoryManager::LoadRepositoryReferences(std::string &outErr)
{
	std::string rootDir = "core/shared/";
	std::string localRepositoryDir = "temp/plad_repo/";
	auto absRepositoryDir = FileManager::GetProgramPath() +'/' +localRepositoryDir;

	ClearLocalRepositoryDir();
	FileManager::CreatePath(localRepositoryDir.c_str());

	std::string commitId;
	auto result = m_gitClone(
		REPOSITORY_CHECKOUT_URL,REPOSITORY_BRANCH,
		{rootDir +"*.cpp"},absRepositoryDir,outErr,&commitId
	);
	util::ScopeGuard sgClearTmpRepo {[this]() {ClearLocalRepositoryDir();}};
	if(!result)
		return false;

	std::vector<std::string> files;
	auto &sourceCodeDir = absRepositoryDir;
	CollectSourceCodeFiles(sourceCodeDir +rootDir,files);
	for(auto &fname : files)
	{
		if(ufile::get_file_from_filename(fname) == "ldoc.cpp")
			continue; // Skip this file
		auto f = FileManager::OpenSystemFile(fname.c_str(),"r");
		if(!f)
			continue;
		auto contents = f->ReadString();
		f = nullptr;
		auto pos = contents.find("#PLAD");
		if(pos == std::string::npos)
			continue;
		auto e = contents.find('\n',pos);
		auto sub = contents.substr(pos +5,e -(pos +5));
		ustring::remove_whitespace(sub);

		uint32_t lineIdx = 0;
		size_t c = 0;
		while(c < pos)
		{
			c = contents.find('\n',c);
			++lineIdx;
			if(c != std::string::npos)
				++c;
		}

		Con::cout<<"Found PLAD ID: "<<sub<<" (Line "<<lineIdx<<")"<<Con::endl;

		auto relPath = fname.substr(sourceCodeDir.length());
		std::string url = std::string{REPOSITORY_URL} +std::string{REPOSITORY_BRANCH} +"/" +relPath +"#L" +std::to_string(lineIdx);
		std::string urlCommit = std::string{REPOSITORY_URL} +commitId +"/" +relPath +"#L" +std::to_string(lineIdx);
		Con::cout<<"URL: "<<url<<Con::endl;
		Con::cout<<"URL Commit: "<<urlCommit<<Con::endl;
	}
	return true;
}
#endif
