/*!
\brief Class for dealing with time in a cross-platform way.
\file PVRCore/Time_.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <cstdint>
// Required forward declarations
#if defined(__APPLE__)
struct mach_timebase_info;
#endif
#if !defined(__APPLE__)
#define PVR_PLATFORM_USES_TIMESTAMP
#endif
namespace pvr {
/// <summary>This class provides functions for measuring time: current time, elapsed time etc. High performance
/// timers are used if available by the platform.</summary>
class Time
{
public:
	Time();
	~Time();
#if defined(PVR_PLATFORM_USES_TIMESTAMP)
	/// <summary>Sets the current time as a the initial point to measure time from.</summary>
	void Reset();
#endif
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in nanoseconds.</returns>
	uint64_t getElapsedNanoSecs() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in microseconds.</returns>
	uint64_t getElapsedMicroSecs() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in microseconds.</returns>
	float getElapsedMicroSecsF() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in milliseconds.</returns>
	uint64_t getElapsedMilliSecs() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in milliseconds.</returns>
	float getElapsedMilliSecsF() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in seconds.</returns>
	uint64_t getElapsedSecs() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in seconds.</returns>
	float getElapsedSecsF() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in minutes.</returns>
	uint64_t getElapsedMins() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in seconds.</returns>
	float getElapsedMinsF() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in hours.</returns>
	uint64_t getElapsedHours() const;
	/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
	/// <returns>The elapsed time in seconds.</returns>
	float getElapsedHoursF() const;

private:
#if defined(PVR_PLATFORM_USES_TIMESTAMP)
	/// <summary>Provides the current time. Current time is abstract (not connected to the time of day) and only useful for
	/// comparison.</summary>
	/// <returns>The current time in nanoseconds.</returns>
	uint64_t getCurrentTimeStamp() const;

	uint64_t getTimerFrequencyHertz() const;
	uint64_t _startTime;
	uint64_t _timerFrequency;
#endif

#if defined(__APPLE__)
	struct mach_timebase_info* _timeBaseInfo;
#endif
};
} // namespace pvr
