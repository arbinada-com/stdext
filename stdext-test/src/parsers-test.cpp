#include <gtest/gtest.h>
#include "parsers.h"
#include <fstream>
#include <limits>
#include "strutils.h"
#include "testutils.h"

using namespace std;
using namespace stdext;
using namespace parsers;
using namespace testutils;

namespace parsers_test
{

enum class test_message_kind
{
    w_warning1,
    w_warning2,
    e_error1,
    e_error2,
    h_hint1,
    info1
};

typedef parsers::message<test_message_kind> test_message_t;
typedef parsers::msg_collector<test_message_kind> test_msg_collector_t;

class ParsersTest : public testing::Test
{
protected:
    void CompareMessages(const test_message_t* m1, const test_message_t* m2, wstring title)
    {
        title += L": ";
        EXPECT_TRUE(m1->origin() == m2->origin()) << title + L"Origin";
        EXPECT_TRUE(m1->severity() == m2->severity()) << title + L"Severity";
        EXPECT_TRUE(m1->kind() == m2->kind()) << title + L"Kind";
        EXPECT_TRUE(m1->pos() == m2->pos()) << title + L"Pos";
        EXPECT_EQ(m1->source(), m2->source()) << title + L"Source";
        EXPECT_EQ(m1->text(), m2->text()) << title + L"Text";
    }
    void CompareMessages(const test_message_t& m1, const test_message_t& m2, wstring title)
    {
        CompareMessages(&m1, &m2, title);
    }
};

TEST_F(ParsersTest, TestTextPos)
{
    textpos pos0;
    ASSERT_EQ(1, pos0.line()) << L"Line 0";
    ASSERT_EQ(1, pos0.col()) << L"Col 0";
    textpos pos1(11, 22);
    ASSERT_EQ(11, pos1.line()) << L"Line 1";
    ASSERT_EQ(22, pos1.col()) << L"Col 1";
    textpos pos2(pos1);
    ASSERT_EQ(11, pos1.line()) << L"Line 2.1";
    ASSERT_EQ(22, pos1.col()) << L"Col 2.1";
    ASSERT_EQ(11, pos2.line()) << L"Line 2.2";
    ASSERT_EQ(22, pos2.col()) << L"Col 2.2";
    ASSERT_TRUE(pos1 == pos2) << L"Operator ==";
    textpos pos3 = pos1;
    ASSERT_EQ(11, pos1.line()) << L"Line 3.1";
    ASSERT_EQ(22, pos1.col()) << L"Col 3.1";
    ASSERT_EQ(11, pos3.line()) << L"Line 3.2";
    ASSERT_EQ(22, pos3.col()) << L"Col 3.2";
    textpos pos4(std::move(pos1));
    ASSERT_EQ(11, pos4.line()) << L"Line 4";
    ASSERT_EQ(22, pos4.col()) << L"Col 4";
    textpos pos5 = std::move(pos2);
    ASSERT_EQ(11, pos5.line()) << L"Line 5";
    ASSERT_EQ(22, pos5.col()) << L"Col 5";
    //
    textpos pos6(1, 1);
    pos6++;
    ASSERT_EQ(1, pos6.line()) << L"Line 6.1";
    ASSERT_EQ(2, pos6.col()) << L"Col 6.1";
    ASSERT_TRUE(textpos(1, 2) == pos6) << L"== 6.1";
    pos6.newline();
    ASSERT_EQ(2, pos6.line()) << L"Line 6.2";
    ASSERT_EQ(1, pos6.col()) << L"Col 6.2";
    ASSERT_TRUE(textpos(2, 1) == pos6) << L"== 6.2";
}

TEST_F(ParsersTest, TestMessages)
{
    textpos pos1(11, 22);
    test_message_t m1(msg_origin::lexer, msg_severity::error, test_message_kind::e_error2, pos1, L"file1.txt", L"Invalid char");
    ASSERT_TRUE(msg_origin::lexer == m1.origin()) << L"Origin 1";
    ASSERT_TRUE(msg_severity::error == m1.severity()) << L"Severity 1";
    ASSERT_TRUE(test_message_kind::e_error2 == m1.kind()) << L"Kind 1";
    ASSERT_TRUE(pos1 == m1.pos()) << L"Pos 1";
    ASSERT_EQ(L"file1.txt", m1.source()) << L"Source 1";
    ASSERT_EQ(L"Invalid char", m1.text()) << L"Text 1";
    test_message_t m21(m1);
    CompareMessages(m1, m21, L"Copy ctor");
    test_message_t m22 = m1;
    CompareMessages(m1, m22, L"Copy oper");
    test_message_t m31(std::move(m21));
    CompareMessages(m1, m31, L"Move ctor");
    test_message_t m32 = std::move(m22);
    CompareMessages(m1, m32, L"Move oper");
    textpos pos4(33, 77);
    test_message_t m4(msg_origin::parser, msg_severity::hint, test_message_kind::h_hint1, pos4, L"");
    ASSERT_TRUE(msg_origin::parser == m4.origin()) << L"Origin 4";
    ASSERT_TRUE(msg_severity::hint == m4.severity()) << L"Severity 4";
    ASSERT_TRUE(test_message_kind::h_hint1 == m4.kind()) << L"Kind 4";
    ASSERT_TRUE(pos4 == m4.pos()) << L"Pos 4";
    ASSERT_EQ(L"", m4.source()) << L"Source 4";
    ASSERT_EQ(L"", m4.text()) << L"Text 4";
}

TEST_F(ParsersTest, TestMessageCollector)
{
    auto check_size = [](test_msg_collector_t& collector, size_t total, size_t errors, size_t warns, size_t hints, size_t infos, wstring title)
    {
        title += L": ";
        EXPECT_EQ(total, collector.size()) << title + L"total";
        EXPECT_EQ(errors, collector.errors().size()) << title + L"errors";
        EXPECT_EQ(warns, collector.warnings().size()) << title + L"warns";
        EXPECT_EQ(hints, collector.hints().size()) << title + L"hints";
        EXPECT_EQ(infos, collector.infos().size()) << title + L"infos";
        EXPECT_TRUE((total == 0 && !collector.has_messages()) || collector.has_messages()) << title + L"has_messages";
        EXPECT_TRUE((errors == 0 && !collector.has_errors()) || collector.has_errors()) << title + L"has_errors";
        EXPECT_TRUE((warns == 0 && !collector.has_warnings()) || collector.has_warnings()) << title + L"has_warnings";
        EXPECT_TRUE((hints == 0 && !collector.has_hints()) || collector.has_hints()) << title + L"has_hints";
        EXPECT_TRUE((infos == 0 && !collector.has_infos()) || collector.has_hints()) << title + L"has_infos";
    };

    test_msg_collector_t collect1;
    check_size(collect1, 0, 0, 0, 0, 0, L"Size 0");
    {
        textpos pos(11, 22);
        collect1.add_error(msg_origin::parser, test_message_kind::e_error1, pos, L"file1.txt", L"Error 1");
        check_size(collect1, 1, 1, 0, 0, 0, L"Size 1");
        test_message_t* msg = collect1.errors()[0];
        EXPECT_EQ(msg_origin::parser, msg->origin()) << L"Error origin";
        EXPECT_EQ(msg_severity::error, msg->severity()) << L"Error severity";
        EXPECT_EQ(test_message_kind::e_error1, msg->kind()) << L"Error kind";
        EXPECT_EQ(pos, msg->pos()) << L"Error pos";
        EXPECT_EQ(L"file1.txt", msg->source()) << L"Error source";
        EXPECT_EQ(L"Error 1", msg->text()) << L"Error text";
    }
    {
        textpos pos(21, 32);
        collect1.add_error(msg_origin::lexer, test_message_kind::e_error2, pos, L"file1.txt", L"Error 2");
        check_size(collect1, 2, 2, 0, 0, 0, L"Size 2");
    }
    {
        textpos pos(33, 44);
        collect1.add_warning(msg_origin::lexer, test_message_kind::w_warning1, pos, L"file2.txt", L"Warn 1");
        check_size(collect1, 3, 2, 1, 0, 0, L"Size 3");
        test_message_t* msg = collect1.warnings()[0];
        EXPECT_EQ(msg_origin::lexer, msg->origin()) << L"Warn origin";
        EXPECT_EQ(msg_severity::warning, msg->severity()) << L"Warn severity";
        EXPECT_EQ(test_message_kind::w_warning1, msg->kind()) << L"Warn kind";
        EXPECT_EQ(pos, msg->pos()) << L"Warn pos";
        EXPECT_EQ(L"file2.txt", msg->source()) << L"Warn source";
        EXPECT_EQ(L"Warn 1", msg->text()) << L"Warn text";
    }
    {
        textpos pos(55, 77);
        collect1.add_hint(msg_origin::generator, test_message_kind::h_hint1, pos, L"file3.txt", L"Hint 1");
        check_size(collect1, 4, 2, 1, 1, 0, L"Size 4");
        test_message_t* msg = collect1.hints()[0];
        EXPECT_EQ(msg_origin::generator, msg->origin()) << L"Hint origin";
        EXPECT_EQ(msg_severity::hint, msg->severity()) << L"Hint severity";
        EXPECT_EQ(test_message_kind::h_hint1, msg->kind()) << L"Hint kind";
        EXPECT_EQ(pos, msg->pos()) << L"Hint pos";
        EXPECT_EQ(L"file3.txt", msg->source()) << L"Hint source";
        EXPECT_EQ(L"Hint 1", msg->text()) << L"Hint text";
    }
    {
        textpos pos(57, 79);
        collect1.add_info(msg_origin::generator, test_message_kind::info1, pos, L"file31.txt", L"Info 1");
        check_size(collect1, 5, 2, 1, 1, 1, L"Size 5");
        test_message_t* msg = collect1.infos()[0];
        EXPECT_EQ(msg_origin::generator, msg->origin()) << L"Info origin";
        EXPECT_EQ(msg_severity::info, msg->severity()) << L"Info severity";
        EXPECT_EQ(test_message_kind::info1, msg->kind()) << L"Info kind";
        EXPECT_EQ(pos, msg->pos()) << L"Info pos";
        EXPECT_EQ(L"file31.txt", msg->source()) << L"Info source";
        EXPECT_EQ(L"Info 1", msg->text()) << L"Info text";
    }
    //
    textpos pos5(99, 88);
    collect1.add(new test_message_t(msg_origin::other, msg_severity::error, test_message_kind::e_error2, pos5, L"file41.txt", L"Msg 11"));
    check_size(collect1, 6, 3, 1, 1, 1, L"Size 6.1");
    collect1.add(new test_message_t(msg_origin::other, msg_severity::warning, test_message_kind::w_warning2, pos5, L"file42.txt", L"Msg 12"));
    check_size(collect1, 7, 3, 2, 1, 1, L"Size 6.2");
    collect1.add(new test_message_t(msg_origin::other, msg_severity::hint, test_message_kind::h_hint1, pos5, L"file43.txt", L"Msg 13"));
    check_size(collect1, 8, 3, 2, 2, 1, L"Size 6.3");
    collect1.add(new test_message_t(msg_origin::other, msg_severity::info, test_message_kind::info1, pos5, L"file44.txt", L"Msg 14"));
    check_size(collect1, 9, 3, 2, 2, 2, L"Size 6.4");
    //
    int count = 0;
    for (test_message_t* msg : collect1)
    {
        ASSERT_NE(msg, nullptr);
        count++;
    }
    EXPECT_EQ(9, count) << L"Count 1";
    count = 0;
    for (test_message_t* msg : collect1.errors())
    {
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg_severity::error, msg->severity()) << L"Error count";
        count++;
    }
    EXPECT_EQ(3, count) << L"Count 2";
    count = 0;
    for (test_message_t* msg : collect1.warnings())
    {
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg_severity::warning, msg->severity()) << L"Warn count";
        count++;
    }
    EXPECT_EQ(2, count) << L"Count 3";
    count = 0;
    for (test_message_t* msg : collect1.hints())
    {
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg_severity::hint, msg->severity()) << L"Hint count";
        count++;
    }
    EXPECT_EQ(2, count) << L"Count 4";
    count = 0;
    for (test_message_t* msg : collect1.infos())
    {
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg_severity::info, msg->severity()) << L"Info count";
        count++;
    }
    EXPECT_EQ(2, count) << L"Count 5";
}

}
