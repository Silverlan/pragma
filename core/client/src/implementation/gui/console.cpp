// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :gui.command_line_entry;
import :gui.console;
import :gui.frame;
import :gui.scroll_container;
import :gui.snap_area;

import :client_state;
import :engine;
import pragma.gui;
import pragma.string.unicode;

static pragma::gui::WIHandle s_hConsole = {};
pragma::gui::types::WIConsole *pragma::gui::types::WIConsole::Open()
{
	if(s_hConsole.IsValid()) {
		auto *pConsole = static_cast<WIConsole *>(s_hConsole.get());
		auto *pFrame = pConsole->GetFrame();
		if(pFrame)
			pFrame->SetVisible(true);
		pConsole->UpdateConsoleMode();
		pConsole->RequestFocus();
		return pConsole;
	}
	auto &wgui = WGUI::GetInstance();
	auto *pFrame = wgui.Create<WIFrame>();
	pFrame->SetSize(512, 256);
	pFrame->SetMinSize(160, 100);
	pFrame->SetZPos(10'000);
	pFrame->SetTitle(locale::get_text("console"));
	pFrame->SetPos(675, 86);
	pFrame->SetSize(450, 876);
	pFrame->SetAnchor(0, 0, 1, 1, 1'280, 1'024);
	pFrame->SetRemoveOnClose(false);

	auto *contents = pFrame->GetContents();
	assert(contents);

	auto *pConsole = wgui.Create<WIConsole>(contents);
	contents->AddCallback("OnDetaching", FunctionCallback<void>::Create([pConsole]() { pConsole->SetSimpleConsoleMode(false, true); }));
	contents->AddCallback("OnDetached", FunctionCallback<void>::Create([pConsole]() { pConsole->m_mode = Mode::ExternalWindow; }));
	contents->AddCallback("OnReattached", FunctionCallback<void>::Create([pConsole]() { pConsole->m_mode = Mode::Standard; }));
	pConsole->SetSize(contents->GetWidth(), contents->GetHeight() - pConsole->GetTop());
	pConsole->SetAnchor(0, 0, 1, 1);
	pConsole->SetFrame(*pFrame);
	pConsole->SetIgnoreParentAlpha(true);

	pConsole->UpdateConsoleMode();
	pConsole->RequestFocus();
	s_hConsole = pConsole->GetHandle();

	auto wikiUrl = engine_info::get_wiki_url();
	pConsole->SetText("{[c:c2003b]}"
	                  "    _____ ____  _   _  _____  ____  _      ______ \n"
	                  "   / ____/ __ \\| \\ | |/ ____|/ __ \\| |    |  ____|\n"
	                  "  | |   | |  | |  \\| | (___ | |  | | |    | |__   \n"
	                  "  | |   | |  | | . ` |\\___ \\| |  | | |    |  __|  \n"
	                  "  | |___| |__| | |\\  |____) | |__| | |____| |____ \n"
	                  "   \\_____\\____/|_| \\_|_____/ \\____/|______|______|{[/c]}\n"
	                  "\n"
	                  "\nTo toggle the console, press the '`'-key on your keyboard.\n"
	                  "Here are some useful console commands to get you started:\n"
	                  "- {[l:list]}{[c:fde4f2]}list{[/c]}{[/l]}: Lists all available console commands.\n"
	                  "- {[c:f9cee7]}help{[/c]} <cvarName>: Prints a description for a particular console command or variable.\n"
	                  "- {[c:f4b8da]}lua_help{[/c]} <query>: Prints information about a particular Lua class/library/function/enum.\n"
	                  "- {[c:f4b8da]}lua_exec / lua_exec_cl{[/c]} <luaFile>: Executes the specified Lua-file serverside/clientside.\n"
	                  "- {[c:f4b8da]}lua_run / lua_run_cl{[/c]} <luaCode>: Executes the specified Lua-code serverside/clientside.\n"
	                  "- {[c:eea1cd]}map{[/c]} <mapName>: Disconnects from the current game and starts a new game with the specified map.\n"
	                  "- {[c:e68bbe]}exit{[/c]}: Exits the engine\n\n"
	                  "Useful links:\n"
	                  "- {[l:url \""
	  + wikiUrl
	  + "\"]}{[c:fde4f2]}Wiki{[/c]}{[/l]}\n"
	    "- {[l:url \"https://wiki.pragma-engine.com/books/lua-api/page/introduction\"]}{[c:fde4f2]}Lua Documentation{[/c]}{[/l]}\n"
	    "\n");
	return pConsole;
}
void pragma::gui::types::WIConsole::Close()
{
	if(s_hConsole.IsValid() == false)
		return;
	auto *pFrame = static_cast<WIConsole *>(s_hConsole.get())->GetFrame();
	if(pFrame)
		pFrame->SetVisible(false);
}
pragma::gui::types::WIConsole *pragma::gui::types::WIConsole::GetConsole() { return static_cast<WIConsole *>(s_hConsole.get()); }

/////////////

void pragma::gui::types::WIConsole::SetSimpleConsoleMode(bool simple, bool force)
{
	if(m_mode == Mode::ExternalWindow)
		return;
	if(simple == (m_mode == Mode::SimplifiedOverlay) && !force)
		return;
	auto a = simple ? 0.f : 1.f;
	auto *pFrame = GetFrame();
	if(pFrame) {
		pFrame->SetAlpha(simple ? 0.f : 0.85f);
		pFrame->SetRenderIfZeroAlpha(simple);

		if(simple) {
			m_menuConsolePos = pFrame->GetPos();
			m_menuConsoleSize = pFrame->GetSize();

			auto *pSnapArea = static_cast<WISnapArea *>(m_hTopSnapArea.get());
			if(pSnapArea)
				pFrame->SnapToTarget(*pSnapArea);
		}
		else {
			pFrame->SetPos(m_menuConsolePos);
			pFrame->SetSize(m_menuConsoleSize);
		}
	}
	if(m_hLogBg.IsValid()) {
		m_hLogBg->SetAlpha(simple ? 0.f : 0.75f);
		m_hLogBg->SetRenderIfZeroAlpha(simple);
	}
	if(m_hCommandEntry.IsValid()) {
		m_hCommandEntry->SetVisible(!simple);
		//static_cast<WICommandLineEntry*>(m_hCommandEntry.get())->SetAutocompleteEnabled(!simple);

		if(simple) {
			m_hCommandEntry->KillFocus();
			m_hCommandEntry->SetVisible(false);
		}
	}
	if(m_hLog.IsValid())
		m_hLog->SetIgnoreParentAlpha(simple);
	m_mode = simple ? Mode::SimplifiedOverlay : Mode::Standard;
}

void pragma::gui::types::WIConsole::Initialize()
{
	WIBase::Initialize();

	EnableThinking();
	SetSize(512, 256);
	FontSettings settings {};
	settings.fontSize = 12;
	FontManager::LoadFont("console", "dejavu/DejaVuSansMono", settings);

	auto &wgui = WGUI::GetInstance();
	auto *pLogBg = wgui.Create<WIRect>(this);
	pLogBg->SetColor(Color {20, 20, 20, 255});
	m_hLogBg = pLogBg->GetHandle();

	auto *pLogScrollContainer = wgui.Create<WIScrollContainer>(pLogBg);
	pLogScrollContainer->SetAutoStickToBottom(true);
	pLogScrollContainer->SetAutoAlignToParent(true);

	auto *pLog = wgui.Create<WITextEntry>(pLogScrollContainer);
	pLog->SetMultiLine(true);
	pLog->SetEditable(false);
	pLog->SetSelectable(true);
	m_hLog = pLog->GetHandle();

	auto *pText = pLog->GetTextElement();
	if(pText) {
		pText->SetFont("console");
		pText->SetAutoBreakMode(WIText::AutoBreak::WHITESPACE);
		pText->SetTagsEnabled(true);
		pText->SetName("console_log");
	}

	auto *pCaret = pLog->GetCaretElement();
	if(pCaret)
		pCaret->SetColor(Color {222, 222, 222, 255});

	auto *pBg = pLog->FindDescendantByName("background");
	if(pBg)
		pBg->Remove();

	auto *pBgOutline = pLog->FindDescendantByName("background_outline");
	if(pBgOutline)
		pBgOutline->Remove();

	auto *pEntry = wgui.Create<WICommandLineEntry>(this);
	pEntry->SetName("console_entry");
	pEntry->SetSize(GetWidth() - 20, 26);
	pEntry->SetPos(10, GetBottom() - pEntry->GetHeight() - 10);
	pEntry->SetAnchor(0, 1, 1, 1);
	auto hScrollContainer = pLogScrollContainer->GetHandle();
	auto hThis = GetHandle();
	m_cbCommandEntryVisibility = pEntry->GetVisibilityProperty()->AddCallback([hScrollContainer, hThis](std::reference_wrapper<const bool> oldValue, std::reference_wrapper<const bool> visible) mutable {
		if(hScrollContainer.IsValid() == false || hThis.IsValid() == false)
			return;
		static_cast<WIScrollContainer *>(hScrollContainer.get())->ScrollToBottom();

		auto *pScrollBar = static_cast<WIScrollContainer *>(hScrollContainer.get())->GetVerticalScrollBar();
		pScrollBar->SetIgnoreParentAlpha(visible);
	});
	auto hLog = pLog->GetHandle();
	pEntry->AddCallback("OnTextEntered", FunctionCallback<void>::Create([this, hThis, pEntry, hLog]() mutable {
		if(hThis.IsValid() == false)
			return;
		auto cmd = pEntry->GetText();
#ifdef WITEXT_VALIDATION_ENABLED
		if(cmd == "validate") {
			if(hLog.IsValid()) {
				auto *pLog = static_cast<WITextEntry *>(hLog.get());
				if(pLog) {
					auto *pText = pLog->GetTextElement();
					if(pText)
						pText->UnitTest();
				}
			}
			return;
		}
#endif
		get_cengine()->ConsoleInput(cmd.cpp_str());
		if(hLog.IsValid()) {
			auto &log = *static_cast<WITextEntry *>(hLog.get());
			auto *pText = log.GetTextElement();
			AppendText(cmd.cpp_str() + '\n');
		}
	}));
	pEntry->SetAutocompleteHandler([this, pEntry, hThis](const std::string &cmd, std::vector<std::string> &args) {
		if(hThis.IsValid() == false)
			return;
		std::vector<std::string> subStrings {};
		string::explode_whitespace(cmd, subStrings);
		if(subStrings.empty() == false) {
			auto *cf = get_cengine()->GetConVar(subStrings.front());
			if(cf && cf->GetType() == console::ConType::Command) {
				auto &c = static_cast<console::ConCommand &>(*cf);
				auto &fAutoComplete = c.GetAutoCompleteCallback();
				if(fAutoComplete) {
					auto arg = (subStrings.size() > 1) ? subStrings.at(1) : std::string {};
					fAutoComplete(arg, args, false);
					for(auto &arg : args)
						arg = subStrings.front() + " " + arg;
					return;
				}
			}
		}

		std::vector<std::pair<std::string_view, float>> bestCandidates(pEntry->GetAutoCompleteEntryLimit(), std::pair<std::string_view, float> {std::string_view {}, std::numeric_limits<float>::max()});
		const auto fProcessConVars = [&cmd, &bestCandidates](const std::map<std::string, std::shared_ptr<console::ConConf>> &conVars) {
			for(auto &pair : conVars) {
				auto percentage = string::calc_similarity(cmd, pair.first);
				auto it = std::find_if(bestCandidates.begin(), bestCandidates.end(), [percentage](const std::pair<std::string_view, float> &pair) { return percentage < pair.second; });
				if(it == bestCandidates.end())
					continue;
				it->first = pair.first;
				it->second = percentage;
			}
		};
		fProcessConVars(get_cengine()->GetConVars());
		auto *client = get_client_state();
		if(client != nullptr)
			fProcessConVars(client->GetConVars());
		args.reserve(bestCandidates.size());
		for(auto &candidate : bestCandidates) {
			if(candidate.second == std::numeric_limits<float>::max())
				break;
			args.push_back(std::string {candidate.first});
		}
	});
	m_hCommandEntry = pEntry->GetHandle();

	pText = pEntry->GetTextElement();
	if(pText)
		pText->SetFont("console");
	get_cengine()->SetRecordConsoleOutput(true);

	pLog->SetColor(Color {220, 220, 220, 255});

	pLogBg->SetSize(GetWidth() - 20, GetHeight() - 40);
	pLogBg->SetPos(10, 0);
	pLogBg->SetAnchor(0, 0, 1, 1);

	pLog->SetSize(pLogBg->GetSize());
	pLog->AddCallback("SetSize", FunctionCallback<void>::Create([pLog, hScrollContainer]() mutable {
		if(hScrollContainer.IsValid() == false)
			return;
		static_cast<WIScrollContainer *>(hScrollContainer.get())->Update();
	}));
	pLogBg->AddCallback("SetSize", FunctionCallback<void>::Create([hLog, pLogBg]() mutable {
		if(hLog.IsValid())
			hLog.get()->SetWidth(pLogBg->GetWidth());
	}));

	pLog->GetTextElement()->SetTagArgument("luafile", 0u, FunctionCallback<util::EventReply, std::reference_wrapper<const std::vector<std::string>>>::CreateWithOptionalReturn([](util::EventReply *result, std::reference_wrapper<const std::vector<std::string>> args) -> CallbackReturnType {
		if(args.get().empty() == false) {
			auto &f = args.get().front();
			auto lineIdx = 0u;
			if(args.get().size() > 1u)
				lineIdx = util::to_int(args.get().at(1u));
			Lua::OpenFileInZeroBrane("lua/" + f, lineIdx);
		}
		*result = util::EventReply::Handled;
		return CallbackReturnType::HasReturnValue;
	}));

	if(m_hCommandEntry.IsValid())
		m_hCommandEntry->SetIgnoreParentAlpha(true);

	auto *pMainMenu = get_client_state()->GetMainMenu();
	if(pMainMenu) {
		m_cbMainMenuVisibility = pMainMenu->GetVisibilityProperty()->AddCallback([this](std::reference_wrapper<const bool> oldValue, std::reference_wrapper<const bool> visible) { UpdateConsoleMode(); });
	}
}
void pragma::gui::types::WIConsole::OnDescendantFocusGained(WIBase &el)
{
	WIBase::OnDescendantFocusGained(el);
	if(&el == m_hCommandEntry.get())
		el.SetVisible(true);
}
void pragma::gui::types::WIConsole::OnDescendantFocusKilled(WIBase &el)
{
	WIBase::OnDescendantFocusKilled(el);
	if(&el == m_hCommandEntry.get())
		el.SetVisible(m_mode != Mode::SimplifiedOverlay);
}
void pragma::gui::types::WIConsole::RequestFocus()
{
	if(m_hCommandEntry.IsValid())
		m_hCommandEntry->RequestFocus();
}
void pragma::gui::types::WIConsole::OnRemove()
{
	WIBase::OnRemove();
	if(m_cbConsoleOutput.IsValid())
		m_cbConsoleOutput.Remove();
	if(m_cbMainMenuVisibility.IsValid())
		m_cbMainMenuVisibility.Remove();
	if(m_cbCommandEntryVisibility.IsValid())
		m_cbCommandEntryVisibility.Remove();
	get_cengine()->SetRecordConsoleOutput(false);
}
void pragma::gui::types::WIConsole::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIBase::Think(drawCmd);
	if(m_hLog.IsValid() == false)
		return;
	auto conOutput = get_cengine()->PollConsoleOutput();
	if(conOutput.has_value() == false)
		return;
	std::string text = "";
	std::optional<Color> currentColor = {};
	while(conOutput.has_value()) {
		if(conOutput->color) {
			if(currentColor.has_value() == false || *conOutput->color != *currentColor) {
				if(currentColor.has_value())
					text += "{[/c]}";
				currentColor = *conOutput->color;
				text += "{[c:" + conOutput->color->ToHexColor() + "]}";
			}
		}
		else if(currentColor.has_value()) {
			text += "{[/c]}";
			currentColor = {};
		}
		text += conOutput->output;
		conOutput = get_cengine()->PollConsoleOutput();
	}
	if(currentColor.has_value()) {
		text += "{[/c]}";
		currentColor = {};
	}

	if(m_pendingConsoleOutput.empty() == false)
		text = m_pendingConsoleOutput + text;
	m_pendingConsoleOutput = AppendText(text);
}

