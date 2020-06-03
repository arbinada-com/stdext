#pragma once
/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Testing utilities
 */
#include <string>
#include <random>
#include <ctime>
#include <chrono>
#include <cstdint>
#include <iostream>
#include "platforms.h"

#if defined(__STDEXT_WINDOWS)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
    #include <sstream>
#endif


namespace stdext
{
    namespace testutils
    {
        class timer
        {
        public:
            timer(std::string title);
        public:
            void start(std::string title);
            void stop();
            std::string to_string();
            int64_t duration_mcs() const noexcept { return m_duration_mcs; }
            int64_t total_duration_mcs() const noexcept { return m_total_duration_mcs; }
        private:
            std::chrono::system_clock::time_point m_t1;
            std::chrono::system_clock::time_point m_t2;
            std::string m_title;
            std::string m_point_title;
            int64_t m_duration_mcs = 0;
            int64_t m_total_duration_mcs = 0;
        };

        class rnd_helper
        {
        public:
            rnd_helper();
        public:
            class char_range
            {
            public:
                char_range() {};
                char_range(const wchar_t min, const wchar_t max)
                    : m_min(min), m_max(max)
                { }
                char_range(const char_range&) = default;
                char_range& operator=(const char_range&) = default;
                char_range(char_range&&) = default;
                char_range& operator=(char_range&&) = default;
            public:
                bool contains(const wchar_t c) const noexcept { return c >= m_min && c <= m_max; }
                wchar_t min() const noexcept { return m_min; }
                void min(const wchar_t c) noexcept { m_min = c; }
                wchar_t max() const noexcept { return m_max; }
                void max(const wchar_t c) noexcept { m_max = c; }
            private:
                wchar_t m_min = 0x0001;
                wchar_t m_max = 0xFFFF;
            };
        public:
            int random_range(int from, int to); // [from, to]
            double random_float(); // [0, 1)
            bool random_bool() { return random_range(0, 1) == 0; }
            std::wstring random_wstring(const int avg_length);
            std::wstring random_wstring(const int avg_length, const char_range& range);
            std::wstring random_wstring(const int min_length, const int max_length);
            std::wstring random_wstring(const int min_length, const int max_length, const char_range& range);
        private:
            std::random_device m_device;
            std::default_random_engine m_engine;
        };

        class memchecker
        {
        public:
#if defined(__STDEXT_WINDOWS)
            typedef _CrtMemState memstate_t;
#else
            typedef struct
            {
                // to do
            } memstate_t;
#endif
        public:
            memchecker(bool check_on_destroy = false);
            ~memchecker();
        public:
            void checkpoint() noexcept;
            void check() noexcept(false);
            bool has_leaks() const noexcept;
            std::string report() const noexcept;
            std::wstring wreport() const noexcept;
        private:
            bool m_check_on_destroy;
#if defined(__STDEXT_WINDOWS)
            memstate_t m_first_state;
            memstate_t m_curr_state;
            memstate_t m_curr_diff;
#endif
        };

    }
}


