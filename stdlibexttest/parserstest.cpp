#include "CppUnitTest.h"
#include "parsers.h"
#include <fstream>
#include <limits>
#include "strutils.h"
#include "memchecker.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;
using namespace parsers;

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

TEST_CLASS(ParsersTest)
{
public:
    TEST_METHOD(TestTextPos)
    {
        textpos pos0;
        Assert::AreEqual(1, pos0.line(), L"Line 0");
        Assert::AreEqual(1, pos0.col(), L"Col 0");
        textpos pos1(11, 22);
        Assert::AreEqual(11, pos1.line(), L"Line 1");
        Assert::AreEqual(22, pos1.col(), L"Col 1");
        textpos pos2(pos1);
        Assert::AreEqual(11, pos1.line(), L"Line 2.1");
        Assert::AreEqual(22, pos1.col(), L"Col 2.1");
        Assert::AreEqual(11, pos2.line(), L"Line 2.2");
        Assert::AreEqual(22, pos2.col(), L"Col 2.2");
        Assert::IsTrue(pos1 == pos2, L"Operator ==");
        textpos pos3 = pos1;
        Assert::AreEqual(11, pos1.line(), L"Line 3.1");
        Assert::AreEqual(22, pos1.col(), L"Col 3.1");
        Assert::AreEqual(11, pos3.line(), L"Line 3.2");
        Assert::AreEqual(22, pos3.col(), L"Col 3.2");
        textpos pos4(std::move(pos1));
        Assert::AreEqual(11, pos4.line(), L"Line 4");
        Assert::AreEqual(22, pos4.col(), L"Col 4");
        textpos pos5 = std::move(pos2);
        Assert::AreEqual(11, pos5.line(), L"Line 5");
        Assert::AreEqual(22, pos5.col(), L"Col 5");
        //
        textpos pos6(1, 1);
        pos6++;
        Assert::AreEqual(1, pos6.line(), L"Line 6.1");
        Assert::AreEqual(2, pos6.col(), L"Col 6.1");
        Assert::IsTrue(textpos(1, 2) == pos6, L"== 6.1");
        pos6.newline();
        Assert::AreEqual(2, pos6.line(), L"Line 6.2");
        Assert::AreEqual(1, pos6.col(), L"Col 6.2");
        Assert::IsTrue(textpos(2, 1) == pos6, L"== 6.2");
    }

    void CompareMessages(const test_message_t* m1, const test_message_t* m2, wstring title)
    {
        title += L": ";
        Assert::IsTrue(m1->origin() == m2->origin(), (title + L"Origin").c_str());
        Assert::IsTrue(m1->severity() == m2->severity(), (title + L"Severity").c_str());
        Assert::IsTrue(m1->kind() == m2->kind(), (title + L"Kind").c_str());
        Assert::IsTrue(m1->pos() == m2->pos(), (title + L"Pos").c_str());
        Assert::AreEqual<wstring>(m1->source(), m2->source(), (title + L"Source").c_str());
        Assert::AreEqual<wstring>(m1->text(), m2->text(), (title + L"Text").c_str());
    }
    void CompareMessages(const test_message_t& m1, const test_message_t& m2, wstring title)
    {
        CompareMessages(&m1, &m2, title);
    }

    TEST_METHOD(TestMessages)
    {
        textpos pos1(11, 22);
        test_message_t m1(msg_origin::lexer, msg_severity::error, test_message_kind::e_error2, pos1, L"file1.txt", L"Invalid char");
        Assert::IsTrue(msg_origin::lexer == m1.origin(), L"Origin 1");
        Assert::IsTrue(msg_severity::error == m1.severity(), L"Severity 1");
        Assert::IsTrue(test_message_kind::e_error2 == m1.kind(), L"Kind 1");
        Assert::IsTrue(pos1 == m1.pos(), L"Pos 1");
        Assert::AreEqual<wstring>(L"file1.txt", m1.source(), L"Source 1");
        Assert::AreEqual<wstring>(L"Invalid char", m1.text(), L"Text 1");
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
        Assert::IsTrue(msg_origin::parser == m4.origin(), L"Origin 4");
        Assert::IsTrue(msg_severity::hint == m4.severity(), L"Severity 4");
        Assert::IsTrue(test_message_kind::h_hint1 == m4.kind(), L"Kind 4");
        Assert::IsTrue(pos4 == m4.pos(), L"Pos 4");
        Assert::AreEqual<wstring>(L"", m4.source(), L"Source 4");
        Assert::AreEqual<wstring>(L"", m4.text(), L"Text 4");
    }

    TEST_METHOD(TestMessageCollector)
    {
        auto check_size = [](test_msg_collector_t& collector, int total, int errors, int warns, int hints, wstring title)
        {
            title += L": ";
            Assert::AreEqual<int>(total, collector.size(), (title + L"total").c_str());
            Assert::AreEqual<int>(errors, collector.errors().size(), (title + L"errors").c_str());
            Assert::AreEqual<int>(warns, collector.warnings().size(), (title + L"warns").c_str());
            Assert::AreEqual<int>(hints, collector.hints().size(), (title + L"hints").c_str());
            Assert::IsTrue((total == 0 && !collector.has_messages()) || collector.has_messages(), (title + L"has_messages").c_str());
            Assert::IsTrue((errors == 0 && !collector.has_errors()) || collector.has_errors(), (title + L"has_errors").c_str());
            Assert::IsTrue((warns == 0 && !collector.has_warnings()) || collector.has_warnings(), (title + L"has_warnings").c_str());
            Assert::IsTrue((hints == 0 && !collector.has_hints()) || collector.has_hints(), (title + L"has_hints").c_str());
        };

        test_msg_collector_t collect1;
        check_size(collect1, 0, 0, 0, 0, L"Size 1");
        textpos pos1(11, 22);
        collect1.add_error(msg_origin::parser, test_message_kind::e_error1, pos1, L"file1.txt", L"Error 1");
        check_size(collect1, 1, 1, 0, 0, L"Size 2");
        test_message_t* m1 = collect1.errors()[0];
        Assert::IsTrue(msg_origin::parser == m1->origin(), L"Origin 1");
        Assert::IsTrue(msg_severity::error == m1->severity(), L"Severity 1");
        Assert::IsTrue(test_message_kind::e_error1 == m1->kind(), L"Kind 1");
        Assert::IsTrue(pos1 == m1->pos(), L"Pos 1");
        Assert::AreEqual<wstring>(L"file1.txt", m1->source(), L"Source 1");
        Assert::AreEqual<wstring>(L"Error 1", m1->text(), L"Text 1");
        textpos pos2(21, 32);
        collect1.add_error(msg_origin::lexer, test_message_kind::e_error2, pos2, L"file1.txt", L"Error 2");
        check_size(collect1, 2, 2, 0, 0, L"Size 3");
        textpos pos3(33, 44);
        collect1.add_warning(msg_origin::lexer, test_message_kind::w_warning1, pos3, L"file2.txt", L"Warn 1");
        check_size(collect1, 3, 2, 1, 0, L"Size 4");
        textpos pos4(55, 77);
        collect1.add_hint(msg_origin::generator, test_message_kind::h_hint1, pos4, L"file3.txt", L"Hint 1");
        check_size(collect1, 4, 2, 1, 1, L"Size 5");
        //
        textpos pos5(99, 88);
        collect1.add(new test_message_t(msg_origin::other, msg_severity::error, test_message_kind::e_error2, pos5, L"file41.txt", L"Msg 11"));
        check_size(collect1, 5, 3, 1, 1, L"Size 6.1");
        collect1.add(new test_message_t(msg_origin::other, msg_severity::warning, test_message_kind::w_warning2, pos5, L"file42.txt", L"Msg 12"));
        check_size(collect1, 6, 3, 2, 1, L"Size 6.2");
        collect1.add(new test_message_t(msg_origin::other, msg_severity::hint, test_message_kind::h_hint1, pos5, L"file43.txt", L"Msg 13"));
        check_size(collect1, 7, 3, 2, 2, L"Size 6.3");
        collect1.add(new test_message_t(msg_origin::other, msg_severity::none, test_message_kind::info1, pos5, L"file44.txt", L"Msg 14"));
        check_size(collect1, 8, 3, 2, 2, L"Size 6.4");
        //
        int count = 0;
        for (test_message_t* msg : collect1)
        {
            count++;
        }
        Assert::AreEqual(8, count, L"Count 1");
        count = 0;
        for (test_message_t* msg : collect1.errors())
        {
            Assert::IsTrue(msg_severity::error == msg->severity(), L"Error count");
            count++;
        }
        Assert::AreEqual(3, count, L"Count 2");
        count = 0;
        for (test_message_t* msg : collect1.warnings())
        {
            Assert::IsTrue(msg_severity::warning == msg->severity(), L"Warn count");
            count++;
        }
        Assert::AreEqual(2, count, L"Count 3");
        count = 0;
        for (test_message_t* msg : collect1.hints())
        {
            Assert::IsTrue(msg_severity::hint == msg->severity(), L"Hint count");
            count++;
        }
        Assert::AreEqual(2, count, L"Count 4");
    }
};