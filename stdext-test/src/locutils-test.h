#pragma once
#include <string>

namespace locale_utils_test
{

// 01: "ABC déjà строка"
const std::wstring string_01_utf16 = L"ABC d\u00E9j\u00E0 \u0441\u0442\u0440\u043E\u043A\u0430";
const std::string  string_01_utf8  =  "ABC d\xC3\xA9j\xC3\xA0 \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0";
                                    //       é--------à--------с-------т-------р-------о-------к-------а
const std::wstring string_01_utf16_cp1251 = L"ABC \u0441\u0442\u0440\u043E\u043A\u0430";
const std::string  string_01_utf8_cp1251  =  "ABC \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0";
const std::string  string_01_ansi_cp1251  =  "ABC \xF1\xF2\xF0\xEE\xEA\xE0";
const std::wstring string_01_utf16_cp1252 = L"ABC d\u00E9j\u00E0";
const std::string  string_01_utf8_cp1252  =  "ABC d\xC3\xA9j\xC3\xA0";
const std::string  string_01_ansi_cp1252  =  "ABC d\xE9j\xE0";

}
