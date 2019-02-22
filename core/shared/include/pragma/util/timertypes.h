#ifndef __TIMERTYPES_H__
#define __TIMERTYPES_H__

enum class TimerType : uint8_t
{
	CurTime = 0,
	RealTime,
	ServerTime
};

#endif