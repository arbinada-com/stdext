#pragma once
#include "CppUnitTest.h"
#include "parsers.h"
#include "jsonlexer.h"
#include "jsonparser.h"

template<> inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<stdext::json::token>(const stdext::json::token& t) { return stdext::json::to_wstring(t); }
template<> inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<stdext::parsers::textpos>(const stdext::parsers::textpos& t) { return t.to_wstring(); }


