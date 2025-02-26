/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/util/util_python.hpp"
#include <sharedutils/util_library.hpp>

extern DLLNETWORK Engine *engine;

struct IPythonWrapper final {
	IPythonWrapper(util::Library &lib);
	IPythonWrapper() = default;

	bool (*run)(const char *code) = nullptr;
	bool (*exec)(const char *fileName, uint32_t argc, const char **argv) = nullptr;
	bool (*get_last_error)(std::string &) = nullptr;
	void (*reload)() = nullptr;

	bool valid() const { return m_bValid; }
  private:
	bool m_bValid = false;
};

#define PR_PYTHON_FIND_SYMBOL(lib, sym) (sym = lib.FindSymbolAddress<decltype(sym)>("pr_py_" #sym)) != nullptr
IPythonWrapper::IPythonWrapper(util::Library &lib) { m_bValid = PR_PYTHON_FIND_SYMBOL(lib, run) && PR_PYTHON_FIND_SYMBOL(lib, exec) && PR_PYTHON_FIND_SYMBOL(lib, get_last_error) && PR_PYTHON_FIND_SYMBOL(lib, reload); }

////////////////

static std::unique_ptr<IPythonWrapper> g_pyWrapper = nullptr;
static void clear_py_wrapper() { g_pyWrapper = nullptr; }
static IPythonWrapper *get_py_wrapper()
{
	static auto initialized = false;
	if(initialized)
		return g_pyWrapper.get();
	initialized = true;

	// Which networkstate we use doesn't really matter
	auto *nw = engine->GetServerNetworkState();
	if(!nw)
		nw = engine->GetClientState();
	if(!nw)
		return nullptr;
	std::string err;
	auto lib = nw->InitializeLibrary("python/pr_python", &err);
	if(!lib) {
		Con::cwar << "Failed to load python module: " << err << Con::endl;
		return nullptr;
	}
	auto wrapper = std::make_unique<IPythonWrapper>(*lib);
	if(!wrapper->valid())
		return nullptr;
	g_pyWrapper = std::move(wrapper);
	nw->AddCallback("OnClose", FunctionCallback<void>::Create([]() { clear_py_wrapper(); }));
	return g_pyWrapper.get();
}

////////////////

bool pragma::python::run(const char *code)
{
	auto *wrapper = get_py_wrapper();
	if(!wrapper)
		return false;
	return wrapper->run(code);
}
bool pragma::python::exec(std::string fileName, uint32_t argc, const char **argv)
{
	auto *wrapper = get_py_wrapper();
	if(!wrapper)
		return false;
	auto path = util::Path::CreateFile(fileName);
	path.Canonicalize();
	path = util::Path::CreatePath(util::get_program_path()) + path;
	return wrapper->exec(path.GetString().c_str(), argc, argv);
}

std::optional<std::string> pragma::python::get_last_error()
{
	auto *wrapper = get_py_wrapper();
	if(!wrapper)
		return {};
	std::string err;
	return wrapper->get_last_error(err) ? err : std::optional<std::string> {};
}

static auto g_blenderInitialized = false;
static auto g_blenderInitSuccess = false;
bool pragma::python::init_blender()
{
	if(g_blenderInitialized)
		return g_blenderInitSuccess;
	g_blenderInitialized = true;
	auto programPath = util::Path::CreatePath(util::get_program_path());
	auto scriptsPath = programPath + util::Path::CreatePath("modules/blender/3.2/scripts");
	if(!util::set_env_variable("BLENDER_SYSTEM_SCRIPTS", scriptsPath.GetString().c_str()))
		return false;
	if(!run("import sys"))
		return false;
	auto sitePackagesPath = programPath + util::Path::CreatePath("modules/blender/site-packages");
	g_blenderInitSuccess = run(("sys.path.append(\"" + sitePackagesPath.GetString() + "\")").c_str());
	return g_blenderInitSuccess;
}

void pragma::python::reload()
{
	auto *wrapper = get_py_wrapper();
	if(!wrapper)
		return;
	wrapper->reload();
	g_blenderInitialized = false;
	g_blenderInitSuccess = false;
}
