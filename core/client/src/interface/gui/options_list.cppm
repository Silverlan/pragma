// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.options_list;

export import :gui.checkbox;
export import :gui.choice_list;
export import :gui.slider;
export import :gui.table;

export namespace pragma::gui::types {
	class DLLCLIENT WIOptionsList : public WIBase {
	  private:
		WIHandle m_hTable;
		WIHandle m_hHeaderRow;
		uint32_t m_maxHeight = 512;
		std::unordered_map<std::string, std::string> m_updateCvars;
		std::unordered_map<std::string, platform::Key> m_keyBindingsAdd[2];
		std::unordered_map<std::string, platform::Key> m_keyBindingsErase[2];
		std::unordered_map<std::string, WIHandle> m_rows;
		template<class T>
		WIChoiceList *AddChoiceList(const std::string &name, T list, const std::string &cvarName, const std::function<void(WIChoiceList *)> &initializer, const std::optional<std::string> &optRowIdent = {});
		template<class T>
		WIDropDownMenu *AddDropDownMenu(const std::string &name, T list, const std::string &cvarName, const std::function<void(WIDropDownMenu *)> &initializer);
		virtual void DoUpdate() override;
	  public:
		WIOptionsList();
		virtual ~WIOptionsList() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		void SetMaxHeight(uint32_t h);
		WITableRow *AddRow(const std::optional<std::string> &identifier = {});
		WITableRow *AddHeaderRow();
		WICheckbox *AddToggleChoice(const std::string &name, const std::string &cvarName, const std::function<std::string(bool)> &translator, const std::function<bool(std::string)> &translator2 = nullptr);
		WICheckbox *AddToggleChoice(const std::string &name, const std::string &cvarName);
		WIChoiceList *AddChoiceList(const std::string &name, const std::string &cvarName = "");
		WIChoiceList *AddChoiceList(const std::string &name, const std::function<void(WIChoiceList *)> &initializer, const std::string &cvarName = "");
		WIChoiceList *AddChoiceList(const std::string &name, const std::vector<std::string> &list, const std::string &cvarName = "");
		WIChoiceList *AddChoiceList(const std::string &name, const std::vector<std::pair<std::string, std::string>> &list, const std::string &cvarName = "", const std::optional<std::string> &optRowIdent = {});
		WIDropDownMenu *AddDropDownMenu(const std::string &name, const std::string &cvarName = "");
		WIDropDownMenu *AddDropDownMenu(const std::string &name, const std::function<void(WIDropDownMenu *)> &initializer, const std::string &cvarName);
		WIDropDownMenu *AddDropDownMenu(const std::string &name, const std::vector<std::string> &list, const std::string &cvarName);
		WIDropDownMenu *AddDropDownMenu(const std::string &name, const std::unordered_map<std::string, std::string> &list, const std::string &cvarName);
		WISlider *AddSlider(const std::string &name, const std::function<void(WISlider *)> &initializer, const std::string &cvarName = "");
		void AddKeyBinding(const std::string &keyName, const std::string &cvarName);
		WITextEntry *AddTextEntry(const std::string &name, const std::string &cvarName = "");
		virtual void SizeToContents(bool x = true, bool y = true) override;
		void SetTitle(const std::string &title);
		void SetUpdateConVar(const std::string &cvar, const std::string &value);
		void RunUpdateConVars(bool bClear = true);
		WITableRow *GetRow(const std::string &identifier) const;
		std::unordered_map<std::string, std::string> &GetUpdateConVars();
	};
};
