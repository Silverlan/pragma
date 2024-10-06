/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/witable.h"
#include <wgui/types/witext.h>
#include <wgui/types/wirect.h>
#include <wgui/types/wiscrollbar.h>
#include "pragma/gui/wiscrollcontainer.h"
#include <prosper_window.hpp>

import pragma.string.unicode;

LINK_WGUI_TO_CLASS(WITable, WITable);
LINK_WGUI_TO_CLASS(WITableRow, WITableRow);
LINK_WGUI_TO_CLASS(WITableCell, WITableCell);

extern DLLCLIENT CEngine *c_engine;
extern ClientState *client;
#pragma optimize("", off)

WITable::SortData::SortData(WITable *t, bool bAsc, unsigned int col) : table(t), ascending(bAsc), column(col) {}
bool WITable::SortData::operator()(const WIHandle &a, const WIHandle &b)
{
	auto &func = table->GetSortFunction();
	if(func) {
		auto *rowA = dynamic_cast<const WITableRow *>(a.get());
		auto *rowB = dynamic_cast<const WITableRow *>(b.get());
		if(!rowA || !rowB)
			return false;
		return func(*rowA, *rowB, column, ascending);
	}
	return WITable::SortRows(ascending, column, a, b);
}

WITable::WITable() : WIContainer(), m_bSortAsc(true), m_sortColumn(CUInt32(-1)), m_rowHeight(-1), m_bSortable(false), m_bScrollable(false) { RegisterCallback<void, WITableRow *>("OnRowCreated"); }

WITable::~WITable() { SetSortable(false); }

void WITable::OnScrollOffsetChanged(unsigned int, void *)
{
	//WITable *t = static_cast<WITable*>(userData);
}

void WITable::RemoveRow(uint32_t rowIdx)
{
	if(rowIdx >= m_rows.size())
		return;
	auto &hRow = m_rows[rowIdx];
	if(hRow.IsValid())
		hRow->Remove();
	m_rows.erase(m_rows.begin() + rowIdx);
	Resize();
}

void WITable::SizeToContents(bool x, bool y)
{
	if(m_bScrollable == true && m_hScrollContainer.IsValid()) {
		m_hScrollContainer->SizeToContents(x, y);
		//auto sz = m_hScrollContainer.get()->GetSize();
		//SetSize(sz.x,sz.y);
		//return;
	}
	WIBase::SizeToContents(x, y);
}

void WITable::OnChildAdded(WIBase *child)
{
	WIBase::OnChildAdded(child);
	ScheduleUpdate();
}

bool WITable::SortRows(bool bAsc, unsigned int col, const WIHandle &a, const WIHandle &b)
{
	if(a.IsValid() == false || b.IsValid() == false)
		return false;
	const pragma::string::Utf8String *textA = nullptr;
	const pragma::string::Utf8String *textB = nullptr;
	const WITableRow *rowA = a.get<const WITableRow>();
	if(rowA != nullptr) {
		WITableCell *cellA = rowA->GetCell(col);
		if(cellA != nullptr) {
			WIText *vA = dynamic_cast<WIText *>(cellA->GetFirstElement());
			if(vA != nullptr)
				textA = &vA->GetText();
		}
	}
	const WITableRow *rowB = b.get<const WITableRow>();
	if(rowB != nullptr) {
		WITableCell *cellB = rowB->GetCell(col);
		if(cellB != nullptr) {
			WIText *vB = dynamic_cast<WIText *>(cellB->GetFirstElement());
			if(vB != nullptr)
				textB = &vB->GetText();
		}
	}
	return bAsc == true ? ((((textA != nullptr) ? *textA : "") < ((textB != nullptr) ? *textB : "")) ? true : false) : ((((textB != nullptr) ? *textB : "") < ((textA != nullptr) ? *textA : "")) ? true : false);
}
void WITable::Sort(bool bAsc, unsigned int col)
{
	SortData sort(this, bAsc, col);
	std::sort(m_rows.begin(), m_rows.end(), sort);
	ScheduleUpdate();
}

