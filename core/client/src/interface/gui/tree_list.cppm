// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.tree_list;

export import :gui.table;

export namespace pragma::gui::types {
	class WITreeList;
	class DLLCLIENT WITreeListElement : public WITableRow {
	  public:
		WITreeListElement();
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnVisibilityChanged(bool bVisible) override;
		void SetTreeParent(WITreeListElement *pEl);
		void SetXOffset(uint32_t x);
		void SetList(WITreeList *pList);
		uint32_t GetDepth() const;
		bool IsCollapsed() const;
		void Toggle(bool bAll = false);
		void Collapse(bool bAll = false);
		void Expand(bool bAll = false);
		void Clear();
		WIText *GetTextElement() const;
		const std::vector<WIHandle> &GetItems() const;
		WITreeListElement *AddItem(const std::string &text, const std::function<void(WITreeListElement &)> &fPopulate = nullptr);
	  protected:
		std::vector<WIHandle> m_items;
		bool m_bCollapsed;
		std::function<void(WITreeListElement &)> m_fPopulate = nullptr;
		uint32_t m_xOffset;
		uint32_t m_depth;
		WIHandle m_pTreeParent;
		WIHandle m_pArrow;
		WIHandle m_pList;
		WIHandle m_hText = {};
		WITreeListElement *GetLastItem() const;
		void SetTextElement(WIText *pText);
	};

	class DLLCLIENT WITreeList : public WITable {
	  protected:
		virtual void DoUpdate() override;
		WIHandle m_pRoot;
	  public:
		WITreeList();
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		virtual util::EventReply MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods) override;
		virtual WITableRow *AddRow() override;
		WITreeListElement *AddItem(const std::string &text, const std::function<void(WITreeListElement &)> &fPopulate = nullptr);
		WITreeListElement *GetRootItem() const;
		void ExpandAll();
		void CollapseAll();
	};
};
