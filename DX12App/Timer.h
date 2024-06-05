#pragma once
typedef signed long long INT64;

class Timer
{
public:
	Timer();
	void Tick();
	float GetDelta() const;
private:
	INT64 GetCountsElapsed() const;
	double	_secPerCount	= 0.0;
	INT64	_prevCounts		= 0;
	float	_deltaSec		= 0.0f;

};