void WITable::OnRowCellCreated(WITableCell *cell)
{
	cell->SetMouseInputEnabled(true);
	m_sortCallbacks.push_back(cell->AddCallback("OnMousePressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([this, cell](util::EventReply *reply) -> CallbackReturnType {
		*reply = util::EventReply::Handled;
		OnHeaderCellPressed(cell);
		return CallbackReturnType::HasReturnValue;
	})));
}

void WITable::Sort() { Sort(m_bSortAsc, m_sortColumn); }

void WITable::OnHeaderCellPressed(WITableCell *cell)
{
	WITableRow *row = dynamic_cast<WITableRow *>(cell->GetParent());
	if(row == nullptr)
		return;
	WITable *t = dynamic_cast<WITable *>(row->GetParent());
	if(t == nullptr)
		return;
	for(unsigned int i = 0; i < row->GetCellCount(); i++) {
		WITableCell *cellOther = row->GetCell(i);
		if(cellOther == cell) {
			if(i != t->m_sortColumn)
				t->m_bSortAsc = true;
			t->Sort(t->m_bSortAsc, i);

			if(t->m_hSortArrow.IsValid()) {
				WIShape *pArrow = t->m_hSortArrow.get<WIShape>();
				if(pArrow != nullptr) {
					pArrow->SetParent(cell);
					pArrow->SetVisible(true);
					pArrow->SetY(CInt32(cell->GetHeight() * 0.5f - pArrow->GetHeight() * 0.5f));
					int x = 0;
					WIText *pText = dynamic_cast<WIText *>(cell->GetFirstElement());
					if(pText != nullptr)
						x = pText->GetX() + pText->GetWidth() + 5;
					pArrow->SetX(x);
					pArrow->ClearVertices();
					if(t->m_bSortAsc == true) {
						pArrow->AddVertex(Vector2(0.f, -1.f));
						pArrow->AddVertex(Vector2(1.f, 1.f));
						pArrow->AddVertex(Vector2(-1.f, 1.f));
					}
					else {
						pArrow->AddVertex(Vector2(0.f, 1.f));
						pArrow->AddVertex(Vector2(-1.f, -1.f));
						pArrow->AddVertex(Vector2(1.f, -1.f));
					}
					pArrow->Update();
				}
			}
			t->m_sortColumn = i;
			t->m_bSortAsc = !t->m_bSortAsc;
			break;
		}
	}
}

void WITable::SetSortable(bool b)
{
	if(b == m_bSortable)
		return;
	m_bSortable = b;
	if(b == false) {
		std::vector<CallbackHandle>::iterator it;
		for(it = m_sortCallbacks.begin(); it != m_sortCallbacks.end(); it++) {
			CallbackHandle &hCallback = *it;
			if(hCallback.IsValid())
				hCallback.Remove();
		}
		if(m_hSortArrow.IsValid())
			m_hSortArrow->Remove();
		WITableRow *row = GetHeaderRow();
		if(row == nullptr)
			return;
		for(unsigned int i = 0; i < row->GetCellCount(); i++) {
			WITableCell *cell = row->GetCell(i);
			if(cell != nullptr)
				cell->SetMouseInputEnabled(false);
		}
		return;
	}
	if(!m_hSortArrow.IsValid()) {
		WIShape *pArrow = WGUI::GetInstance().Create<WIShape>();
		pArrow->AddStyleClass("table_sort_arrow");
		pArrow->SetSize(10, 10);
		pArrow->SetColor(1.f, 1.f, 1.f, 1.f);
		pArrow->SetMouseInputEnabled(true);
		pArrow->Update();
		pArrow->SetVisible(false);
		m_hSortArrow = pArrow->GetHandle();
	}
	WITableRow *row = GetHeaderRow();
	if(row == nullptr)
		return;
	for(unsigned int i = 0; i < row->GetCellCount(); i++) {
		WITableCell *cell = row->GetCell(i);
		if(cell != nullptr) {
			cell->SetMouseInputEnabled(true);
			m_sortCallbacks.push_back(cell->AddCallback("OnMousePressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([this, cell](util::EventReply *reply) -> CallbackReturnType {
				*reply = util::EventReply::Handled;
				OnHeaderCellPressed(cell);
				return CallbackReturnType::HasReturnValue;
			})));
		}
	}
	m_sortCallbacks.push_back(row->AddCallback("OnCellCreated", FunctionCallback<void, WITableCell *>::Create(std::bind(&WITable::OnRowCellCreated, this, std::placeholders::_1))));
}
bool WITable::IsSortable() const { return m_bSortable; }

