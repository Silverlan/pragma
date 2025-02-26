/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/gui/wilinegraph.h"
#include <wgui/types/wirect.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(WILineGraph, WILineGraph);

WILineGraph::WILineGraph() : WIBase() {}

void WILineGraph::Initialize()
{
	WIBase::Initialize();
	auto *pOutline = WGUI::GetInstance().Create<WIRect>(this);
	m_hOutline = pOutline->GetHandle();
	pOutline->SetColor(Color::White);
	pOutline->SetHeight(1);
	pOutline->SetAutoAlignToParent(true, false);
}

void WILineGraph::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	if(m_hOutline.IsValid())
		m_hOutline->SetY(y - 1);
}

void WILineGraph::DoUpdate()
{
	WIBase::DoUpdate();
	UpdateGraph();
}

void WILineGraph::UpdateLines()
{
	auto numLines = m_lines.size();
	auto numLinesNew = m_values.size();
	m_lines.reserve(numLinesNew);
	for(auto i = numLines; i < numLinesNew; ++i) {
		auto *pLine = WGUI::GetInstance().Create<WILine>(this);
		pLine->SetLineWidth(2);
		m_lines.push_back(pLine->GetHandle());
	}
}

void WILineGraph::SetValue(uint32_t idx, float val)
{
	if(idx >= GetSegmentCount())
		SetSegmentCount(idx + 1);
	m_values[idx] = val;
}

void WILineGraph::SetSegmentCount(uint32_t count)
{
	auto numValues = m_values.size();
	if(count == numValues)
		return;
	if(count > numValues) {
		m_values.resize(count, 0.f);
		UpdateLines();
		return;
	}
	for(auto i = count; i < m_values.size(); ++i) {
		auto &hLine = m_lines[i];
		if(hLine.IsValid())
			hLine->Remove();
	}
	m_values.resize(count);
	m_lines.resize(count);
}

uint32_t WILineGraph::GetSegmentCount() const { return static_cast<uint32_t>(m_values.size()); }

void WILineGraph::UpdateGraph()
{
	auto &values = m_values;
	if(values.empty())
		return;
	auto numValues = values.size();
	auto w = GetWidth();
	auto h = GetHeight();
	auto &colA = Color::Lime;
	auto &colB = Color::Red;
	auto inc = w / static_cast<float>(numValues);
	Vector2i prevEndPos {0.f, h - h * values[0]};
	auto colPrev = colA.Lerp(colB, values[0]);
	for(auto i = decltype(numValues) {1}; i < numValues; ++i) {
		auto v = values[i];
		auto xOffset = inc * i;
		auto endPos = Vector2(xOffset + inc, h - h * v);
		auto endColor = colA.Lerp(colB, v);

		auto &hLine = m_lines[i];
		if(hLine.IsValid()) {
			auto *pLine = static_cast<WILine *>(hLine.get());
			pLine->SetStartColor(colPrev);
			pLine->SetEndColor(endColor);
			pLine->SetStartPos(prevEndPos);
			pLine->SetEndPos(endPos);
			pLine->SizeToContents();
			pLine->Update();
		}

		prevEndPos = endPos;
		colPrev = endColor;
	}
}
