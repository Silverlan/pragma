/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WILINEGRAPH_H__
#define __WILINEGRAPH_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WILineGraph
	: public WIBase
{
protected:
	WIHandle m_hOutline;
	std::vector<float> m_values;
	std::vector<WIHandle> m_lines;
	virtual void DoUpdate() override;
	void UpdateGraph();
	void UpdateLines();
public:
	WILineGraph();
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	using WIBase::SetSize;
	void SetValue(uint32_t idx,float val);
	void SetSegmentCount(uint32_t count);
	uint32_t GetSegmentCount() const;
};

#endif