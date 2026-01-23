// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.command_line_entry;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WICommandLineEntry : public WITextEntry {
	  public:
		WICommandLineEntry();
		virtual void Initialize() override;

		void SetAutoCompleteEntryLimit(uint32_t limit);
		uint32_t GetAutoCompleteEntryLimit() const;
		void AddCommandHistoryEntry(const std::string_view &entry);
		void ClearCommandHistory();

		void FindAutocompleteOptions(const std::string &cmd, std::vector<std::string> &args) const;
		void SetAutocompleteHandler(const std::function<void(const std::string &, std::vector<std::string> &)> &fAutoCompleteHandler);

		void SetAutocompleteEnabled(bool enabled);
		bool IsAutocompleteEnabled() const;
	  private:
		virtual void OnFocusGained() override;
		virtual void OnFocusKilled() override;
		virtual void OnTextEntered() override;
		virtual void OnTextChanged(const string::Utf8String &text, bool changedByUser) override;
		void InitializeAutoCompleteList();
		std::vector<std::string> m_commandHistory = {};
		std::function<void(const std::string &, std::vector<std::string> &)> m_fAutoCompleteHandler = nullptr;
		uint32_t m_commandHistoryCount = 0u;
		uint32_t m_nextCommandHistoryInsertPos = 0u;
		WIHandle m_hAutoCompleteList = {};
		bool m_bSkipAutoComplete = false;
		bool m_bAutocompleteEnabled = true;
	};
}
