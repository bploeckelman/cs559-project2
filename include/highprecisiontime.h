#ifndef	HIGHPRECISIONTIME_H
#define	HIGHPRECISIONTIME_H

#ifdef	WIN32
#include <Windows.h>
#endif

class HighPrecisionTime
{
public:
	HighPrecisionTime();
	double TimeSinceLastCall();
	double TotalTime();

private:

#ifdef	WIN32
	LARGE_INTEGER initializationTicks;
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER previousTicks;
#endif

};

#endif