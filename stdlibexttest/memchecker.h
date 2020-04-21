#pragma once
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <string>

class memchecker
{
public:
    memchecker(bool check_on_destroy = false);
    ~memchecker();
public:
    _CrtMemState checkpoint() noexcept;
    _CrtMemState curr_diff() const  noexcept { return m_curr_diff; }
    void check() noexcept(false);
    bool has_leaks() const noexcept;
    std::string report() const noexcept;
    std::wstring wreport() const noexcept;
private:
    bool m_check_on_destroy;
    _CrtMemState m_first_state;
    _CrtMemState m_curr_state;
    _CrtMemState m_curr_diff;
};
