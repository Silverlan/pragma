// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.umodule;

std::string pragma::util::get_normalized_module_path(const std::string &lib, std::optional<bool> checkForClientSide)
{
	auto r = fs::get_canonicalized_path(lib);
	std::replace(r.begin(), r.end(), '\\', '/');
	if(string::substr(r, 0, 8) != "modules/")
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
		return fs::exists(outName);
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

std::shared_ptr<pragma::util::Library> pragma::util::load_library_module(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories, std::optional<bool> checkForClientSide, std::string *err)
{
	auto libPath = get_normalized_module_path(lib, checkForClientSide);
	std::string lpath;
	if(fs::find_absolute_path(libPath, lpath) == false)
		lpath = libPath;
#ifdef __linux__
	std::replace(lpath.begin(), lpath.end(), '\\', '/');

	auto linAdditionalSearchDirectories = additionalSearchDirectories;
	std::string modPath;
	if(fs::find_absolute_path(DirPath("modules", ufile::get_path_from_filename(lib)).GetString(), modPath))
		linAdditionalSearchDirectories.push_back(modPath);
	return Library::Load(lpath, linAdditionalSearchDirectories, err);
#else
	return pragma::util::Library::Load(lpath, additionalSearchDirectories, err);
#endif
}

std::vector<std::string> pragma::util::get_default_additional_library_search_directories(const std::string &libModulePath)
{
	auto brLast = libModulePath.find_last_of('/');
	auto libPath = libModulePath.substr(0, brLast);
	std::vector<std::string> paths;
	auto &rootPaths = fs::get_absolute_root_paths();
	paths.reserve(rootPaths.size() * 2);
#ifdef _WIN32
	std::string binDir = "bin";
#else
	std::string binDir = "lib";
#endif
	for(auto &rootPath : rootPaths) {
		auto binPath = DirPath(rootPath, binDir);
		paths.push_back(binPath.GetString());

		auto modPath = DirPath(rootPath, libPath);
		paths.push_back(modPath.GetString());
	}
	return paths;
}