void WITable::SetSortFunction(const std::function<bool(const WITableRow &, const WITableRow &, uint32_t, bool)> &sortFunc) { m_sortFunction = sortFunc; }
const std::function<bool(const WITableRow &, const WITableRow &, uint32_t, bool)> &WITable::GetSortFunction() const { return m_sortFunction; }

void WITable::SetScrollable(bool b)
{
	if(m_bScrollable == b)
		return;
	m_bScrollable = b;
	SetScrollInputEnabled(b);
	if(b == true) {
		if(!m_hScrollContainer.IsValid())
			m_hScrollContainer = CreateChild<WIScrollContainer>();
		WIScrollContainer *sc = m_hScrollContainer.get<WIScrollContainer>();
		sc->ScheduleUpdate();
		std::vector<WIHandle>::iterator it;
		for(it = m_rows.begin(); it != m_rows.end(); it++) {
			WIHandle &hRow = *it;
			if(hRow.IsValid())
				hRow->SetParent(sc);
		}
	}
	else {
		std::vector<WIHandle>::iterator it;
		for(it = m_rows.begin(); it != m_rows.end(); it++) {
			WIHandle &hRow = *it;
			if(hRow.IsValid())
				hRow->SetParent(this);
		}
		if(m_hScrollContainer.IsValid())
			m_hScrollContainer->Remove();
	}
	ScheduleUpdate();
}
bool WITable::IsScrollable() const { return m_bScrollable; }

void WITable::Clear(bool bAll)
{
	if(m_hScrollContainer.IsValid() == true) {
		auto *pScrollBar = static_cast<WIScrollContainer *>(m_hScrollContainer.get())->GetVerticalScrollBar();
		if(pScrollBar != nullptr)
			pScrollBar->SetScrollOffset(0);
		pScrollBar = static_cast<WIScrollContainer *>(m_hScrollContainer.get())->GetHorizontalScrollBar();
		if(pScrollBar != nullptr)
			pScrollBar->SetScrollOffset(0);
	}
	for(auto it = m_rows.begin(); it != m_rows.end(); it++) {
		auto &hRow = *it;
		if(hRow.IsValid())
			hRow->RemoveSafely();
	}
	m_rows.clear();
	if(bAll == true) {
		if(m_hRowHeader.IsValid()) {
			m_hRowHeader->RemoveSafely();
			m_hRowHeader = WIHandle {};
		}
	}
}

