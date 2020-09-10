/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "testutils.h"
#include <cmath>
#include <sstream>

using namespace std;

namespace stdext
{
namespace testutils
{

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
    return random_wstring(avg_length, locutils::wchar_range());
}

std::wstring rnd_helper::random_wstring(const int avg_length, const locutils::wchar_range& range)
{
    int length = abs(avg_length);
    int deviation = random_range(0, length);
    return random_wstring(length - deviation, length + deviation, range);
}

std::wstring rnd_helper::random_wstring(const int min_length, const int max_length)
{
    return random_wstring(min_length, max_length, locutils::wchar_range());
}

std::wstring rnd_helper::random_wstring(const int min_length, const int max_length, const locutils::wchar_range& range)
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
        wchar_t c;
        do
        {
            c = static_cast<wchar_t>(random_range(range.min(), range.max()));
        } while (locutils::utf16::is_noncharacter(c));
        s[i] = c;
    }
    return s;
}


/*
 * memchecker class
 */
memchecker::memchecker(bool check_on_destroy)
    : m_check_on_destroy(check_on_destroy)
{
#if defined(_DEBUG)
    #if defined(__STDEXT_WINDOWS)
    _CrtMemCheckpoint(&m_first_state);
    #endif
#endif
    checkpoint();
}

memchecker::~memchecker()
{
    if (m_check_on_destroy)
    {
        checkpoint();
        check();
    }
}

void memchecker::checkpoint() noexcept
{
#if defined(_DEBUG)
    #if defined(__STDEXT_WINDOWS)
    _CrtMemCheckpoint(&m_curr_state);
    _CrtMemDifference(&m_curr_diff, &m_first_state, &m_curr_state);
    #endif
#endif
}

void memchecker::check() noexcept(false)
{
    checkpoint();
    if (has_leaks())
    {
        string msg = report();
        throw std::logic_error(msg.c_str());
    }
}

bool memchecker::has_leaks() const noexcept
{
#if defined(_DEBUG)
    #if defined(__STDEXT_WINDOWS)
    return
        m_curr_diff.lCounts[_FREE_BLOCK] > 0 ||
        m_curr_diff.lCounts[_NORMAL_BLOCK] > 0 ||
        m_curr_diff.lCounts[_CRT_BLOCK] > 0 ||
        m_curr_diff.lCounts[_IGNORE_BLOCK] > 0 ||
        m_curr_diff.lCounts[_CLIENT_BLOCK] > 0;
    #else
    return false;
    #endif
#else
    return false;
#endif
}

std::string memchecker::report() const noexcept
{
    stringstream ss;
    ss << "Memory difference report" << endl
        << "Blocks difference:" << endl;
#if defined(__STDEXT_WINDOWS)
    ss << "\t_FREE_BLOCK: " << m_curr_diff.lSizes[_FREE_BLOCK] << "bytes in " << m_curr_diff.lCounts[_FREE_BLOCK] << " blocks" << endl;
    ss << "\t_NORMAL_BLOCK: " << m_curr_diff.lSizes[_NORMAL_BLOCK] << "bytes in " << m_curr_diff.lCounts[_NORMAL_BLOCK] << " blocks" << endl;
    ss << "\t_CRT_BLOCK: " << m_curr_diff.lSizes[_CRT_BLOCK] << "bytes in " << m_curr_diff.lCounts[_CRT_BLOCK] << " blocks" << endl;
    ss << "\t_IGNORE_BLOCK: " << m_curr_diff.lSizes[_IGNORE_BLOCK] << "bytes in " << m_curr_diff.lCounts[_IGNORE_BLOCK] << " blocks" << endl;
    ss << "\t_CLIENT_BLOCK: " << m_curr_diff.lSizes[_CLIENT_BLOCK] << "bytes in " << m_curr_diff.lCounts[_CLIENT_BLOCK] << " blocks" << endl;
    ss << "Largest number used: " << m_curr_diff.lHighWaterCount << " bytes" << endl;
    ss << "Total allocations: " << m_curr_diff.lTotalCount << " bytes" << endl;
#endif
    return ss.str();
}

std::wstring memchecker::wreport() const noexcept
{
    std::string s = report();
    return std::wstring(s.begin(), s.end());
}

}
}
