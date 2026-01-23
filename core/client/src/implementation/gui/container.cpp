// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.container;

pragma::gui::types::WIContainer::WIContainer() : WIBase(), m_padding {0, 0, 0, 0} {}

void pragma::gui::types::WIContainer::OnChildAdded(WIBase *child)
{
	WIBase::OnChildAdded(child);
	ScheduleUpdate();

	auto it = m_childCallbacks.find(child);
	if(it == m_childCallbacks.end())
		it = m_childCallbacks.insert(std::make_pair(child, std::array<CallbackHandle, 2> {})).first;
	auto &callbacks = it->second;
	for(auto i = decltype(callbacks.size()) {0u}; i < callbacks.size(); ++i) {
		auto &cb = callbacks.at(i);
		cb = child->AddCallback((i == 0) ? "SetPos" : "SetSize", FunctionCallback<void>::Create([this]() { ScheduleUpdate(); }));
	}
}
void pragma::gui::types::WIContainer::OnChildRemoved(WIBase *child)
{
	WIBase::OnChildRemoved(child);
	ScheduleUpdate();

	auto it = m_childCallbacks.find(child);
	if(it == m_childCallbacks.end())
		return;
	auto &callbacks = it->second;
	for(auto i = decltype(callbacks.size()) {0u}; i < callbacks.size(); ++i) {
		auto &cb = callbacks.at(i);
		if(cb.IsValid())
			cb.Remove();
	}
	m_childCallbacks.erase(it);
}
void pragma::gui::types::WIContainer::SetPadding(int32_t padding)
{
	for(auto &v : m_padding)
		v = padding;
}
void pragma::gui::types::WIContainer::SetPadding(int32_t top, int32_t right, int32_t bottom, int32_t left) { m_padding = {top, right, bottom, left}; }
void pragma::gui::types::WIContainer::SetPadding(Padding paddingType, int32_t padding)
{
	switch(paddingType) {
	case Padding::Top:
		SetPaddingTop(padding);
		break;
	case Padding::Right:
		SetPaddingRight(padding);
		break;
	case Padding::Bottom:
		SetPaddingBottom(padding);
		break;
	default:
		SetPaddingLeft(padding);
		break;
	}
}
void pragma::gui::types::WIContainer::SizeToContents(bool x, bool y)
{
	WIBase::SizeToContents(x, y);
	SetSize(GetWidth() + GetPaddingLeft() + GetPaddingRight(), GetHeight() + GetPaddingTop() + GetPaddingBottom());
}
void pragma::gui::types::WIContainer::DoUpdate()
{
	WIBase::DoUpdate();
	SizeToContents();
}
void pragma::gui::types::WIContainer::SetPaddingTop(int32_t top) { m_padding[0] = top; }
void pragma::gui::types::WIContainer::SetPaddingRight(int32_t right) { m_padding[1] = right; }
void pragma::gui::types::WIContainer::SetPaddingBottom(int32_t bottom) { m_padding[2] = bottom; }
void pragma::gui::types::WIContainer::SetPaddingLeft(int32_t left) { m_padding[3] = left; }
const std::array<int32_t, 4> &pragma::gui::types::WIContainer::GetPadding() const { return m_padding; }
int32_t pragma::gui::types::WIContainer::GetPaddingTop() const { return m_padding[0]; }
int32_t pragma::gui::types::WIContainer::GetPaddingRight() const { return m_padding[1]; }
int32_t pragma::gui::types::WIContainer::GetPaddingBottom() const { return m_padding[2]; }
int32_t pragma::gui::types::WIContainer::GetPaddingLeft() const { return m_padding[3]; }
int32_t pragma::gui::types::WIContainer::GetPadding(Padding paddingType) const
{
	switch(paddingType) {
	case Padding::Top:
		return GetPaddingTop();
	case Padding::Right:
		return GetPaddingRight();
	case Padding::Bottom:
		return GetPaddingBottom();
	default:
		return GetPaddingLeft();
	}
}
