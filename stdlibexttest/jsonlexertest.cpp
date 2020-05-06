#include "CppUnitTest.h"
#include "json.h"
#include <fstream>
#include <limits>
#include "strutils.h"
#include "memchecker.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;
using namespace parsers;

template<> inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<json::token>(const json::token& t) { return json::to_wstring(t); }
template<> inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<parsers::textpos>(const parsers::textpos& t) { return t.to_wstring(); }

TEST_CLASS(JsonLexerTest)
{
public:
    TEST_METHOD(TestEmptyStreams)
    {
        wstringstream ss(L"");
        ioutils::text_reader r(ss);
        json::msg_collector_t mc;
        json::lexer lexer(&r, &mc);
        json::lexeme l;
        Assert::IsFalse(lexer.next_lexeme(l));
        Assert::IsFalse(lexer.has_errors());
    }

    void CompareLexemes(const json::lexeme& expected, const json::lexeme& lex, wstring title)
    {
        title += L": ";
        Assert::AreEqual<json::token>(expected.token(), lex.token(), (title + L"token").c_str());
        Assert::AreEqual<parsers::textpos>(expected.pos(), lex.pos(), (title + L"pos").c_str());
        Assert::AreEqual<wstring>(expected.text(), lex.text(), (title + L"text").c_str());
    }

    void CheckLexeme(const wstring input, const json::lexeme& expected, const wstring title)
    {
        wstringstream ss(input);
        ioutils::text_reader r(ss);
        json::msg_collector_t mc;
        json::lexer lexer(&r, &mc);
        json::lexeme lex;
        bool next_ok = lexer.next_lexeme(lex);
        if (lexer.has_errors())
        {
            wstring msg = L"next_lexeme() failed:";
            for (json::message_t* err : lexer.messages()->errors())
            {
                msg += L"\n" + err->to_wstring();
            }
            Assert::Fail(msg.c_str());
        }
        wstring title2 = title + L": ";
        Assert::IsTrue(next_ok, (title2 + L"next_lexeme() failed with no errors").c_str());
        Assert::IsFalse(lexer.has_errors(), (title2 + L"has errors").c_str());
        CompareLexemes(expected, lex, title);
    }

    TEST_METHOD(TestSimpleTokens)
    {
        CheckLexeme(L"[", json::lexeme(json::token::begin_array, textpos(1, 1), L"["), L"Begin array 1.1");
        CheckLexeme(L" \t[", json::lexeme(json::token::begin_array, textpos(1, 3), L"["), L"Begin array 1.2");
        CheckLexeme(L"\r\n[", json::lexeme(json::token::begin_array, textpos(2, 1), L"["), L"Begin array 1.3");
        CheckLexeme(L" \t\r\n[", json::lexeme(json::token::begin_array, textpos(2, 1), L"["), L"Begin array 1.4");
        CheckLexeme(L" \t\r\n \t[", json::lexeme(json::token::begin_array, textpos(2, 3), L"["), L"Begin array 1.5");
        CheckLexeme(L"\r\n\r\n\t[", json::lexeme(json::token::begin_array, textpos(3, 2), L"["), L"Begin array 1.6");
        CheckLexeme(L"{", json::lexeme(json::token::begin_object, textpos(1, 1), L"{"), L"Begin object 1.1");
        CheckLexeme(L"]", json::lexeme(json::token::end_array, textpos(1, 1), L"]"), L"End array 1.1");
        CheckLexeme(L"}", json::lexeme(json::token::end_object, textpos(1, 1), L"}"), L"End object 1.1");
        CheckLexeme(L"false", json::lexeme(json::token::literal_false, textpos(1, 1), L"false"), L"Literal 1.1");
        CheckLexeme(L"null", json::lexeme(json::token::literal_null, textpos(1, 1), L"null"), L"Literal 2.1");
        CheckLexeme(L"true", json::lexeme(json::token::literal_true, textpos(1, 1), L"true"), L"Literal 3.1");
        CheckLexeme(L":", json::lexeme(json::token::name_separator, textpos(1, 1), L":"), L"Name separator 1.1");
        CheckLexeme(L",", json::lexeme(json::token::value_separator, textpos(1, 1), L","), L"Value separator 1.1");
    }

    TEST_METHOD(TestStrings)
    {
        CheckLexeme(L"\"Hello world!\"", json::lexeme(json::token::string, textpos(1, 1), L"Hello world!"), L"String 1");
        CheckLexeme(L"\"\\\"\"", json::lexeme(json::token::string, textpos(1, 1), L"\""), L"String 2.1");
        CheckLexeme(L"\"\\\\\"", json::lexeme(json::token::string, textpos(1, 1), L"\\"), L"String 2.2");
        CheckLexeme(L"\"\\/\"", json::lexeme(json::token::string, textpos(1, 1), L"/"), L"String 2.3");
        CheckLexeme(L"\"\\b\"", json::lexeme(json::token::string, textpos(1, 1), L"\b"), L"String 2.4");
        CheckLexeme(L"\"\\f\"", json::lexeme(json::token::string, textpos(1, 1), L"\f"), L"String 2.5");
        CheckLexeme(L"\"\\n\"", json::lexeme(json::token::string, textpos(1, 1), L"\n"), L"String 2.6");
        CheckLexeme(L"\"\\r\"", json::lexeme(json::token::string, textpos(1, 1), L"\r"), L"String 2.7");
        CheckLexeme(L"\"\\t\"", json::lexeme(json::token::string, textpos(1, 1), L"\t"), L"String 2.8");
        CheckLexeme(L"\"-\\\"-\\\\-\\/-\\b-\\f-\\n-\\r-\\t-\"", json::lexeme(json::token::string, textpos(1, 1), L"-\"-\\-/-\b-\f-\n-\r-\t-"), L"String 2.All");
        CheckLexeme(L"\"\\u0022\"", json::lexeme(json::token::string, textpos(1, 1), L"\x0022"), L"String 3.1");
        CheckLexeme(L"\"Abc\\u1234Def\"", json::lexeme(json::token::string, textpos(1, 1), L"Abc\x1234""Def"), L"String 3.2");
        CheckLexeme(L"\"Строка déjà\"", json::lexeme(json::token::string, textpos(1, 1), L"Строка déjà"), L"String 3.3");
    }

    void CheckError(const wstring input, const json::parser_msg_kind kind, const textpos& pos, const wstring title)
    {
        wstringstream ss(input);
        ioutils::text_reader r(ss, L"ChkErrStream");
        json::msg_collector_t mc;
        json::lexer lexer(&r, &mc);
        json::lexeme lex;
        while (!lexer.eof() && lexer.next_lexeme(lex))
        {
        }
        wstring title2 = title + L": ";
        Assert::IsTrue(lexer.has_errors(), (title2 + L"no errors").c_str());
        json::message_t* err = lexer.messages()->errors()[0];
        Assert::AreEqual((int)kind, (int)err->kind(), (title2 + L"kind").c_str());
        Assert::IsFalse(err->text().empty(), (title2 + L"text is empty").c_str());
        Assert::AreEqual<parsers::textpos>(pos, lex.pos(), (title + L"pos").c_str());
        Assert::AreEqual(r.source_name(), err->source(), (title2 + L"source").c_str());
    }

    TEST_METHOD(TestLexerErrors)
    {
        CheckError(L"try", json::parser_msg_kind::err_invalid_literal_fmt, textpos(1, 2), L"E1010.1");
        CheckError(L"true\ntrue2", json::parser_msg_kind::err_invalid_literal_fmt, textpos(2, 1), L"E1010.2");
        CheckError(L"null\nnulltrue", json::parser_msg_kind::err_invalid_literal_fmt, textpos(2, 1), L"E1010.3");
        CheckError(L"false\nfalsetrue", json::parser_msg_kind::err_invalid_literal_fmt, textpos(2, 1), L"E1010.4");
        //
        CheckError(L"\"\\u123H\"", json::parser_msg_kind::err_unallowed_char_fmt, textpos(1, 6), L"E1030.1");
        CheckError(L"\"\\uABCD\\u123H\"", json::parser_msg_kind::err_unallowed_char_fmt, textpos(1, 6), L"E1030.2");
        //
        CheckError(L"\t\b", json::parser_msg_kind::err_unexpected_char_fmt, textpos(1, 2), L"E1060.1");
    }

};