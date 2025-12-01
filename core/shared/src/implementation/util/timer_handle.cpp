// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.timer_handle;

TimerHandle::TimerHandle(Timer *timer) { m_timer = timer; }
TimerHandle::TimerHandle() {}
void TimerHandle::Invalidate() { m_timer = nullptr; }
TimerHandle::~TimerHandle()
{
	if(m_timer != nullptr)
		m_timer->InvalidateHandle(this);
}
bool TimerHandle::IsValid() const { return m_timer != nullptr; }
Timer *TimerHandle::GetTimer() const { return m_timer; }
