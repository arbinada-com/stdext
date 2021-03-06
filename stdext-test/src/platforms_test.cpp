#include <gtest/gtest.h>
#include "platforms.h"

using namespace std;

namespace stdext
{
namespace platform_test
{

TEST(PlatformsTest, TestEndianess)
{
#if defined(__STDEXT_X86_X64_OR_I386)
    ASSERT_EQ(endianess::platform_value(), endianess::byte_order::little_endian) << L"x86_64";
#else
    ASSERT_EQ(endianess::platform_value(), endianess::byte_order::big_endian) << L"not x86_64";
#endif
}

}
}
