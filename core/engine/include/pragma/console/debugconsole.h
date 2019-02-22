#include "pragma/console/console.h"
#ifdef _WIN32
#include <Windows.h> // Required for FreeConsole
#endif
#define DEBUGCONSOLE \
	void OpenConsole(); \
	void CloseConsole(); \
	bool IsConsoleOpen(); \
	DebugConsole *GetConsole();
