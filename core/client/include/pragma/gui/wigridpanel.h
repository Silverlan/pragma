// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WIGRIDPANEL_H__
#define __WIGRIDPANEL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/witable.h"

class DLLCLIENT WIGridPanel : public WITable {
  public:
	enum class DLLCLIENT ResizeMode { FitToChildren = 0, Uniform };
  private:
	using WITable::AddHeaderRow;
  protected:
	ResizeMode m_resizeMode;
	uint32_t m_numColumns;
	void FillRows(uint32_t count);
	virtual float UpdateRowHeights(float yOffset, float defHeight) override;
	virtual void DoUpdate() override;
  public:
	WIGridPanel();
	virtual WITableRow *AddRow() override;
	virtual void SetSize(int x, int y) override;
	WITableCell *AddItem(WIBase *el, uint32_t row, uint32_t col);
	uint32_t GetColumnCount() const;
};

#endif
