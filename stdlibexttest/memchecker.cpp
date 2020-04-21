#include "memchecker.h"
#include <iostream>
#include <sstream>
#include <exception>

using namespace std;

memchecker::memchecker(bool check_on_destroy)
    : m_check_on_destroy(check_on_destroy)
{ 
    _CrtMemCheckpoint(&m_first_state);
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

_CrtMemState memchecker::checkpoint() noexcept
{
    _CrtMemCheckpoint(&m_curr_state);
    _CrtMemDifference(&m_curr_diff, &m_first_state, &m_curr_state);
    return m_curr_diff;
}

void memchecker::check() noexcept(false)
{
    checkpoint();
    if (has_leaks())
    {
        string msg = report();
        throw exception(msg.c_str());
    }
}

bool memchecker::has_leaks() const noexcept
{
    return 
        m_curr_diff.lCounts[_FREE_BLOCK] > 0 ||
        m_curr_diff.lCounts[_NORMAL_BLOCK] > 0 ||
        m_curr_diff.lCounts[_CRT_BLOCK] > 0 ||
        m_curr_diff.lCounts[_IGNORE_BLOCK] > 0 ||
        m_curr_diff.lCounts[_CLIENT_BLOCK] > 0;
}

std::string memchecker::report() const noexcept
{
    stringstream ss;
    ss << "Memory difference report" << endl
        << "Blocks difference:" << endl;
    ss << "\t_FREE_BLOCK: " << m_curr_diff.lSizes[_FREE_BLOCK] << "bytes in " << m_curr_diff.lCounts[_FREE_BLOCK] << " blocks" << endl;
    ss << "\t_NORMAL_BLOCK: " << m_curr_diff.lSizes[_NORMAL_BLOCK] << "bytes in " << m_curr_diff.lCounts[_NORMAL_BLOCK] << " blocks" << endl;
    ss << "\t_CRT_BLOCK: " << m_curr_diff.lSizes[_CRT_BLOCK] << "bytes in " << m_curr_diff.lCounts[_CRT_BLOCK] << " blocks" << endl;
    ss << "\t_IGNORE_BLOCK: " << m_curr_diff.lSizes[_IGNORE_BLOCK] << "bytes in " << m_curr_diff.lCounts[_IGNORE_BLOCK] << " blocks" << endl;
    ss << "\t_CLIENT_BLOCK: " << m_curr_diff.lSizes[_CLIENT_BLOCK] << "bytes in " << m_curr_diff.lCounts[_CLIENT_BLOCK] << " blocks" << endl;
    ss << "Largest number used: " << m_curr_diff.lHighWaterCount << " bytes" << endl;
    ss << "Total allocations: " << m_curr_diff.lTotalCount << " bytes" << endl;
    return ss.str();
}

std::wstring memchecker::wreport() const noexcept
{
    std::string s = report();
    return std::wstring(s.begin(), s.end());
}
