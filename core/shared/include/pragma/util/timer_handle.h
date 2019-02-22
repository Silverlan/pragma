#ifndef __TIMER_HANDLE_H__
#define __TIMER_HANDLE_H__

#include "pragma/networkdefinitions.h"

class Timer;
class DLLNETWORK TimerHandle
{
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
