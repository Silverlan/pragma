// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.grid_panel;

import :client_state;

pragma::gui::types::WIGridPanel::WIGridPanel() : WITable(), m_resizeMode(ResizeMode::FitToChildren), m_numColumns(0) {}

void pragma::gui::types::WIGridPanel::SetSize(int x, int y)
{
	WITable::SetSize(x, y);
	//auto &children = *GetChildren();
}

void pragma::gui::types::WIGridPanel::FillRows(uint32_t count)
{
	auto numRows = GetRowCount();
	for(auto i = numRows; i < count; ++i)
		AddRow();
}

float pragma::gui::types::WIGridPanel::UpdateRowHeights(float yOffset, float defHeight)
{
	if(m_resizeMode == ResizeMode::FitToChildren) {
		auto w = GetWidth();
		auto numRows = m_rows.size();
		for(auto i = decltype(numRows) {0}; i < numRows; ++i) {
			auto &hRow = m_rows[i];
			if(hRow.IsValid() && hRow->IsVisible()) {
				auto *row = static_cast<WITableRow *>(hRow.get());

				auto hRow = 0.f;
				auto numCells = row->GetCellCount();
				for(auto j = decltype(numCells) {0}; j < numCells; ++j) {
					auto *pCell = row->GetCell(j);
					if(pCell == nullptr)
						continue;
					auto &children = *pCell->GetChildren();
					for(auto &hChild : children) {
						if(hChild.IsValid() == false)
							continue;
						hRow = math::max(hRow, static_cast<float>(hChild->GetHeight()));
					}
				}
				row->SetSize(w, CInt32(hRow));
				row->SetY(CInt32(yOffset));
				yOffset += hRow;
			}
		}
		return yOffset;
	}
	return WITable::UpdateRowHeights(yOffset, defHeight);
}

void pragma::gui::types::WIGridPanel::DoUpdate()
{
	switch(m_resizeMode) {
	case ResizeMode::Uniform:
		{
			auto numRows = GetRowCount();
			Vector2 sz(GetWidth(), GetHeight());
			sz.x /= static_cast<float>(m_numColumns);
			sz.y /= static_cast<float>(numRows);
			auto rowHeight = sz.y;
			for(auto i = decltype(numRows) {0}; i < numRows; ++i) {
				auto *pRow = GetRow(i);
				if(pRow != nullptr)
					pRow->SetHeight(static_cast<int32_t>(rowHeight));
			}
		}
	}
	WITable::DoUpdate();
}

pragma::gui::types::WITableRow *pragma::gui::types::WIGridPanel::AddRow() { return WITable::AddRow<WITableRow>(); }

uint32_t pragma::gui::types::WIGridPanel::GetColumnCount() const { return m_numColumns; }

pragma::gui::types::WITableCell *pragma::gui::types::WIGridPanel::AddItem(WIBase *el, uint32_t rowId, uint32_t colId)
{
	FillRows(rowId + 1);
	auto *pRow = GetRow(rowId);
	if(pRow == nullptr) {
		el->SetParent(this);
		return nullptr;
	}
	auto *pCell = pRow->InsertElement(colId, el);
	m_numColumns = math::max(m_numColumns, colId + 1);
	return pCell;
}
