/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WIGRIDPANEL_H__
#define __WIGRIDPANEL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/witable.h"

class DLLCLIENT WIGridPanel
	: public WITable
{
public:
	enum class DLLCLIENT ResizeMode
	{
		FitToChildren = 0,
		Uniform
	};
private:
	using WITable::AddHeaderRow;
protected:
	ResizeMode m_resizeMode;
	uint32_t m_numColumns;
	void FillRows(uint32_t count);
	virtual float UpdateRowHeights(float yOffset,float defHeight) override;
	virtual void DoUpdate() override;
public:
	WIGridPanel();
	virtual WITableRow *AddRow() override;
	virtual void SetSize(int x,int y) override;
	WITableCell *AddItem(WIBase *el,uint32_t row,uint32_t col);
	uint32_t GetColumnCount() const;
};

#endif
