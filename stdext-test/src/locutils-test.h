#pragma once
#include <string>

namespace locutils_test
{

// 01: "ABC déjà строка"
const std::wstring string_01_utf16 = L"ABC d\u00E9j\u00E0 \u0441\u0442\u0440\u043E\u043A\u0430";
                                    // ABC-d-é----j-à------с-----т-----р-----о-----к-----а
const std::string  string_01_utf8  =  "ABC d\xC3\xA9j\xC3\xA0 \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0";
                                    // ABC-d-é------j-à--------с-------т-------р-------о-------к-------а
const std::wstring string_01_utf16_cp1251 = L"ABC \u0441\u0442\u0440\u043E\u043A\u0430";
const std::string  string_01_utf8_cp1251  =  "ABC \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0";
const std::string  string_01_ansi_cp1251  =  "ABC \xF1\xF2\xF0\xEE\xEA\xE0";
const std::wstring string_01_utf16_cp1252 = L"ABC d\u00E9j\u00E0";
const std::string  string_01_utf8_cp1252  =  "ABC d\xC3\xA9j\xC3\xA0";
const std::string  string_01_ansi_cp1252  =  "ABC d\xE9j\xE0";

const std::string  string_02_ansi_cp1251  =  "\xC0\xC1\xC2\xDE\xDF\xE0\xE1\xE2\xFE\xFF";
                                          //   А   Б   В   Ю   Я   а   б   в   ю   я
const std::wstring string_02_utf16_cp1251 = L"\u0410\u0411\u0412\u042E\u042F\u0430\u0431\u0432\u044E\u044F";
                                          //   А     Б     В     Ю     Я     а     б     в     ю     я
const std::string  string_02_ansi_cp1252  =  "\xC0\xC7\xC8\xC9\xCF\xD9\xE0\xE7\xE8\xE9\xEF\xF9";
                                          //   À   Ç   È   É   Ï   Ù   à   ç   è   é   ï   ù
const std::wstring string_02_utf16_cp1252 = L"\u00C0\u00C7\u00C8\u00C9\u00CF\u00D9\u00E0\u00E7\u00E8\u00E9\u00EF\u00F9";
                                           //  À     Ç     È     É     Ï     Ù     à     ç     è     é     ï     ù
}
