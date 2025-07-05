// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "fsys/filesystem.h"
#include "stdafx_shared.h"
#include "pragma/util/util_module.hpp"
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>

std::string util::get_normalized_module_path(const std::string &lib, std::optional<bool> checkForClientSide)
{
	auto r = FileManager::GetCanonicalizedPath(lib);
	std::replace(r.begin(), r.end(), '\\', '/');
	if(ustring::substr(r, 0, 8) != "modules/")
		r = "modules/" + r;
	std::string ext;
	if(ufile::get_extension(r, &ext))
		return r;
#ifdef _WIN32
	r += ".dll";
#else
	r += ".so";
#endif
	auto brLast = r.find_last_of("\\/");
	auto fGetFileName = [&brLast](const std::string &name, std::string &outName, const std::string &prefixRepl = "") -> bool {
		auto r = name;
		auto prefix = r.substr(brLast + 1, 3);
		if(prefix != prefixRepl)
			r = r.substr(0, brLast + 1) + prefixRepl + r.substr(brLast + 1, r.length());
		outName = r;
		std::replace(outName.begin(), outName.end(), '\\', '/');
		return FileManager::Exists(outName);
	};
	std::string outName;
	if(fGetFileName(r, outName) == false && (checkForClientSide.has_value() == false || *checkForClientSide == false || fGetFileName(r, outName, "cl_") == false) && (checkForClientSide.has_value() == false || *checkForClientSide == true || fGetFileName(r, outName, "sv_") == false)
#ifdef __linux__
	  && fGetFileName(r, outName, "lib") == false && (checkForClientSide.has_value() == false || *checkForClientSide == false || fGetFileName(r, outName, "libcl_") == false)
	  && (checkForClientSide.has_value() == false || *checkForClientSide == true || fGetFileName(r, outName, "libsv_") == false)
#endif
	) {
		fGetFileName(r, outName);
	}
	return outName;
}

std::shared_ptr<util::Library> util::load_library_module(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories, std::optional<bool> checkForClientSide, std::string *err)
{
	auto libPath = get_normalized_module_path(lib, checkForClientSide);
	std::string lpath;
	if(FileManager::FindAbsolutePath(libPath, lpath) == false)
		lpath = libPath;
#ifdef __linux__
	std::replace(lpath.begin(), lpath.end(), '\\', '/');

	auto linAdditionalSearchDirectories = additionalSearchDirectories;
	std::string modPath;
	if(filemanager::find_absolute_path(util::DirPath("modules", ufile::get_path_from_filename(lib)).GetString(), modPath))
		linAdditionalSearchDirectories.push_back(modPath);
	return util::Library::Load(lpath, linAdditionalSearchDirectories, err);
#else
	return util::Library::Load(lpath, additionalSearchDirectories, err);
#endif
}

std::vector<std::string> util::get_default_additional_library_search_directories(const std::string &libModulePath)
{
	//prefer platform agnostic forward slash here.
	auto brLast = libModulePath.find_last_of('/');
	auto programPath = FileManager::GetProgramPath();
#ifdef _WIN32
	auto pathBin = programPath + "/bin";
#else
	auto pathBin = programPath + "/lib";
#endif
	auto pathModules = programPath + std::string("/") + libModulePath.substr(0, brLast);
	return std::vector<std::string> {pathBin, pathModules};
}
