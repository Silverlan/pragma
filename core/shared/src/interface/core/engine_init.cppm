// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:core.engine_init;

import :debug.crashdump;
import :engine;

export namespace pragma {
	template<class T>
	std::shared_ptr<T> initialize_engine(int argc, char *argv[])
	{
		auto exe = engine_info::get_executable_name();
		debug::CrashHandler::Get().SetAppName(exe);
		auto en = std::shared_ptr<T> {new T {argc, argv}, [](T *p) {
#ifdef _WIN32
			                              if(std::uncaught_exceptions() > 0) {
				                              // If we're stack unwinding due to an uncaught exception,
				                              // we DON'T want to destroy the engine, since we'll need
				                              // the engine object to collect information for our crashdump!
				                              return;
			                              }
#endif
			                              p->Release();
			                              delete p;
		                              }};
#ifdef __linux__
		en->OpenConsole();
#endif
		if(en->Initialize(argc, argv) == false)
			return nullptr;
		en->Start();
		return en;
	}

	inline DLLNETWORK std::shared_ptr<Engine> initialize_server(int argc, char *argv[])
	{
		auto exe = engine_info::get_executable_name();
		debug::CrashHandler::Get().SetAppName(exe);
		auto en = std::shared_ptr<Engine> {new Engine {argc, argv}, [](Engine *p) {
			                                           if(std::uncaught_exceptions() > 0) {
				                                           // If we're stack unwinding due to an uncaught exception,
				                                           // we DON'T want to destroy the engine, since we'll need
				                                           // the engine object to collect information for our crashdump!
				                                           return;
			                                           }
			                                           p->Release();
			                                           delete p;
		                                           }};
		en->OpenConsole();
		if(en->Initialize(argc, argv) == false)
			return nullptr;
		// en->StartServer(false);
		en->Start();
		return en;
	}
};
