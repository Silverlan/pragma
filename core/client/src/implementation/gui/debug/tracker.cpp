// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.debug_tracker;

static std::unique_ptr<pragma::gui::debug_tracker::Tracker> g_debugTracker;
static pragma::gui::debug_tracker::Tracker &get_debug_tracker()
{
	if(!g_debugTracker) {
		g_debugTracker = std::make_unique<pragma::gui::debug_tracker::Tracker>();
		g_debugTracker->SetLogEntryHandler([](const pragma::gui::types::WIBase &e, pragma::gui::debug_tracker::LogEntry &entry) {
			auto *game = pragma::get_cgame();
			if(!game)
				return;
			auto *l = game->GetLuaState();
			if(!l)
				return;
			std::stringstream ss;
			if(!Lua::PrintTraceback(l, ss))
				return;
			entry.AddInfo("lua_callstack", ss.str());
		});
	}
	return *g_debugTracker;
}

void pragma::gui::set_debug_tracking_enabled(const types::WIBase &el, bool enabled)
{
	if(enabled)
		get_debug_tracker().EnableTracking(el);
	else
		get_debug_tracker().DisableTracking(el);
}
const pragma::gui::debug_tracker::Log *pragma::gui::get_debug_tracking_log(const types::WIBase &el) { return get_debug_tracker().GetLog(el); }
void pragma::gui::print_debug_tracking_log(const types::WIBase &el)
{
	auto *log = get_debug_tracker().GetLog(el);
	if(!log) {
		Con::CWAR << "No debug log found for UI element!" << Con::endl;
		return;
	}
	Con::COUT << "Log Entries:" << Con::endl;
	size_t idx = 1;
	for(auto &entry : log->entries) {
		std::stringstream ss;
		ss << "#" << idx++ << ":" << "\n";
		ss << entry.name << "(";
		auto firstArg = true;
		for(auto &arg : entry.args) {
			if(firstArg)
				firstArg = false;
			else
				ss << ", ";
			ss << arg.name << "=" << arg.value;
		}
		ss << ")\n";
		for(auto &[name, info] : entry.info) {
			ss << "\n";
			ss << name << ":\n" << info << "\n";
		}
		ss << "\n\n\n";
		Con::COUT << ss.str();
	}
}
