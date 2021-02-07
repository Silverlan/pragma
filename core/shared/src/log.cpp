/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include <fsys/filesystem.h>

void Engine::StartLogging()
{
	EndLogging();
	std::string file = GetConVarString("log_file");
	if(file.empty())
		file = "log.txt";
	else if(file.length() < 4 || file.substr(file.length() -4) != ".txt")
		file += ".txt";
	auto f = FileManager::OpenFile<VFilePtrReal>(file.c_str(),"a");
	if(f == NULL)
	{
		Con::cwar<<"WARNING: Unable to write log!"<<Con::endl;
		return;
	}
	m_logFile = f;
	WriteToLog("--- Start of log (" +std::string(GetDate()) +")" +" ---\n");
}

void Engine::EndLogging()
{
	if(m_logFile == NULL)
		return;
	WriteToLog("--- End of log (" +std::string(GetDate()) +")" +" ---\n");
	m_logFile.reset();
	m_logFile = NULL;
}

void Engine::WriteToLog(const std::string &str)
{
	if(m_logFile == NULL)
		return;
	m_logFile->WriteString(str.c_str());
	/*
	std::string file = GetConVarString("log_file");
	std::string path = GetAppPath();
	path += "\\";
	path += file;
	freopen(path.c_str(),"a",m_logFile); // Re-Opening file to update it in realtime. TODO: Find a better solution?
	*/
}