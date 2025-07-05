// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include "pragma/logging.hpp"
#include <fsys/filesystem.h>

void Engine::StartLogging()
{
	// TODO: This is obsolete, remove this function!
}

void Engine::EndLogging()
{
	// TODO: This is obsolete, remove this function!
}

void Engine::WriteToLog(const std::string &str) { pragma::log(str); }
