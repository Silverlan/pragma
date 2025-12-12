// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <signal.h>
#include <glob.h>
#endif
#include <cstdlib>

export module pragma.updater;

import pragma.string;

// These are directly copied from the sharedutils library.
// We can't create a dependency to the sharedutils library directly because
// the updater has to be able to remove the binary files during runtime.
export namespace pragma::util {
	std::unordered_map<std::string, std::string> get_launch_parameters(int argc, char *argv[])
	{
		std::unordered_map<std::string, std::string> out;
		for(auto i = 0; i < argc; i++) {
			char *arg = argv[i];
			std::vector<std::string> sub;
			pragma::string::explode(arg, "=", sub);
			if(!sub.empty()) {
				if(sub.size() > 1)
					out.insert(std::unordered_map<std::string, std::string>::value_type(sub[0], sub[1]));
				else
					out.insert(std::unordered_map<std::string, std::string>::value_type(sub[0], "1"));
			}
		}
		return out;
	}

#ifdef __linux__
	pid_t find_process_id(const char *process_name)
	{
		pid_t pid = -1;
		glob_t pglob;
		char *procname, *readbuf;
		int buflen = strlen(process_name) + 2;
		unsigned i;

		/* Get a list of all comm files. man 5 proc */
		if(glob("/proc/*/comm", 0, NULL, &pglob) != 0)
			return pid;

		/* The comm files include trailing newlines, so... */
		procname = (char *)malloc(buflen);
		strcpy(procname, process_name);
		procname[buflen - 2] = '\n';
		procname[buflen - 1] = 0;

		/* readbuff will hold the contents of the comm files. */
		readbuf = (char *)malloc(buflen);

		for(i = 0; i < pglob.gl_pathc; ++i) {
			FILE *comm;
			char *ret;

			/* Read the contents of the file. */
			if((comm = fopen(pglob.gl_pathv[i], "r")) == NULL)
				continue;
			ret = fgets(readbuf, buflen, comm);
			fclose(comm);
			if(ret == NULL)
				continue;

			/*
            If comm matches our process name, extract the process ID from the
            path, convert it to a pid_t, and return it.
            */
			if(strcmp(readbuf, procname) == 0) {
				pid = (pid_t)atoi(pglob.gl_pathv[i] + strlen("/proc/"));
				break;
			}
		}

		/* Clean up. */
		free(procname);
		free(readbuf);
		globfree(&pglob);
		return pid;
	}
#endif

	bool is_process_running(unsigned long long id)
	{
#ifdef _WIN32
		auto process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(id));
		if(process == nullptr)
			return false;
		unsigned long exitCode = 0;
		auto r = GetExitCodeProcess(process, &exitCode);
		if(r == false || exitCode != STILL_ACTIVE)
			return false;
		return true;
#else
		return (kill(id, 0) == 0) ? true : false;
#endif
	}

	bool is_process_running(const char *name)
	{
#ifdef _WIN32
		HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if(SnapShot == INVALID_HANDLE_VALUE)
			return false;
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(PROCESSENTRY32);
		if(!Process32First(SnapShot, &procEntry))
			return false;
		do {
			if(strcmp(procEntry.szExeFile, name) == 0)
				return true;
		} while(Process32Next(SnapShot, &procEntry));
		return false;
#else
		auto id = find_process_id(name);
		if(id == -1)
			return false;
		return is_process_running(id);
#endif
	}
}
