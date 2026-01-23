// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.choice_list;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIChoiceList : public WIBase {
	  public:
		struct Choice {
			Choice(const std::string &c, const std::string &v) : choice(c), value(v) {}
			std::string choice;
			std::string value;
		};
		WIChoiceList();
		virtual ~WIChoiceList() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		void SetChoices(const std::vector<std::string> &choices);
		void SetChoices(const std::vector<std::pair<std::string, std::string>> &choices);
		void AddChoice(const std::string &choice, const std::string &val);
		void AddChoice(const std::string &choice);
		uint32_t GetChoiceCount() const;
		void SelectChoice(UInt choice);
		void SelectChoice(const std::string &choice);
		void SelectNext();
		void SelectPrevious();
		const Choice *GetSelectedChoice();
	  private:
		WIHandle m_text;
		WIHandle m_buttonPrev;
		WIHandle m_buttonNext;
		std::vector<Choice> m_choices;
		UInt m_selected;
		void UpdateButtons();
	};
};
