#pragma once
#include <chrono>

class Timer
{
public:
	using Point = std::chrono::steady_clock::time_point;
	using Duration = std::chrono::duration<float>;

public:
	Timer()
		: m_begin_time(std::chrono::steady_clock::now())
		, m_last_time(std::chrono::steady_clock::now())
	{}
	Timer(const Timer&) = default;
	Timer& operator=(const Timer&) = default;

	float totalTime() const
	{
		return Duration(std::chrono::steady_clock::now() - m_begin_time).count();
	}

	float mark() const
	{
		if (m_is_pause)
		{
			return 0.0f;
		}
		Point curr_time = std::chrono::steady_clock::now();
		Duration delta_time = curr_time - m_last_time;
		m_last_time = curr_time;
		return delta_time.count();
	}

	float peek() const
	{
		return m_is_pause ? 0.0f : Duration(std::chrono::steady_clock::now() - m_last_time).count();
	}

	void start() const
	{
		m_is_pause = false;
		m_last_time = std::chrono::steady_clock::now();
	}

	void pause() const
	{
		m_is_pause = true;
	}

private:
	Point m_begin_time;
	mutable Point m_last_time;
	mutable bool m_is_pause = false;
};