void WITable::Initialize()
{
	WIBase::Initialize();
	AddStyleClass("table");
}
WITableRow *WITable::GetHeaderRow()
{
	if(!m_hRowHeader.IsValid())
		return nullptr;
	return m_hRowHeader.get<WITableRow>();
}
WITable::SelectableMode WITable::GetSelectableMode() const { return m_selectableMode; }
void WITable::SetSelectable(SelectableMode mode)
{
	if(m_selectableMode == mode)
		return;
	m_selectableMode = mode;
	for(auto &hRow : m_rows) {
		if(hRow.IsValid() == false)
			continue;
		hRow->SetMouseInputEnabled(true);
	}

	if(m_selectedRows.empty() || mode == SelectableMode::Multi)
		return;
	auto selectedRows = m_selectedRows;
	auto itStart = selectedRows.begin();
	if(mode == SelectableMode::Single)
		++itStart; // We'll want to keep the first row selected
	for(auto it = itStart; it != selectedRows.end(); ++it) {
		auto &hRow = *it;
		if(hRow.IsValid() == false)
			continue;
		static_cast<WITableRow *>(hRow.get())->Deselect();
	}
}
void WITable::DeselectAllRows()
{
	auto selectedRows = m_selectedRows;
	for(auto &hRow : selectedRows) {
		if(hRow.IsValid() == false)
			continue;
		static_cast<WITableRow *>(hRow.get())->Deselect();
	}
	m_selectedRows.clear();
}
void WITable::SelectRow(WITableRow &row)
{
	auto deselect = (m_selectableMode == SelectableMode::Single);
	if(m_selectableMode == SelectableMode::Multi) {
		auto &window = WGUI::GetInstance().GetContext().GetWindow();
		auto lctrl = window->GetKeyState(GLFW::Key::LeftControl);
		auto rctrl = window->GetKeyState(GLFW::Key::RightControl);
		deselect = !(lctrl == GLFW::KeyState::Press || lctrl == GLFW::KeyState::Held || rctrl == GLFW::KeyState::Press || rctrl == GLFW::KeyState::Held);
	}
	if(deselect)
		DeselectAllRows();
	row.Select();
}
void WITable::OnRowSelected(WITableRow *row) { m_selectedRows.push_back(row->GetHandle()); }
void WITable::InitializeRow(WITableRow *row, bool bHeader)
{
	if(m_bSortable == true && bHeader == true) {
		row->SetMouseInputEnabled(true);
		m_sortCallbacks.push_back(row->AddCallback("OnCellCreated", FunctionCallback<void, WITableCell *>::Create(std::bind(&WITable::OnRowCellCreated, this, std::placeholders::_1))));
	}
	else
		row->SetMouseInputEnabled(GetSelectableMode() != SelectableMode::None);
	if(m_rowHeight != -1)
		row->SetHeight(m_rowHeight);
	std::unordered_map<unsigned int, int>::iterator itCol;
	for(itCol = m_columnWidths.begin(); itCol != m_columnWidths.end(); itCol++)
		row->SetCellWidth(itCol->first, itCol->second);
	ScheduleUpdate();
	CallCallbacks<void, WITableRow *>("OnRowCreated", row);
}
unsigned int WITable::GetRowCount() const { return CUInt32(m_rows.size()); }
WITableRow *WITable::AddRow()
{
	auto *pRow = AddRow<WITableRow>();
	if(pRow != nullptr)
		pRow->AddStyleClass("table_row_offset");
	return pRow;
}
WITableRow *WITable::AddHeaderRow()
{
	WIHandle hRow;
	if(m_hRowHeader.IsValid()) {
		WIBase *parent = this;
		if(m_bScrollable == true && m_hScrollContainer.IsValid())
			parent = m_hScrollContainer.get();
		hRow = WGUI::GetInstance().Create<WITableRow>(parent)->GetHandle();
	}
	else
		hRow = CreateChild<WITableRow>();
	hRow->AddStyleClass("table_row_header");
	if(m_hRowHeader.IsValid()) // Treat it as a regular row, but styled as a header
	{
		m_rows.push_back(hRow);
		InitializeRow(hRow.get<WITableRow>(), false);
		return hRow.get<WITableRow>();
	}
	m_hRowHeader = hRow;
	InitializeRow(hRow.get<WITableRow>(), true);
	return hRow.get<WITableRow>();
}
void WITable::SetRowHeight(int h)
{
	if(h < -1)
		h = -1;
	m_rowHeight = h;
	ScheduleUpdate();
}
int WITable::GetRowHeight() const { return m_rowHeight; }
const std::vector<WIHandle> &WITable::GetSelectedRows() const { return m_selectedRows; }
WIHandle WITable::GetFirstSelectedRow() const { return (m_selectedRows.empty() == false) ? m_selectedRows.front() : WIHandle {}; }
WITableRow *WITable::GetRow(unsigned int id) const
{
	if(id >= m_rows.size())
		return nullptr;
	const WIHandle &hRow = m_rows[id];
	if(!hRow.IsValid())
		return nullptr;
	return const_cast<WITableRow *>(hRow.get<const WITableRow>());
}

