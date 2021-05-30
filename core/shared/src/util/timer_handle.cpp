/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
