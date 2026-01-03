// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :console.commands;

void LPARAM_enable_gfx_api_dump(const std::vector<std::string> &argv)
{
#ifdef PR_DEBUG_API_DUMP
	prosper::debug::set_api_dump_enabled(argv.empty() || pragma::util::to_boolean(argv.front()));
#else
	Con::CERR << "API dump launch parameter was set, but API dump is not enabled in this build. Build with -DENABLE_DEBUG_API_DUMP to enable it." << Con::endl;
#endif
}

#ifdef PR_DEBUG_API_DUMP
namespace pragma::debug {
	// These are mainly used in the VS immediate window for debugging purposes
	DLLCLIENT void dump_api_dump_recorder_ss(std::stringstream &ss, uint64_t addr)
	{
		auto *obj = reinterpret_cast<prosper::ContextObject *>(addr);
		if(obj == nullptr)
			return;
		auto *cmd = dynamic_cast<prosper::ICommandBuffer *>(obj);
		if(!cmd)
			return;
		auto &apiDumpRecorder = cmd->GetApiDumpRecorder();
		apiDumpRecorder.Print(ss);
	}
	DLLCLIENT void dump_api_dump_recorder(uint64_t addr)
	{
		std::stringstream ss;
		dump_api_dump_recorder_ss(ss, addr);
		Con::COUT << ss.str() << Con::endl;
	}
	DLLCLIENT void dump_api_dump_recorder_f(uint64_t addr)
	{
		std::stringstream ss;
		dump_api_dump_recorder_ss(ss, addr);
		fs::write_file("temp/api_dump_recorder.txt", ss.str());
	}
	DLLCLIENT void dump_api_dump_recorder_f()
	{
		auto &apiDumpRecorder = pragma::get_cengine()->GetRenderContext().GetApiDumpRecorder();

		std::stringstream ss;
		apiDumpRecorder.Print(ss);
		fs::write_file("temp/api_dump_recorder.txt", ss.str());
	}
	DLLCLIENT void dump_api_dump_recorder_calltrace_ss(std::stringstream &ss, uint64_t addr, uint64_t cmdIdx)
	{
		auto *obj = reinterpret_cast<prosper::ContextObject *>(addr);
		if(obj == nullptr)
			return;
		auto *cmd = dynamic_cast<prosper::ICommandBuffer *>(obj);
		if(!cmd)
			return;
		auto &apiDumpRecorder = cmd->GetApiDumpRecorder();
		apiDumpRecorder.PrintCallTrace(cmdIdx, ss);
	}
	DLLCLIENT void dump_api_dump_recorder_calltrace(uint64_t addr, uint64_t cmdIdx)
	{
		std::stringstream ss;
		dump_api_dump_recorder_calltrace_ss(ss, addr, cmdIdx);
		Con::COUT << ss.str() << Con::endl;
	}
	DLLCLIENT void dump_api_dump_recorder_calltrace_main(uint64_t cmdIdx, int32_t recordIdx)
	{
		auto &apiDumpRecorder = pragma::get_cengine()->GetRenderContext().GetApiDumpRecorder();

		std::stringstream ss;
		apiDumpRecorder.PrintCallTrace(cmdIdx, ss, recordIdx);
		Con::COUT << ss.str() << Con::endl;
	}
	DLLCLIENT void dump_api_dump_recorder_calltrace_main(uint64_t cmdIdx)
	{
		auto &apiDumpRecorder = pragma::get_cengine()->GetRenderContext().GetApiDumpRecorder();

		std::stringstream ss;
		apiDumpRecorder.PrintCallTrace(cmdIdx, ss);
		Con::COUT << ss.str() << Con::endl;
	}
	DLLCLIENT void dump_api_dump_recorder_calltrace_f(uint64_t addr, uint64_t cmdIdx)
	{
		std::stringstream ss;
		dump_api_dump_recorder_calltrace_ss(ss, addr, cmdIdx);
		fs::write_file("temp/api_dump_recorder_calltrace.txt", ss.str());
	}
};
#endif