void WITable::UpdateCell(const WITableCell &cell)
{
	const auto max = std::numeric_limits<decltype(m_rows.size())>::max();
	auto startIdx = max;
	auto colIdx = max;
	for(auto it = m_rows.begin(); it != m_rows.end(); ++it) {
		auto &hRow = *it;
		if(!hRow.IsValid())
			continue;
		auto *pRow = static_cast<WITableRow *>(hRow.get());
		auto numCells = pRow->GetCellCount();
		for(auto i = decltype(numCells) {0}; i < numCells; ++i) {
			auto *pRowCell = pRow->GetCell(i);
			if(pRowCell != &cell)
				continue;
			if(startIdx == max) {
				startIdx = it - m_rows.begin();
				colIdx = i;
			}
			pRow->DetachCell(i);
		}
	}
	assert(startIdx != max);
	if(startIdx == max)
		return;
	auto rowSpan = cell.GetRowSpan();
	auto requiredRowCount = startIdx + rowSpan;
	for(auto i = m_rows.size(); i < requiredRowCount; ++i)
		AddRow();

	for(auto i = startIdx; i < startIdx + rowSpan; ++i) {
		auto &hRow = m_rows[i];
		if(!hRow.IsValid())
			continue;
		auto *pRow = static_cast<WITableRow *>(hRow.get());
		pRow->AttachCell(static_cast<uint32_t>(colIdx), cell);
		pRow->UpdateCell(cell);
	}
}

void WITable::SetColumnWidth(unsigned int col, int width)
{
	m_columnWidths[col] = width;
	std::vector<WIHandle>::iterator it;
	for(it = m_rows.begin(); it != m_rows.end(); it++) {
		WIHandle &hRow = *it;
		if(hRow.IsValid()) {
			WITableRow *pRow = hRow.get<WITableRow>();
			std::unordered_map<unsigned int, int>::iterator itCol;
			for(itCol = m_columnWidths.begin(); itCol != m_columnWidths.end(); itCol++)
				pRow->SetCellWidth(itCol->first, itCol->second);
		}
	}
	if(m_hRowHeader.IsValid()) {
		WITableRow *pRow = m_hRowHeader.get<WITableRow>();
		std::unordered_map<unsigned int, int>::iterator itCol;
		for(itCol = m_columnWidths.begin(); itCol != m_columnWidths.end(); itCol++)
			pRow->SetCellWidth(itCol->first, itCol->second);
	}
}

void WITable::UpdateTableBounds()
{
	auto numRows = m_rows.size();
	if(numRows == 0 && !m_hRowHeader.IsValid())
		return;
	auto x = GetWidth();
	auto y = GetHeight();
	float rowHeight;
	if(m_rowHeight != -1)
		rowHeight = CFloat(m_rowHeight);
	else {
		rowHeight = CFloat((y - GetPaddingTop() - GetPaddingBottom()) / (m_hRowHeader.IsValid() ? (numRows + 1) : numRows));
	}
	auto yRow = static_cast<float>(GetPaddingTop());
	if(m_hRowHeader.IsValid()) {
		UpdateHeaderRowHeight(static_cast<WITableRow *>(m_hRowHeader.get()), rowHeight);
		yRow += m_hRowHeader->GetHeight();
	}
	if(m_hScrollContainer.IsValid()) {
		auto *sc = m_hScrollContainer.get<WIScrollContainer>();
		sc->SetY(CInt32(yRow));
		sc->SetSize(x, CInt32(y - yRow));
		yRow = 0;
	}
	if(numRows > 0) {
		yRow = UpdateRowHeights(yRow, rowHeight);
		if(m_hScrollContainer.IsValid()) {
			auto *sc = m_hScrollContainer.get<WIScrollContainer>();
			sc->Update();
			auto wRow = sc->GetContentWidth();
			for(size_t i = 0; i < numRows; i++) {
				WIHandle &hRow = m_rows[i];
				if(hRow.IsValid())
					hRow->SetWidth(wRow);
			}
		}
	}
}

