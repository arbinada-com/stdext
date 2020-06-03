#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "json.h"
#include <fstream>
#include <limits>
#include "strutils.h"
#include "testutils.h"
#include "parsers-test.h"

using namespace std;
using namespace stdext;
using namespace parsers;
using namespace testutils;

namespace json_lexer_test
{

class lex_vector : public stdext::ptr_vector<json::lexeme>
{
public:
    void add(
        const json::token token, 
        const parsers::textpos::pos_t line, 
        const parsers::textpos::pos_t col, 
        const std::wstring text)
    {
        push_back(new json::lexeme(token, textpos(line, col), text));
    }
};


class JsonLexerTest : public testing::Test
{
protected:
    void CompareLexemes(const json::lexeme& expected, const json::lexeme& lex, wstring title)
    {
        title += L": ";
        ASSERT_EQ(expected.token(), lex.token()) << title + L"token";
        ASSERT_EQ(expected.pos(), lex.pos()) << title + L"pos";
        ASSERT_EQ(expected.text(), lex.text()) << title + L"text";
    }

    void CheckLexeme(const wstring input, const json::lexeme& expected, const wstring title)
    {
        wstring title2 = title + L": ";
        wstringstream ss(input);
        ioutils::text_reader r(ss);
        json::msg_collector_t mc;
        json::lexer lexer(r, mc);
        json::lexeme lex;
        bool next_ok = lexer.next_lexeme(lex);
        if (lexer.has_errors())
        {
            wstring msg = title2 + L"next_lexeme() failed:";
            for (json::message_t* err : lexer.messages().errors())
            {
                msg += L"\n" + err->to_wstring();
            }
            FAIL() << msg;
        }
        EXPECT_TRUE(next_ok) << title2 + L"next_lexeme() failed with no errors";
        ASSERT_FALSE(lexer.has_errors()) << title2 + L"has errors";
        CompareLexemes(expected, lex, title);
    }

    void CheckError(const wstring input, const json::parser_msg_kind kind, const textpos& pos, const wstring title)
    {
        wstringstream ss(input);
        ioutils::text_reader r(ss, L"ChkErrStream");
        json::msg_collector_t mc;
        json::lexer lexer(r, mc);
        json::lexeme lex;
        while (!lexer.eof() && lexer.next_lexeme(lex))
        {
        }
        wstring title2 = title + L": ";
        ASSERT_TRUE(lexer.has_errors()) << title2 + L"no errors";
        json::message_t* err = lexer.messages().errors()[0];
        ASSERT_TRUE(msg_origin::lexer == err->origin()) << title2 + L"origin. " + err->text();
        EXPECT_EQ(kind, err->kind()) << title2 + L"kind. " + err->text();
        EXPECT_FALSE(err->text().empty()) << title2 + L"text is empty";
        EXPECT_EQ(pos, err->pos()) << title2 + L"error pos. " + err->text();
        EXPECT_EQ(r.source_name(), err->source()) << title2 + L"source. " + err->text();
    }

