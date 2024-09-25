/*!
\brief Implementations of methods of the Time class.
\file PVRCore/Time.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#elif defined(__APPLE__)
#include <mach/mach_time.h>

#elif defined(__QNX__)
#include <sys/time.h>

#else
#include <time.h>
#ifdef _POSIX_MONOTONIC_CLOCK
#define PVR_TIMER_CLOCK CLOCK_MONOTONIC
#else
#define PVR_TIMER_CLOCK CLOCK_REALTIME
#endif
#endif

#include "PVRCore/Time_.h"
#include <ctime>

namespace pvr {
Time::Time()
{
#if defined(_WIN32)
	QueryPerformanceFrequency((LARGE_INTEGER*)&_timerFrequency);
#elif defined(__APPLE__)
	_timeBaseInfo = new mach_timebase_info_data_t;
	mach_timebase_info(_timeBaseInfo);
	_timerFrequency = static_cast<uint64_t>((1.0e9 * static_cast<double>(_timeBaseInfo->numer)) / static_cast<double>(_timeBaseInfo->denom));
#elif !defined(__QNX__)
	timespec timerInfo;
	if (clock_getres(PVR_TIMER_CLOCK, &timerInfo) != 0) { _timerFrequency = static_cast<uint64_t>(timerInfo.tv_sec); }
#endif

#if defined(PVR_PLATFORM_USES_TIMESTAMP)
	// Reset the time so that we start afresh
	Reset();
#endif
}

Time::~Time()
{
#if defined(__APPLE__)
	delete _timeBaseInfo;
	_timeBaseInfo = 0;
#endif
}

#if defined(PVR_PLATFORM_USES_TIMESTAMP)
void Time::Reset() { _startTime = getCurrentTimeStamp(); }
#endif

uint64_t Time::getElapsedNanoSecs() const
{
	uint64_t currentTime{};
#if defined(_WIN32)
	currentTime = (1000000000 * (getCurrentTimeStamp() - _startTime)) / _timerFrequency;
#elif defined(__APPLE__)
	uint64_t time = mach_absolute_time();
	currentTime = static_cast<uint64_t>(time * (_timeBaseInfo->numer / _timeBaseInfo->denom));
#elif defined(__QNX__)
	timeval tv;
	gettimeofday(&tv, NULL);
	currentTime = static_cast<uint64_t>((tv.tv_sec * (unsigned long)1000) + (tv.tv_usec / 1000.0)) * 1000000;
#else
	currentTime = getCurrentTimeStamp() - _startTime;
#endif
	return currentTime;
}

uint64_t Time::getElapsedMicroSecs() const { return getElapsedNanoSecs() / 1000ull; }
float Time::getElapsedMicroSecsF() const { return float(getElapsedNanoSecs() / 1000.); }

uint64_t Time::getElapsedMilliSecs() const { return getElapsedNanoSecs() / 1000000ull; }
float Time::getElapsedMilliSecsF() const { return float(getElapsedNanoSecs() / 1000000.); }

uint64_t Time::getElapsedSecs() const { return getElapsedNanoSecs() / 1000000000ull; }
float Time::getElapsedSecsF() const { return float(getElapsedNanoSecs() / 1000000000.); }

uint64_t Time::getElapsedMins() const { return getElapsedNanoSecs() / (1000000000ull * 60ull); }
float Time::getElapsedMinsF() const { return float(getElapsedNanoSecs() / 1000000000.) / 60.f; }

uint64_t Time::getElapsedHours() const { return getElapsedNanoSecs() / (1000000000ull * 60ull * 60ull); }
float Time::getElapsedHoursF() const { return float(getElapsedNanoSecs() / 1000000000.) / (60.f * 60.f); }

#if defined(_WIN32)
static inline uint64_t helperQueryPerformanceCounter()
{
	uint64_t lastTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&lastTime);
	return lastTime;
}
#endif
#if defined(PVR_PLATFORM_USES_TIMESTAMP)
uint64_t Time::getCurrentTimeStamp() const
{
	uint64_t currentTime;
#if defined(_WIN32)
	currentTime = helperQueryPerformanceCounter();
#elif defined(__QNX__)
	timeval tv;
	gettimeofday(&tv, NULL);
	currentTime = static_cast<uint64_t>((tv.tv_sec * (unsigned long)1000) + (tv.tv_usec / 1000.0)) * 1000000;
#else
	timespec time;
	clock_gettime(PVR_TIMER_CLOCK, &time);
	currentTime = static_cast<uint64_t>(time.tv_nsec) +
		// convert seconds to ns and add
		1e9 * static_cast<uint64_t>(time.tv_sec);
#endif

	return currentTime;
}

uint64_t Time::getTimerFrequencyHertz() const { return _timerFrequency; }
#endif
} // namespace pvr
//!\endcond