void WITable::DoUpdate()
{
	UpdateTableBounds();
	WIBase::DoUpdate();
}

uint32_t WITable::GetRowIndex(WITableRow *pRow) const
{
	for(auto i = decltype(m_rows.size()) {0}; i < m_rows.size(); ++i) {
		auto &hRow = m_rows[i];
		if(hRow.get() == pRow)
			return static_cast<uint32_t>(i);
	}
	return std::numeric_limits<uint32_t>::max();
}

void WITable::MoveRow(WITableRow *a, WITableRow *pos, bool bAfter)
{
	auto idx = GetRowIndex(a);
	if(idx == std::numeric_limits<uint32_t>::max())
		return;
	auto idxOther = GetRowIndex(pos);
	if(idxOther == std::numeric_limits<uint32_t>::max() || idx == idxOther)
		return;
	m_rows.erase(m_rows.begin() + idx);
	if(idxOther > idx)
		--idxOther;
	if(bAfter == false) {
		m_rows.insert(m_rows.begin() + idxOther, a->GetHandle());
		return;
	}
	if(idxOther >= m_rows.size()) {
		m_rows.push_back(a->GetHandle());
		return;
	}
	m_rows.insert(m_rows.begin() + idxOther + 1, a->GetHandle());
}

void WITable::UpdateHeaderRowHeight(WITableRow *pRow, float defHeight)
{
	pRow->SetSize(GetWidth(), CInt32(defHeight));
	pRow->SetY(CInt32(0));
}

float WITable::UpdateRowHeights(float yOffset, float defHeight)
{
	auto w = GetWidth();
	auto &padding = GetPadding();
	auto numRows = m_rows.size();
	for(size_t i = 0; i < numRows; i++) {
		auto &hRow = m_rows[i];
		if(hRow.IsValid() && hRow->IsVisible()) {
			auto *row = hRow.get<WITableRow>();
			row->SetSize(w - padding[umath::to_integral(WIContainer::Padding::Left)] - padding[umath::to_integral(WIContainer::Padding::Right)], CInt32(defHeight));
			row->SetX(padding[umath::to_integral(WIContainer::Padding::Left)]);
			row->SetY(CInt32(yOffset));
			yOffset += defHeight;
		}
	}
	return yOffset;
}

void WITable::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	UpdateTableBounds();
}

///////////////////////////

