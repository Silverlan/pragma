// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.timer_handle;

export {
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
};
