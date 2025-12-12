// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:gui.main_menu_base;

export import :gui.options_list;

export namespace pragma::gui::types {
	class WIMainMenuElement;
	class DLLCLIENT WIMainMenuBase : public WIBase {
	  public:
		WIMainMenuBase();
		virtual void Initialize() override;
		virtual pragma::util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
		virtual pragma::util::EventReply KeyboardCallback(pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
		void SelectItem(int i);
		void SelectNextItem();
		void SelectPreviousItem();
		WIMainMenuElement *GetElement(int i);
		WIMainMenuElement *GetSelectedElement();
		void AddMenuItem(std::string name, const CallbackHandle &onActivated);
		void AddMenuItem(int pos, std::string name, const CallbackHandle &onActivated);
		void RemoveMenuItem(int i);
		void OnElementSelected(WIMainMenuElement *el);
		void UpdateElements();
		void UpdateElement(int i);
	  protected:
		std::vector<pragma::gui::WIHandle> m_elements;
		std::vector<pragma::gui::WIHandle> m_optionLists = {};
		pragma::gui::WIHandle m_menuElementsContainer = {};
		int m_selected;
		pragma::gui::WIHandle m_hControlSettings;
		void OnGoBack(int button, int action, int mods);
		virtual void InitializeOptionsList(WIOptionsList *pList);
		WIOptionsList *InitializeOptionsList();
		virtual void DoUpdate() override;
	};

	class DLLCLIENT WIMainMenuElement : public WIBase {
	  protected:
		pragma::gui::WIHandle m_hBackground;
		pragma::gui::WIHandle m_hText;
		bool m_bSelected;
	  public:
		WIMainMenuElement();
		virtual ~WIMainMenuElement() override;
		virtual void Initialize() override;
		void Select();
		void Deselect();
		void SetText(std::string &text);
		virtual pragma::util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
		virtual void OnCursorEntered() override;
		virtual void OnCursorExited() override;
		void Activate();
		virtual void SetSize(int x, int y) override;
		Vector4 GetBackgroundColor();
		void SetBackgroundColor(float r, float g, float b, float a = 1.f);

		CallbackHandle onActivated;
		CallbackHandle onSelected;
		CallbackHandle onDeselected;
	};
};