WITableRow::WITableRow() : WIContainer(), m_bSelected(false)
{
	RegisterCallback<void>("OnSelected");
	RegisterCallback<void>("OnDeselected");
	RegisterCallback<void, WITableCell *>("OnCellCreated");
}
WITableRow::~WITableRow() {}
void WITableRow::OnChildAdded(WIBase *child)
{
	WIBase::OnChildAdded(child);
	ScheduleUpdate();
}
void WITableRow::DetachCell(uint32_t colId)
{
	auto numCells = m_cells.size();
	if(colId >= numCells)
		return;
	if(colId == numCells - 1) {
		m_cells.erase(m_cells.end() - 1);
		return;
	}
	m_cells[colId] = WIHandle {};
}
void WITableRow::AttachCell(uint32_t colid, const WITableCell &cell)
{
	if(colid >= m_cells.size())
		SetCellCount(colid + 1);
	m_cells[colid] = cell.GetHandle();
}
void WITableRow::SetCellWidth(unsigned int col, int width)
{
	m_cellWidths[col] = width;
	ScheduleUpdate();
}
util::EventReply WITableRow::MouseCallback(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods)
{
	if(WIBase::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(button == GLFW::MouseButton::Left && state == GLFW::KeyState::Press) {
		auto *t = GetTable();
		if(t)
			t->SelectRow(*this);
	}
	return util::EventReply::Handled;
}
unsigned int WITableRow::GetCellCount() const { return CUInt32(m_cells.size()); }
WITableCell *WITableRow::GetCell(unsigned int id) const
{
	if(id >= m_cells.size())
		return nullptr;
	const WIHandle &hCell = m_cells[id];
	if(!hCell.IsValid())
		return nullptr;
	return const_cast<WITableCell *>(static_cast<const WITableCell *>(hCell.get()));
}
WITable *WITableRow::GetTable()
{
	WIBase *parent = GetParent();
	for(int i = 0; i < 3; i++) {
		if(parent == nullptr)
			return nullptr;
		WITable *t = dynamic_cast<WITable *>(parent);
		if(t != nullptr)
			return t;
		parent = parent->GetParent();
	}
	return nullptr;
}
void WITableRow::Select()
{
	if(m_bSelected == true)
		return;
	WITable *t = GetTable();
	if(t == nullptr || t->GetSelectableMode() == WITable::SelectableMode::None)
		return;
	m_bSelected = true;
	t->OnRowSelected(this);
	CallCallbacks<void>("OnSelected");
}
void WITableRow::Deselect()
{
	if(m_bSelected == false)
		return;
	CallCallbacks<void>("OnDeselected");
	m_bSelected = false;
}
bool WITableRow::IsSelected() { return m_bSelected; }
void WITableRow::UpdateCell(const WITableCell &cell)
{
	const auto max = std::numeric_limits<decltype(m_cells.size())>::max();
	;
	auto startIdx = max;
	for(auto it = m_cells.begin(); it != m_cells.end();) {
		auto &hCell = *it;
		if(hCell.get() == &cell) {
			it = m_cells.erase(it);
			if(startIdx == max)
				startIdx = it - m_cells.begin();
		}
		else
			++it;
	}
	assert(startIdx != max);
	if(startIdx == max) // We don't own this cell?
		return;
	auto colSpan = cell.GetColSpan();
	auto requiredCellCount = startIdx + colSpan;
	if(requiredCellCount >= m_cells.size())
		SetCellCount(static_cast<uint32_t>(requiredCellCount));
	for(auto i = startIdx; i < startIdx + colSpan; ++i)
		m_cells[i] = cell.GetHandle();
}
void WITableRow::SetCellCount(unsigned int numCells)
{
	auto numCellsCur = m_cells.size();
	if(numCellsCur > numCells) {
		for(auto i = (numCellsCur - 1); i >= numCells; i--) {
			WIHandle &hEl = m_cells[i];
			if(hEl.IsValid())
				hEl->Remove();
			m_cells.erase(m_cells.begin() + i);
		}
		return;
	}
	else {
		for(unsigned int i = 0; i < (numCells - numCellsCur); i++) {
			WIHandle hCell = CreateChild<WITableCell>();
			m_cells.push_back(hCell);
			CallCallbacks<void, WITableCell *>("OnCellCreated", hCell.get<WITableCell>());
		}
	}
	ScheduleUpdate();
}
void WITableRow::Initialize() { WIBase::Initialize(); }
void WITableRow::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);

	auto numCells = m_cells.size();
	std::vector<int> widths(numCells);
	std::fill(widths.begin(), widths.end(), -1);
	std::unordered_map<unsigned int, int>::iterator itCell;
	int widthTotal = 0;
	int numUsed = 0;
	for(itCell = m_cellWidths.begin(); itCell != m_cellWidths.end(); itCell++) {
		if(itCell->first < widths.size()) {
			widths[itCell->first] = itCell->second;
			widthTotal += itCell->second;
			numUsed++;
		}
	}
	float widthLeft = 0;
	if(numUsed < widths.size()) {
		widthLeft = float(x - widthTotal) / float(widths.size() - numUsed);
		if(widthLeft < 0)
			widthLeft = 0;
	}
	std::vector<int>::iterator it;
	for(it = widths.begin(); it != widths.end(); it++) {
		int &w = *it;
		if(w == -1)
			w = CInt32(widthLeft);
	}

	Vector2 offset(0.f, 0.f);
	//int32_t xoffset = 0;
	for(auto i = decltype(numCells) {0}; i < numCells; ++i) {
		auto &hCell = m_cells[i];
		if(!hCell.IsValid())
			continue;
		auto *cell = hCell.get<WITableCell>();
		int32_t width = 0;
		auto colSpan = cell->GetColSpan();
		for(auto j = decltype(colSpan) {0}; j < colSpan; ++j)
			width += widths[i + j];

		cell->SetSize(width, y);
		cell->SetPos(umath::round(offset.x), umath::round(offset.y));
		offset.x += width;
	}
}

