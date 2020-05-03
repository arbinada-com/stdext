#include "CppUnitTest.h"
#include "json.h"
#include <fstream>
#include <limits>
#include "strutils.h"
#include "memchecker.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

TEST_CLASS(JsonLexerTest)
{
public:
    TEST_METHOD(TestEmptyStreams)
    {
        wstringstream ss(L"");
        ioutils::text_reader r(ss);
        json::lexer lexer(&r);
        json::lexeme l;
        Assert::IsFalse(lexer.next_lexeme(l));
        Assert::IsFalse(lexer.has_error());
    }

};