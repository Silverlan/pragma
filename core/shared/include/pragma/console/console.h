/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __DBGCONSOLE_H__
#define __DBGCONSOLE_H__
#include "pragma/definitions.h"
#include <streambuf>
#include <fstream>

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

#endif // __DBGCONSOLE_H__
