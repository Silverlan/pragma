// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef __linux__
#include <linenoise.h>
#include <errno.h>
#endif

module pragma.shared;

#ifdef __linux__
import :console.line_noise;

static void completion(const char *buf, linenoiseCompletions *lc);
static const char *hints(const char *buf, int *color, int *bold);
static constexpr const char *HISTORY_FILE_LOCATION_RELATIVE = "cache/terminal_history.txt";
static constexpr const char *PROMPT_PREFIX = "> ";
static std::string get_history_file_location() { return pragma::util::FilePath(pragma::fs::get_program_write_path(), HISTORY_FILE_LOCATION_RELATIVE).GetString(); }

static bool g_enabled = false;
bool pragma::console::impl::is_linenoise_enabled() { return g_enabled; }

void pragma::console::impl::init_linenoise()
{
	if(g_enabled)
		return; // Already initialized
	g_enabled = true;
	/* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
	linenoiseSetCompletionCallback(completion);
	linenoiseSetHintsCallback(hints);

	linenoiseSetMultiLine(0);

	/* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
	linenoiseHistoryLoad(get_history_file_location().c_str()); /* Load the history at startup */
}

void pragma::console::impl::close_linenoise() { linenoiseHistorySave(get_history_file_location().c_str()); }

void pragma::console::impl::update_linenoise()
{
	static struct linenoiseState ls;
	static char buf[1024];
	static auto firstLine = true;
	if(firstLine) {
		firstLine = false;
		linenoiseEditStart(&ls, -1, -1, buf, sizeof(buf), PROMPT_PREFIX);
	}

	fd_set readfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&readfds);
	FD_SET(ls.ifd, &readfds);
	tv.tv_sec = 0; // 1 sec timeout
	tv.tv_usec = 0;

	retval = select(ls.ifd + 1, &readfds, nullptr, nullptr, &tv);
	if(retval == -1)
		return; // Error of some kind?
	if(retval) {
		auto *line = linenoiseEditFeed(&ls);
		if(!line) {
			if(errno == EAGAIN) {
				// User has pressed ctrl +c
			}
			else if(errno == ENOENT) {
				// User has pressed ctrl +d
			}
			else {
				// Unknown I/O error
			}
			return;
		}

		if(line != linenoiseEditMore) {
			linenoiseEditStop(&ls);

			get_engine()->ConsoleInput(line, false);
			linenoiseHistoryAdd(line);

			linenoiseFree(const_cast<char *>(line));
			linenoiseEditStart(&ls, -1, -1, buf, sizeof(buf), PROMPT_PREFIX);
		}
	}
}

static void get_autocomplete_options(const std::string &cmd, std::vector<std::string> &args, uint32_t autoCompleteEntryLimit)
{
	auto *en = pragma::get_engine();
	std::vector<std::string> subStrings {};
	pragma::string::explode_whitespace(cmd, subStrings);
	if(subStrings.empty() == false) {
		auto *cf = en->GetConVar(subStrings.front());
		if(cf && cf->GetType() == pragma::console::ConType::Command) {
			auto &c = static_cast<pragma::console::ConCommand &>(*cf);
			auto &fAutoComplete = c.GetAutoCompleteCallback();
			if(fAutoComplete) {
				auto arg = (subStrings.size() > 1) ? subStrings.at(1) : std::string {};
				fAutoComplete(arg, args, true);
				for(auto &arg : args)
					arg = subStrings.front() + " " + arg;
				return;
			}
		}
	}

	std::vector<std::pair<std::string_view, float>> bestCandidates(autoCompleteEntryLimit, std::pair<std::string_view, float> {std::string_view {}, std::numeric_limits<float>::max()});
	std::unordered_set<std::string> traversed;
	const auto fProcessConVars = [&cmd, &bestCandidates, &traversed](const std::map<std::string, std::shared_ptr<pragma::console::ConConf>> &conVars) {
		for(auto &pair : conVars) {
			if(pair.first.length() < cmd.length())
				continue;
			if(!pragma::string::compare(pair.first.c_str(), cmd.c_str(), false, cmd.length()))
				continue;
			if(traversed.find(pair.first) != traversed.end())
				continue;
			auto percentage = pragma::string::calc_similarity(cmd, pair.first);
			auto it = std::find_if(bestCandidates.begin(), bestCandidates.end(), [](const std::pair<std::string_view, float> &pair) { return pair.second == std::numeric_limits<float>::max(); });
			if(it == bestCandidates.end())
				it = std::find_if(bestCandidates.begin(), bestCandidates.end(), [percentage](const std::pair<std::string_view, float> &pair) { return percentage < pair.second; });
			if(it == bestCandidates.end())
				continue;
			it->first = pair.first;
			it->second = percentage;
			traversed.insert(pair.first);
		}
	};
	fProcessConVars(en->GetConVars());
	auto *cl = en->GetClientState();
	if(cl != nullptr)
		fProcessConVars(cl->GetConVars());
	auto *sv = en->GetServerNetworkState();
	if(sv != nullptr)
		fProcessConVars(sv->GetConVars());
	args.reserve(bestCandidates.size());
	for(auto &candidate : bestCandidates) {
		if(candidate.second == std::numeric_limits<float>::max())
			break;
		args.push_back(std::string {candidate.first});
	}
}