pragma::gui::types::WICommandLineEntry *pragma::gui::types::WIConsole::GetCommandLineEntryElement() { return static_cast<WICommandLineEntry *>(m_hCommandEntry.get()); }
pragma::gui::types::WITextEntry *pragma::gui::types::WIConsole::GetTextLogElement() { return static_cast<WITextEntry *>(m_hLog.get()); }
const auto REF_WIDTH = 1'280u;
const auto REF_HEIGHT = 1'024u;
pragma::gui::types::WISnapArea *pragma::gui::types::WIConsole::CreateSnapTarget(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t xt, uint32_t yt, uint32_t wt, uint32_t ht)
{
	auto *pFrame = GetFrame();
	if(pFrame == nullptr)
		return nullptr;
	auto *pSnapTarget = WGUI::GetInstance().Create<WISnapArea>();
	RemoveOnRemoval(pSnapTarget);
	pSnapTarget->SetPos(x, y);
	pSnapTarget->SetSize(w, h);

	auto *pTriggerArea = pSnapTarget->GetTriggerArea();
	if(pTriggerArea) {
		pTriggerArea->SetPos(xt, yt);
		pTriggerArea->SetSize(wt, ht);
		pTriggerArea->SetAnchor(0, 0, 1, 1);
	}
	pSnapTarget->SetAnchor(0, 0, 1, 1, REF_WIDTH, REF_HEIGHT);
	pFrame->AddSnapTarget(*pSnapTarget);
	return pSnapTarget;
}
void pragma::gui::types::WIConsole::SetExternallyOwned(bool externallyOwned)
{
	m_mode = externallyOwned ? Mode::ExternalOwnership : Mode::Standard;
	auto *frame = GetFrame();
	if(frame)
		frame->SetVisible(!externallyOwned);
	if(!externallyOwned) {
		SetParent(frame);
		return;
	}
	SetSimpleConsoleMode(false, true);
}
bool pragma::gui::types::WIConsole::IsExternallyOwned() const { return m_mode == Mode::ExternalOwnership; }
void pragma::gui::types::WIConsole::UpdateConsoleMode()
{
	if(m_mode == Mode::ExternalOwnership)
		return;
	SetSimpleConsoleMode(get_client_state()->IsMainMenuOpen() == false);
}
void pragma::gui::types::WIConsole::InitializeSnapAreas()
{
	const auto size = 242u;
	const auto sizeTrigger = 32u;
	CreateSnapTarget(0, 0, REF_WIDTH, REF_HEIGHT, 0, 0, sizeTrigger, sizeTrigger);
	auto *pTopSnapTarget = CreateSnapTarget(0, 0, REF_WIDTH, size, 0, 0, REF_WIDTH, sizeTrigger);
	CreateSnapTarget(0, REF_HEIGHT - size, REF_WIDTH, size, 0, size - sizeTrigger, REF_WIDTH, sizeTrigger);
	m_hTopSnapArea = pTopSnapTarget->GetHandle();
	/*CreateSnapTarget(
		0,0,size,REF_HEIGHT,
		0,0,sizeTrigger,REF_HEIGHT
	);
	CreateSnapTarget(
		REF_WIDTH -size,0,size,REF_HEIGHT,
		size -sizeTrigger,0,sizeTrigger,REF_HEIGHT
	);*/
}
void pragma::gui::types::WIConsole::SetFrame(WIFrame &frame)
{
	m_hFrame = frame.GetHandle();
	InitializeSnapAreas();

	m_menuConsolePos = frame.GetPos();
	m_menuConsoleSize = frame.GetSize();

	// Reload current console mode
	if(m_mode != Mode::ExternalWindow)
		SetSimpleConsoleMode(m_mode == Mode::SimplifiedOverlay, true);
}
pragma::gui::types::WIFrame *pragma::gui::types::WIConsole::GetFrame() { return static_cast<WIFrame *>(m_hFrame.get()); }

