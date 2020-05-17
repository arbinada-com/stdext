#include "CppUnitTest.h"
#include "ioutils.h"
#include <fstream>
#include <codecvt>
#include "testutils.h"
#include "strutils.h"
#include "memchecker.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

TEST_CLASS(IOUtilsTest)
{
    const wstring test_file_name = L"current.txt";

    void CheckWriteRead(const wstring& s, const ioutils::file_encoding enc, const wstring title)
    {
        wstring title2 = title + L": ";
        {
            ioutils::text_writer w(test_file_name, enc);
            w.stream() << s;
        }
        ioutils::text_reader r(test_file_name, enc);
        wstring s2;
        s2.reserve(s.length());
        r.read_all(s2);
        Assert::AreEqual(s.length(), s2.length(), (title2 + L"length").c_str());
        Assert::IsTrue(s == s2, (title2 + L"length").c_str());
    }

    TEST_METHOD(TestWriteRead)
    {
        wstring s;
        s.reserve(0xFF);
        for (int i = 0x1; i <= 0xFF; i++)
        {
            wchar_t c = static_cast<wchar_t>(i);
            s += c;
        }
        CheckWriteRead(s, ioutils::file_encoding::ansi, L"ANSI");
        s.clear();
        s.reserve(0xFFFF);
        for (int i = 0x1; i <= 0xFFFF; i++)
        {
            wchar_t c = static_cast<wchar_t>(i);
            //s += strutils::format(L"%d(0x%X): ", c, c);
            if (ioutils::is_noncharacter(c) || ioutils::is_high_surrogate(c))
                c = ioutils::replacement_character();
            s += c;
        }
        CheckWriteRead(s, ioutils::file_encoding::utf8, L"UTF-8");
        CheckWriteRead(s, ioutils::file_encoding::utf16, L"UTF-16");
    }
};