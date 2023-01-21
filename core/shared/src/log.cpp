/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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

void Engine::WriteToLog(const std::string &str)
{
	pragma::log(str);
}