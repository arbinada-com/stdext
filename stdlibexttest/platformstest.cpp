#include "CppUnitTest.h"
#include "testutils.h"
#include "platforms.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

TEST_CLASS(PlatformsTest)
{

    TEST_METHOD(TestEndianess)
    {
#if defined(__MACHINEX86_X64)
        Assert::IsTrue(endianess::platform_value() == endianess::byte_order::little_endian, L"x86_64");
#else
        Assert::IsTrue(endianess::platform_value() == endianess::byte_order::big_endian, L"not x86_64");
#endif
    }
};
