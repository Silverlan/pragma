#include "stdafx_shared.h"
#include "pragma/lua/libraries/ltimer.h"

TimerHandle::TimerHandle(Timer *timer)
{
	m_timer = timer;
}
TimerHandle::TimerHandle() {}
void TimerHandle::Invalidate() {m_timer = nullptr;}
TimerHandle::~TimerHandle()
{
	if(m_timer != nullptr)
		m_timer->InvalidateHandle(this);
}
bool TimerHandle::IsValid() const {return m_timer != nullptr;}
Timer *TimerHandle::GetTimer() const {return m_timer;}
