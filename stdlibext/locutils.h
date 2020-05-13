/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Locale manipulation utils
 */
#pragma once
#include <clocale>
#include <string.h>

namespace stdext
{
    namespace locutils
    {
        class locale_guard
        {
        public:
            locale_guard(int category, const char* new_locale_name)
                : m_category(category)
            {
                const char* s = std::setlocale(m_category, nullptr);
                rsize_t len = strlen(s);
                m_old_locale_name = new char[len + 1];
                strncpy_s(m_old_locale_name, len + 1, s, len);
                std::setlocale(m_category, new_locale_name);
            }
            ~locale_guard()
            {
                std::setlocale(m_category, m_old_locale_name);
                delete[] m_old_locale_name;
            }
        private:
            int m_category;
            char* m_old_locale_name;
        };

    }
}

