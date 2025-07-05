// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WILINEGRAPH_H__
#define __WILINEGRAPH_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WILineGraph : public WIBase {
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
	virtual void SetSize(int x, int y) override;
	using WIBase::SetSize;
	void SetValue(uint32_t idx, float val);
	void SetSegmentCount(uint32_t count);
	uint32_t GetSegmentCount() const;
};

#endif