    void CheckText(const wstring input, const lex_vector& expected, const wstring title)
    {
        wstringstream ss(input);
        ioutils::text_reader r(ss);
        json::msg_collector_t mc;
        json::lexer lexer(r, mc);
        json::lexeme lex;
        wstring title2 = title + L": ";
        std::size_t i = 0;
        while (lexer.next_lexeme(lex))
        {
            ASSERT_TRUE(i < expected.size()) << title2 + strutils::wformat(L"lex count %d exceeds expected one %d", i + 1, expected.size());
            json::lexeme* expected_lex = expected.at(i);
            CompareLexemes(*expected_lex, lex, strutils::wformat(L"%ls lexeme[%d]", title.c_str(), i));
            i++;
        }
        ASSERT_EQ(expected.size(), i) << title2 + L"lex count";
    }

};

TEST_F(JsonLexerTest, TestEmptyStreams)
{
    wstringstream ss(L"");
    ioutils::text_reader r(ss);
    json::msg_collector_t mc;
    json::lexer lexer(r, mc);
    json::lexeme l;
    ASSERT_FALSE(lexer.next_lexeme(l));
    ASSERT_FALSE(lexer.has_errors());
}

TEST_F(JsonLexerTest, TestSimpleTokens)
{
    CheckLexeme(L"[", json::lexeme(json::token::begin_array, textpos(1, 1), L"["), L"Begin array 1.1");
    CheckLexeme(L" \t[", json::lexeme(json::token::begin_array, textpos(1, 3), L"["), L"Begin array 1.2");
    CheckLexeme(L"\r\n[", json::lexeme(json::token::begin_array, textpos(2, 1), L"["), L"Begin array 1.3");
    CheckLexeme(L" \t\r\n[", json::lexeme(json::token::begin_array, textpos(2, 1), L"["), L"Begin array 1.4");
    CheckLexeme(L" \t\r\n \t[", json::lexeme(json::token::begin_array, textpos(2, 3), L"["), L"Begin array 1.5");
    CheckLexeme(L"\r\n\r\n\t[", json::lexeme(json::token::begin_array, textpos(3, 2), L"["), L"Begin array 1.6");
    CheckLexeme(L"\n[", json::lexeme(json::token::begin_array, textpos(2, 1), L"["), L"Begin array 1.7");
    CheckLexeme(L"{", json::lexeme(json::token::begin_object, textpos(1, 1), L"{"), L"Begin object 1.1");
    CheckLexeme(L"]", json::lexeme(json::token::end_array, textpos(1, 1), L"]"), L"End array 1.1");
    CheckLexeme(L"}", json::lexeme(json::token::end_object, textpos(1, 1), L"}"), L"End object 1.1");
    CheckLexeme(L"false", json::lexeme(json::token::literal_false, textpos(1, 1), L"false"), L"Literal 1.1");
    CheckLexeme(L"null", json::lexeme(json::token::literal_null, textpos(1, 1), L"null"), L"Literal 2.1");
    CheckLexeme(L"true", json::lexeme(json::token::literal_true, textpos(1, 1), L"true"), L"Literal 3.1");
    CheckLexeme(L":", json::lexeme(json::token::name_separator, textpos(1, 1), L":"), L"Name separator 1.1");
    CheckLexeme(L",", json::lexeme(json::token::value_separator, textpos(1, 1), L","), L"Value separator 1.1");
}

TEST_F(JsonLexerTest, TestStrings)
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

TEST_F(JsonLexerTest, TestNumbers)
{
    CheckLexeme(L"12345", json::lexeme(json::token::number_int, textpos(1, 1), L"12345"), L"Num 1.1");
    CheckLexeme(L"-12345", json::lexeme(json::token::number_int, textpos(1, 1), L"-12345"), L"Num 1.2");
    CheckLexeme(L"123.456", json::lexeme(json::token::number_decimal, textpos(1, 1), L"123.456"), L"Num 2.1");
    CheckLexeme(L"-123.456", json::lexeme(json::token::number_decimal, textpos(1, 1), L"-123.456"), L"Num 2.2");
    CheckLexeme(L"1.23456E10", json::lexeme(json::token::number_float, textpos(1, 1), L"1.23456E10"), L"Num 3.1");
    CheckLexeme(L"1.23456e10", json::lexeme(json::token::number_float, textpos(1, 1), L"1.23456e10"), L"Num 3.2");
    CheckLexeme(L"1.23456E+10", json::lexeme(json::token::number_float, textpos(1, 1), L"1.23456E+10"), L"Num 3.11");
    CheckLexeme(L"1.23456e+10", json::lexeme(json::token::number_float, textpos(1, 1), L"1.23456e+10"), L"Num 3.21");
    CheckLexeme(L"-1.23456E10", json::lexeme(json::token::number_float, textpos(1, 1), L"-1.23456E10"), L"Num 3.3");
    CheckLexeme(L"-1.23456e10", json::lexeme(json::token::number_float, textpos(1, 1), L"-1.23456e10"), L"Num 3.4");
    CheckLexeme(L"1.23456E-10", json::lexeme(json::token::number_float, textpos(1, 1), L"1.23456E-10"), L"Num 3.5");
    CheckLexeme(L"1.23456e-10", json::lexeme(json::token::number_float, textpos(1, 1), L"1.23456e-10"), L"Num 3.6");
    CheckLexeme(L"-1.23456E-10", json::lexeme(json::token::number_float, textpos(1, 1), L"-1.23456E-10"), L"Num 3.7");
    CheckLexeme(L"-1.23456e-10", json::lexeme(json::token::number_float, textpos(1, 1), L"-1.23456e-10"), L"Num 3.8");
    CheckLexeme(L"0", json::lexeme(json::token::number_int, textpos(1, 1), L"0"), L"Num 4.1");
    CheckLexeme(L"-0", json::lexeme(json::token::number_int, textpos(1, 1), L"-0"), L"Num 4.2");
    CheckLexeme(L"0.0", json::lexeme(json::token::number_decimal, textpos(1, 1), L"0.0"), L"Num 4.3");
    CheckLexeme(L"-0.0", json::lexeme(json::token::number_decimal, textpos(1, 1), L"-0.0"), L"Num 4.4");
    CheckLexeme(L"0.0e0", json::lexeme(json::token::number_float, textpos(1, 1), L"0.0e0"), L"Num 4.5");
    CheckLexeme(L"-0.0e-0", json::lexeme(json::token::number_float, textpos(1, 1), L"-0.0e-0"), L"Num 4.6");
}

TEST_F(JsonLexerTest, TestTexts)
{
    lex_vector lv;
    //
    CheckText(L"", lv, L"Txt1");
    //
    lv.clear();
    lv.add(json::token::begin_array, 1, 1, L"[");
    lv.add(json::token::end_array, 1, 2, L"]");
    CheckText(L"[]", lv, L"Txt2");
    //
    lv.clear();
    lv.add(json::token::begin_array, 1, 1, L"[");
    lv.add(json::token::end_array, 1, 2, L"]");
    lv.add(json::token::begin_object, 2, 1, L"{");
    lv.add(json::token::end_object, 2, 2, L"}");
    lv.add(json::token::literal_false, 3, 1, L"false");
    lv.add(json::token::literal_null, 3, 7, L"null");
    lv.add(json::token::literal_true, 3, 12, L"true");
    lv.add(json::token::string, 4, 1, L"Name 1");
    lv.add(json::token::name_separator, 4, 9, L":");
    lv.add(json::token::string, 4, 10, L"Value 1");
    lv.add(json::token::value_separator, 4, 19, L",");
    CheckText(L"[]\n{}\nfalse null true\n\"Name 1\":\"Value 1\",", lv, L"Txt3");
}

TEST_F(JsonLexerTest, TestLexerErrors)
{
    CheckError(L"try", json::parser_msg_kind::err_invalid_literal_fmt, textpos(1, 1), L"E1010.1");
    CheckError(L"\ntrue2", json::parser_msg_kind::err_invalid_literal_fmt, textpos(2, 1), L"E1010.2");
    CheckError(L"true\ntrue2", json::parser_msg_kind::err_invalid_literal_fmt, textpos(2, 1), L"E1010.3");
    CheckError(L"null\nnulltrue", json::parser_msg_kind::err_invalid_literal_fmt, textpos(2, 1), L"E1010.4");
    CheckError(L"false\nfalsetrue", json::parser_msg_kind::err_invalid_literal_fmt, textpos(2, 1), L"E1010.5");
    //
    CheckError(L"-", json::parser_msg_kind::err_invalid_number, textpos(1, 1), L"E1012.1");
    CheckError(L"-.", json::parser_msg_kind::err_invalid_number, textpos(1, 2), L"E1012.2");
    CheckError(L"123.", json::parser_msg_kind::err_invalid_number, textpos(1, 4), L"E1012.3");
    CheckError(L"123.0\"", json::parser_msg_kind::err_invalid_number, textpos(1, 6), L"E1012.4");
    CheckError(L"1e", json::parser_msg_kind::err_invalid_number, textpos(1, 2), L"E1012.5");
    CheckError(L"1eA", json::parser_msg_kind::err_invalid_number, textpos(1, 3), L"E1012.6");
    CheckError(L"1e-1A", json::parser_msg_kind::err_invalid_number, textpos(1, 5), L"E1012.7");
    CheckError(L"00", json::parser_msg_kind::err_invalid_number, textpos(1, 2), L"E1012.8");
    //
    CheckError(L"\"\b\"", json::parser_msg_kind::err_unallowed_char_fmt, textpos(1, 2), L"E1030.1");
    CheckError(L"\"\\u0001\x02\"", json::parser_msg_kind::err_unallowed_char_fmt, textpos(1, 8), L"E1030.2");
    CheckError(L"\"Hello\n", json::parser_msg_kind::err_unallowed_char_fmt, textpos(1, 7), L"E1030.3");
    //
    CheckError(L"\"\\u123\"", json::parser_msg_kind::err_unallowed_escape_seq, textpos(1, 2), L"E1040.1");
    CheckError(L"\"\\u123H\"", json::parser_msg_kind::err_unallowed_escape_seq, textpos(1, 2), L"E1040.2");
    CheckError(L"\"\\uABCD\\u123H\"", json::parser_msg_kind::err_unallowed_escape_seq, textpos(1, 8), L"E1040.3");
    //
    CheckError(L"\"", json::parser_msg_kind::err_unclosed_string, textpos(1, 1), L"E1050.1");
    CheckError(L"\"Hello", json::parser_msg_kind::err_unclosed_string, textpos(1, 6), L"E1050.2");
    CheckError(L"\"Hello\\n", json::parser_msg_kind::err_unclosed_string, textpos(1, 8), L"E1050.3");
    CheckError(L"\"Hello\\\"", json::parser_msg_kind::err_unclosed_string, textpos(1, 8), L"E1050.4");
    //
    CheckError(L"\t\b", json::parser_msg_kind::err_unexpected_char_fmt, textpos(1, 2), L"E1060.1");
    CheckError(L"[\x02]", json::parser_msg_kind::err_unexpected_char_fmt, textpos(1, 2), L"E1060.2");
    //
    CheckError(L"\"\\x", json::parser_msg_kind::err_unrecognized_escape_seq_fmt, textpos(1, 2), L"E1070.1");
}

}