const pragma::string::Utf8String &pragma::gui::types::WIConsole::GetText() const
{
	static string::Utf8String s {};
	if(m_hLog.IsValid()) {
		auto *pTextEntry = static_cast<const WITextEntry *>(m_hLog.get());
		if(pTextEntry == nullptr)
			return s;
		auto *pText = const_cast<WITextEntry *>(pTextEntry)->GetTextElement();
		if(pText == nullptr)
			return s;
		return pText->GetText();
	}
	return s;
}
void pragma::gui::types::WIConsole::SetText(const std::string &text)
{
	if(m_hLog.IsValid() == false)
		return;
	auto *pLog = static_cast<WITextEntry *>(m_hLog.get());
	auto *pText = pLog->GetTextElement();
	if(pText == nullptr)
		return;
	pLog->SetText(text);
	pLog->SizeToContents();
	auto maxLines = GetMaxLogLineCount();
	auto numLinesRemove = static_cast<int32_t>(pText->GetLineCount()) - static_cast<int32_t>(maxLines);
	if(numLinesRemove <= 0)
		return;
	auto ltext = text;
	while(numLinesRemove > 0) {
		auto pos = ltext.find('\n');
		if(pos == std::string::npos)
			break;
		ltext = string::substr(ltext, pos + 1u);
		--numLinesRemove;
	}
	pLog->SetText(ltext);
	pLog->SizeToContents();
}
std::string_view pragma::gui::types::WIConsole::AppendText(const std::string &text)
{
	if(text.empty() || m_hLog.IsValid() == false)
		return {};
	auto *pLog = static_cast<WITextEntry *>(m_hLog.get());
	auto *pText = pLog->GetTextElement();
	if(pText == nullptr)
		return {};
	std::vector<std::string_view> lines {};
	auto lineStartOffset = 0u;
	auto numCharsInLine = 0u;
	for(auto &c : text) {
		if(c == '\n') {
			lines.push_back(std::string_view {text}.substr(lineStartOffset, numCharsInLine + 1u));
			lineStartOffset += numCharsInLine + 1u;
			numCharsInLine = 0u;
			continue;
		}
		++numCharsInLine;
	}

	std::string_view remaining {};
	if(numCharsInLine > 0u) {
		// Last line is incomplete (has not been ended with '\n').
		// Inserting incomplete lines is currently not supported, so we'll
		// remember the line for the next iteration.
		remaining = std::string_view {text}.substr(lineStartOffset, numCharsInLine);
	}
	for(auto &line : lines) {
		string::Utf8String str {std::string {line}};
		pText->AppendText(str);
	}

	auto maxLines = GetMaxLogLineCount();
	while(pText->GetTotalLineCount() > maxLines)
		pText->PopFrontLine();

	pLog->SizeToContents();
	return remaining;
}
void pragma::gui::types::WIConsole::SetMaxLogLineCount(uint32_t count) { m_maxLogLineCount = count; }
uint32_t pragma::gui::types::WIConsole::GetMaxLogLineCount() const { return m_maxLogLineCount; }
void pragma::gui::types::WIConsole::Clear() { SetText(""); }
