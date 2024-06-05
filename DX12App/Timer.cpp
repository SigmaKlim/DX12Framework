#include "Timer.h"
#include <Windows.h>
#include <cassert>

Timer::Timer()
{
	LARGE_INTEGER countsPerSec;
	QueryPerformanceFrequency(&countsPerSec);
	_secPerCount = 1.0 / countsPerSec.QuadPart;
	_prevCounts = GetCountsElapsed();
}

void Timer::Tick()
{
	auto counts = GetCountsElapsed();
	auto deltaCounts = counts - _prevCounts;
	assert(deltaCounts > 0);
	_deltaSec = (float)(deltaCounts * _secPerCount);
	_prevCounts = counts;
}

float Timer::GetDelta() const
{
	return _deltaSec;
}

INT64 Timer::GetCountsElapsed() const
{
	LARGE_INTEGER countsElapsed;
	QueryPerformanceCounter(&countsElapsed);
	return countsElapsed.QuadPart;
}
