// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:console.debug_console;

export import std;

export namespace pragma::console {
	class DLLNETWORK DebugConsole {
	public:
		DebugConsole();
		~DebugConsole();

		void open();
		void close();
	private:
		std::streambuf *_cinbuf;
		std::streambuf *_coutbuf;
		std::streambuf *_cerrbuf;
		std::ifstream _console_cin;
		std::ofstream _console_cout;
		std::ofstream _console_cerr;
	};
}
