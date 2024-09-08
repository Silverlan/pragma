#ifndef F_TIME_GLS
#define F_TIME_GLS

layout(std140, LAYOUT_ID(RENDER_SETTINGS, TIME)) uniform Time
{
	float curTime;
	float deltaTime;
	float realTime;
	float deltaRealTime;
}
u_time;

float cur_time() { return u_time.curTime; }
float delta_time() { return u_time.deltaTime; }
float real_time() { return u_time.realTime; }
float delta_real_time() { return u_time.deltaRealTime; }

#endif