WIHandle WITableRow::SetValue(unsigned int col, std::string val)
{
	auto *pCell = GetCell(col);
	if(pCell != nullptr) {
		auto *pEl = pCell->GetFirstChild("WIText");
		if(pEl != nullptr) {
			auto *pText = static_cast<WIText *>(pEl);
			pText->SetText(val);
			pText->SizeToContents();
			return pText->GetHandle();
		}
	}
	WIHandle hLabel = CreateChild<WIText>();
	WIText *text = hLabel.get<WIText>();
	text->SetText(val);
	text->SizeToContents();
	InsertElement(col, hLabel);
	return hLabel;
}
std::string WITableRow::GetValue(uint32_t col) const
{
	std::string r;
	GetValue(col, r);
	return r;
}
bool WITableRow::GetValue(uint32_t col, std::string &val) const
{
	auto *pCell = GetCell(col);
	if(pCell == nullptr)
		return false;
	auto *pEl = pCell->GetFirstChild("WIText");
	if(pEl == nullptr)
		return false;
	val = static_cast<WIText *>(pEl)->GetText().cpp_str();
	return true;
}
WITableCell *WITableRow::InsertElement(unsigned int col, WIBase *el)
{
	if(col >= GetCellCount())
		SetCellCount(col + 1);
	WIHandle &hCell = m_cells[col];
	if(!hCell.IsValid())
		return nullptr;
	WITableCell *cell = hCell.get<WITableCell>();
	el->SetParent(cell);
	return cell;
}
WITableCell *WITableRow::InsertElement(unsigned int col, WIHandle hElement) { return InsertElement(col, hElement.get()); }

///////////////////////////

WITableCell::WITableCell() : WIContainer(), m_span {1, 1} {}
WITableCell::~WITableCell() {}
void WITableCell::Initialize()
{
	WIBase::Initialize();
	AddStyleClass("table_cell");
}
void WITableCell::OnChildAdded(WIBase *child)
{
	WIBase::OnChildAdded(child);
	ScheduleUpdate();
}
void WITableCell::SetSize(int x, int y) { WIBase::SetSize(x, y); }
WIBase *WITableCell::GetFirstElement()
{
	if(m_children.empty())
		return nullptr;
	std::vector<WIHandle>::iterator it;
	for(it = m_children.begin(); it != m_children.end(); it++) {
		WIHandle &hChild = *it;
		if(hChild.IsValid() && hChild->IsBackgroundElement() == false)
			return hChild.get();
	}
	return nullptr;
}
void WITableCell::DoUpdate()
{
	int32_t xOffset = 0;
	for(auto &hChild : m_children) {
		if(!hChild.IsValid())
			continue;
		hChild->SetX(xOffset);
		xOffset += hChild->GetWidth();
	}
	WIContainer::DoUpdate();
}
WITableRow *WITableCell::GetRow() const
{
	auto *parent = GetParent();
	for(int32_t i = 0; i < 3; ++i) {
		if(parent == nullptr)
			return nullptr;
		auto *t = dynamic_cast<WITableRow *>(parent);
		if(t != nullptr)
			return t;
		parent = parent->GetParent();
	}
	return nullptr;
}

void WITableCell::SetRowSpan(int32_t span)
{
	m_span.first = span;
	auto *pRow = GetRow();
	if(pRow == nullptr)
		return;
	auto *pTable = pRow->GetTable();
	if(pTable == nullptr)
		return;
	pTable->UpdateCell(*this);
}
void WITableCell::SetColSpan(int32_t span) { m_span.second = span; }
int32_t WITableCell::GetRowSpan() const { return m_span.first; }
int32_t WITableCell::GetColSpan() const { return m_span.second; }
