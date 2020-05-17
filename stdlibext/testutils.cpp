/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "testutils.h"
#include <cmath>

using namespace std;
using namespace stdext;
using namespace testutils;

/*
 * timer class
 */
timer::timer(std::string title)
    : m_title(title)
{}

void timer::start(std::string title)
{
    m_point_title = title;
    m_t1 = std::chrono::system_clock::now();
}

void timer::stop()
{
    m_t2 = std::chrono::system_clock::now();
    m_duration_mcs = std::chrono::duration_cast<std::chrono::microseconds>(m_t2 - m_t1).count();
    m_total_duration_mcs += m_duration_mcs;
}

std::string timer::to_string()
{
    return m_title + "." + m_point_title +
        ". Duration, mcs: " + std::to_string(m_duration_mcs) +
        ", total: " + std::to_string(m_total_duration_mcs);
}


/*
 * rnd_helper class 
 */
rnd_helper::rnd_helper()
{
    std::default_random_engine rnd_engine(m_device());
    m_engine = std::move(rnd_engine);
}

int rnd_helper::random_range(int from, int to) // [from, to]
{
    std::uniform_int_distribution<int> dist(from, to);
    return dist(m_engine);
}

double rnd_helper::random_float() // [0, 1)
{
    std::mt19937 gen(m_device());
    std::uniform_real_distribution<double> dist(0, 1);
    return dist(gen);
}

std::wstring rnd_helper::random_wstring(const int avg_length)
{
    return random_wstring(avg_length, char_range());
}

std::wstring rnd_helper::random_wstring(const int avg_length, const char_range& range)
{
    int length = abs(avg_length);
    int deviation = random_range(0, length);
    return random_wstring(length - deviation, length + deviation, range);
}

std::wstring rnd_helper::random_wstring(const int min_length, const int max_length)
{
    return random_wstring(min_length, max_length, char_range());
}

std::wstring rnd_helper::random_wstring(const int min_length, const int max_length, const char_range& range)
{
    const int min_limit = 1;
    constexpr int max_limit = 1024 * 1024 * 1024;
    int length = random_range(
        (min_length < min_limit ? min_limit : (min_length > max_limit ? max_limit : min_length)),
        (max_length < min_limit ? min_limit : (max_length > max_limit ? max_limit : max_length))
    );
    wstring s;
    s.resize(length);
    for (int i = 0; i < length; i++)
    {
        s[i] = static_cast<wchar_t>(random_range(range.min(), range.max()));
    }
    return s;
}

