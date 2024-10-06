/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wiprogressbar.h"
#include <wgui/types/witext.h>
#include <wgui/types/wirect.h>
#include <sharedutils/util.h>
#include <mathutil/umath.h>
#include <sharedutils/property/util_property_color.hpp>

import pragma.string.unicode;

LINK_WGUI_TO_CLASS(WIProgressBar, WIProgressBar);

WIProgressBar::WIProgressBar() : WIBase(), m_progress(std::make_shared<util::FloatProperty>(0.f)), m_min(0.f), m_max(100.f), m_stepSize(1.f), m_numDecimals(0)
{
	RegisterCallback<void, float, float>("OnChange");
	RegisterCallback<bool, float, std::reference_wrapper<std::string>>("TranslateValue");
}

WIProgressBar::~WIProgressBar() {}

void WIProgressBar::Initialize()
{
	WIBase::Initialize();
	SetSize(128, 32);
	m_hLabel = CreateChild<WIText>();
	m_hLabel->SetAutoCenterToParent(true);
	m_hLabel->AddStyleClass("progressbar_label_background");
	m_hProgress = CreateChild<WIRect>();
	m_hProgress->GetColorProperty()->Link(*GetColorProperty());

	m_hLabel2 = WGUI::GetInstance().Create<WIText>(m_hProgress.get())->GetHandle();
	m_hLabel2->AddStyleClass("progressbar_label_overlay");
	UpdateTextPosition();
}

void WIProgressBar::SetLabelVisible(bool visible)
{
	if(m_hLabel.IsValid())
		m_hLabel->SetVisible(visible);
	if(m_hLabel2.IsValid())
		m_hLabel2->SetVisible(visible);
}

void WIProgressBar::UpdateTextPosition()
{
	if(m_hLabel.IsValid() == false)
		return;
	auto x = GetWidth() / 2 - m_hLabel->GetWidth() / 2;
	auto y = GetHeight() / 2 - m_hLabel->GetHeight() / 2;
	m_hLabel->SetPos(x, y);
	if(m_hLabel2.IsValid())
		m_hLabel2->SetPos(x, y);
}

void WIProgressBar::SetValueTranslator(const std::function<std::string(float)> &translator) { m_valueTranslator = translator; }

void WIProgressBar::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	if(m_hProgress.IsValid()) {
		auto *pProgress = m_hProgress.get<WIRect>();
		pProgress->SetHeight(y);
	}
	OnProgressChanged(GetValue(), GetValue());
	UpdateTextPosition();
}

float WIProgressBar::GetProgress() const { return m_progress->GetValue(); }
const util::PFloatProperty &WIProgressBar::GetProgressProperty() const { return m_progress; }
void WIProgressBar::OnProgressChanged(float oldValue, float value)
{
	auto w = GetWidth();
	if(m_hProgress.IsValid()) {
		auto *pProgress = m_hProgress.get<WIRect>();
		pProgress->SetWidth(CInt32(CFloat(w) * GetProgress()));
	}
	UpdateText();
	CallCallbacks<void, float, float>("OnChange", oldValue, value);
}
void WIProgressBar::SetProgress(float progress)
{
	progress = UpdateProgress(progress);
	if(progress == *m_progress)
		return;
	auto value = GetValue();
	*m_progress = progress;
	OnProgressChanged(value, GetValue());
}
void WIProgressBar::SetValue(float v)
{
	auto dt = m_max - m_min;
	SetProgress((dt != 0.f) ? ((v - m_min) / dt) : 0.f);
}
void WIProgressBar::SetRange(float min, float max, float stepSize, bool bEraseOptions)
{
	m_min = min;
	m_max = max;
	m_stepSize = stepSize;
	m_numDecimals = (m_stepSize > 0.f) ? umath::get_number_of_decimals(m_stepSize) : 2;
	if(bEraseOptions == true)
		m_options.clear();
}
void WIProgressBar::SetRange(float min, float max, float stepSize) { SetRange(min, max, stepSize, true); }
std::array<float, 3> WIProgressBar::GetRange() const { return std::array<float, 3> {m_min, m_max, m_stepSize}; }
float WIProgressBar::UpdateProgress(float progress)
{
	if(progress < 0.f)
		progress = 0.f;
	else if(progress > 1.f)
		progress = 1.f;
	if(m_stepSize <= 0.f)
		return progress;
	auto dt = m_max - m_min;
	float v = m_min + progress * dt;
	v -= std::remainderf(v, m_stepSize);
	return (dt != 0.f) ? ((v - m_min) / dt) : 0.f;
}
void WIProgressBar::UpdateOptions()
{
	auto max = m_options.size();
	if(max > 0)
		max--;
	SetRange(0.f, CFloat(max), 1.f, false);
}
void WIProgressBar::UpdateText()
{
	if(!m_hLabel.IsValid())
		return;
	auto rawValue = GetValue();
	UInt32 v = CUInt32(rawValue);
	std::string val;
	if(v >= m_options.size()) {
		if(m_valueTranslator != nullptr)
			val = m_valueTranslator(rawValue);
		else {
			if(CallCallbacks<bool, float, std::reference_wrapper<std::string>>("TranslateValue", rawValue, std::ref(val)) == false)
				GetValue(val);
		}
		val += m_postfix;
	}
	else
		val = m_options[v];
	auto *pLabel = m_hLabel.get<WIText>();
	pLabel->SetText(val);
	pLabel->SizeToContents();
	if(!m_hLabel2.IsValid())
		return;
	auto *pLabel2 = m_hLabel2.get<WIText>();
	pLabel2->SetText(val);
	pLabel2->SizeToContents();
	pLabel2->SetPos(pLabel->GetPos());
}
void WIProgressBar::SetOptions(const std::vector<std::string> &options)
{
	m_options = options;
	UpdateOptions();
}
void WIProgressBar::AddOption(const std::string &option)
{
	m_options.push_back(option);
	UpdateOptions();
}
float WIProgressBar::GetValue() const
{
	auto dt = m_max - m_min;
	float val = (dt != 0.f) ? (m_min + *m_progress * dt) : 0.f;
	return CFloat(umath::round(CDouble(val), m_numDecimals));
}
void WIProgressBar::GetValue(std::string &str) { str = util::round_string(GetValue(), m_numDecimals); }
void WIProgressBar::SetPostFix(const std::string &postfix) { m_postfix = postfix; }
