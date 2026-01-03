// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.table;

export import :gui.container;

export namespace pragma::gui::types {
	class WITableRow;
	class DLLCLIENT WITableCell : public WIContainer {
	  private:
		std::pair<int32_t, int32_t> m_span;
		WITableRow *GetRow() const;
	  public:
		WITableCell();
		virtual ~WITableCell() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		virtual void OnChildAdded(WIBase *child) override;
		virtual void DoUpdate() override;
		WIBase *GetFirstElement();
		// Not yet implemented
		void SetRowSpan(int32_t span);
		void SetColSpan(int32_t span);
		int32_t GetRowSpan() const;
		int32_t GetColSpan() const;
	};

	class DLLCLIENT WITable : public WIContainer {
	  public:
		friend WITableRow;
	  public:
		enum class SelectableMode : uint8_t { None = 0u, Single, Multi };

		WITable();
		virtual ~WITable() override;
		virtual void Initialize() override;
		void SetColumnWidth(unsigned int col, int width);
		virtual WITableRow *AddRow();
		WITableRow *AddHeaderRow();
		void MoveRow(WITableRow *a, WITableRow *pos, bool bAfter = true);
		void SelectRow(WITableRow &row);
		unsigned int GetRowCount() const;
		virtual void SetSize(int x, int y) override;
		uint32_t GetRowIndex(WITableRow *pRow) const;
		void RemoveRow(uint32_t rowIdx);
		void SetSortable(bool b);
		bool IsSortable() const;
		void SetSortFunction(const std::function<bool(const WITableRow &, const WITableRow &, uint32_t, bool)> &sortFunc);
		const std::function<bool(const WITableRow &, const WITableRow &, uint32_t, bool)> &GetSortFunction() const;
		void Sort();
		void SetRowHeight(int h);
		int GetRowHeight() const;
		void SetSelectable(SelectableMode mode);
		SelectableMode GetSelectableMode() const;
		void DeselectAllRows();
		void SetScrollable(bool b);
		bool IsScrollable() const;
		// Removes all rows except for the first one, if it's a header row (Unless bAll is set to true)
		void Clear(bool bAll = false);
		WITableRow *GetRow(unsigned int id) const;
		const std::vector<WIHandle> &GetSelectedRows() const;
		WIHandle GetFirstSelectedRow() const;
		virtual void SizeToContents(bool x = true, bool y = true) override;
	  protected:
		struct SortData {
			SortData(WITable *t, bool bAsc, unsigned int col);
			bool operator()(const WIHandle &a, const WIHandle &b);
			WITable *table;
			bool ascending;
			unsigned int column;
		};
		std::unordered_map<unsigned int, int> m_columnWidths;
		int m_rowHeight;
		SelectableMode m_selectableMode = SelectableMode::None;
		bool m_bSortable;
		std::function<bool(const WITableRow &, const WITableRow &, uint32_t, bool)> m_sortFunction;
		unsigned int m_sortColumn;
		bool m_bSortAsc;
		WIHandle m_hSortArrow;
		WIHandle m_hScrollContainer;
		bool m_bScrollable;
		std::vector<CallbackHandle> m_sortCallbacks;
		WIHandle m_hRowHeader;
		std::vector<WIHandle> m_rows;
		void UpdateTableBounds();
		void OnRowSelected(WITableRow *row);
		std::vector<WIHandle> m_selectedRows;
		static bool SortRows(bool bAsc, unsigned int col, const WIHandle &a, const WIHandle &b);
		void Sort(bool bAsc, unsigned int col = 0);
		virtual void DoUpdate() override;
		WITableRow *GetHeaderRow();
		void InitializeRow(WITableRow *row, bool bHeader = false);
		void OnRowCellCreated(WITableCell *cell);
		static void OnHeaderCellPressed(WITableCell *cell);
		static void OnScrollOffsetChanged(unsigned int offset, void *userData);
		virtual void UpdateHeaderRowHeight(WITableRow *pRow, float defHeight);
		virtual float UpdateRowHeights(float yOffset, float defHeight);
		virtual void OnChildAdded(WIBase *child) override;
		void UpdateCell(const WITableCell &cell);
		template<class TRow>
		TRow *AddRow()
		{
			WIBase *parent = this;
			if(m_bScrollable == true && m_hScrollContainer.IsValid())
				parent = m_hScrollContainer.get();
			auto *pRow = WGUI::GetInstance().Create<TRow>(parent);
			auto hRow = pRow->GetHandle();
			pRow->AddStyleClass("table_row");
			m_rows.push_back(hRow);
			InitializeRow(pRow, false);
			return pRow;
		}
		friend void WITableCell::SetRowSpan(int32_t span);
		friend void WITableCell::SetColSpan(int32_t span);
	};

	class WITableCell;
	class DLLCLIENT WITableRow : public WIContainer {
	  protected:
		std::vector<WIHandle> m_cells;
		bool m_bSelected;
		std::unordered_map<unsigned int, int> m_cellWidths;
		void SetCellCount(unsigned int numCells);
		void UpdateCell(const WITableCell &cell);
		friend void WITable::UpdateCell(const WITableCell &cell);
		friend void WITableCell::SetRowSpan(int32_t span);
		friend void WITableCell::SetColSpan(int32_t span);
	  public:
		WITableRow();
		virtual ~WITableRow() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		virtual void OnChildAdded(WIBase *child) override;
		void SetCellWidth(unsigned int col, int width);
		WIHandle SetValue(unsigned int col, std::string val);
		std::string GetValue(uint32_t col) const;
		bool GetValue(uint32_t col, std::string &val) const;
		WITableCell *InsertElement(unsigned int col, WIBase *el);
		WITableCell *InsertElement(unsigned int col, WIHandle hElement);
		virtual util::EventReply MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods) override;
		void Select();
		void Deselect();
		bool IsSelected();
		// Clears the cell, but does NOT remove the cell GUI element!
		void DetachCell(uint32_t colId);
		void AttachCell(uint32_t colid, const WITableCell &cell);
		unsigned int GetCellCount() const;
		WITableCell *GetCell(unsigned int id) const;
		WITable *GetTable();
	};
};
