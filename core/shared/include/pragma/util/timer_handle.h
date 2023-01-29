/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __TIMER_HANDLE_H__
#define __TIMER_HANDLE_H__

#include "pragma/networkdefinitions.h"

class Timer;
class DLLNETWORK TimerHandle {
  public:
	friend Timer;
  protected:
	TimerHandle(Timer *timer);
  protected:
	Timer *m_timer;
	void Invalidate();
  public:
	TimerHandle();
	~TimerHandle();
	bool IsValid() const;
	Timer *GetTimer() const;
};

#endif
