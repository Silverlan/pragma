// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/console/console.h"
#ifdef _WIN32
#include <Windows.h> // Required for FreeConsole
#endif
#define DEBUGCONSOLE                                                                                                                                                                                                                                                                             \
	virtual void OpenConsole();                                                                                                                                                                                                                                                                  \
	virtual void CloseConsole();                                                                                                                                                                                                                                                                 \
	void ToggleConsole();                                                                                                                                                                                                                                                                        \
	virtual bool IsConsoleOpen() const;                                                                                                                                                                                                                                                          \
	DebugConsole *GetConsole();
