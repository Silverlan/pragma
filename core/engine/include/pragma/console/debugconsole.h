/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/console/console.h"
#ifdef _WIN32
#include <Windows.h> // Required for FreeConsole
#endif
#define DEBUGCONSOLE \
	virtual void OpenConsole(); \
	virtual void CloseConsole(); \
	void ToggleConsole(); \
	virtual bool IsConsoleOpen() const; \
	DebugConsole *GetConsole();