void completion(const char *buf, linenoiseCompletions *lc)
{
	std::vector<std::string> options;
	get_autocomplete_options(buf, options, 100);
	for(auto &option : options)
		linenoiseAddCompletion(lc, option.c_str());
}

const char *hints(const char *buf, int *color, int *bold)
{
	std::string cmd {buf};
	if(cmd.length() == 0)
		return nullptr;
	static std::string bestCandidate;
	bestCandidate.clear();
	const auto fProcessConVars = [&cmd](const std::map<std::string, std::shared_ptr<pragma::console::ConConf>> &conVars) {
		for(auto &pair : conVars) {
			if(pair.first.length() < cmd.length())
				continue;
			if(!bestCandidate.empty() && pair.first.length() > bestCandidate.size())
				continue;
			if(!pragma::string::compare(pair.first.c_str(), cmd.c_str(), false, cmd.length()))
				continue;
			bestCandidate = pair.first;
		}
	};

	auto *en = pragma::get_engine();
	fProcessConVars(en->GetConVars());
	auto *cl = en->GetClientState();
	if(cl != nullptr)
		fProcessConVars(cl->GetConVars());
	auto *sv = en->GetClientState();
	if(sv != nullptr)
		fProcessConVars(sv->GetConVars());
	if(!bestCandidate.empty()) {
		*color = 35; // ANSI color for purple
		*bold = 0;
		bestCandidate = pragma::string::substr(bestCandidate, strlen(buf));
		return bestCandidate.c_str();
	}

	auto st = cmd.find_first_not_of(pragma::string::WHITESPACE);
	st = cmd.find_first_of(pragma::string::WHITESPACE, st);
	if(st != std::string::npos) {
		auto cvarName = pragma::string::substr(cmd, 0, st);
		pragma::string::remove_whitespace(cvarName);
		auto *cf = en->GetConVar(cvarName);
		if(cf && cf->GetType() == pragma::console::ConType::Command) {
			auto &c = static_cast<pragma::console::ConCommand &>(*cf);
			auto &fAutoComplete = c.GetAutoCompleteCallback();
			if(fAutoComplete) {

				auto arg = pragma::string::substr(cmd, cmd.find_first_not_of(pragma::string::WHITESPACE, st));
				std::vector<std::string> args;
				fAutoComplete(arg, args, true);
				if(!args.empty()) {
					bestCandidate = args.front();
					if(pragma::string::compare(bestCandidate.c_str(), arg.c_str(), false, arg.length()))
						bestCandidate = pragma::string::substr(bestCandidate, arg.length());
					*color = 35; // ANSI color for purple
					*bold = 0;
					return bestCandidate.c_str();
				}
			}
		}
	}
	return nullptr;
}

#endif
