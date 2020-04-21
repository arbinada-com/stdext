#include "CppUnitTest.h"
#include "variants.h"
#include "strutils.h"
#include "memchecker.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;
using namespace variants;

TEST_CLASS(VariantsTest)
{
public:
    TEST_METHOD(TestBooleans)
    {
        variants::variant v11 = true;
        variants::variant v12 = v11;
        Assert::IsTrue(v11.vtype() == value_type::vt_bool, L"VType 11");
        Assert::IsTrue(v12.vtype() == value_type::vt_bool, L"VType 12");
        Assert::IsTrue(v11.equal(v12), L"Compare 1.1");
        Assert::IsTrue(v12.equal(v11), L"Compare 1.2");
        Assert::IsTrue(v11 == v12, L"Compare 1.3");
        Assert::IsTrue(v12 == v11, L"Compare 1.4");
        Assert::IsTrue(v11.equal(true), L"Compare 2.1");
        Assert::IsTrue(v11 == true, L"Compare 2.2");
        Assert::IsTrue(v12.equal(true), L"Compare 2.3");
        Assert::IsTrue(v12 == true, L"Compare 2.4.1");
        Assert::IsTrue(true == v12, L"Compare 2.4.2");
        Assert::IsTrue(v12 != false, L"Compare 2.5.1");
        Assert::IsTrue(false != v12, L"Compare 2.5.1");
        bool b1 = static_cast<bool>(v11);
        Assert::IsTrue(b1, L"Compare 3.1");
        Assert::IsTrue(v11.equal(b1), L"Compare 3.2");
        Assert::IsTrue(v11 == b1, L"Compare 3.3");
        variant v21 = false;
        variant v22(v21);
        Assert::IsTrue(v21.vtype() == value_type::vt_bool, L"VType 21");
        Assert::IsTrue(v22.vtype() == value_type::vt_bool, L"VType 22");
        Assert::IsTrue(v21.equal(false), L"Compare 5.1");
        Assert::IsTrue(v21 == false, L"Compare 5.2");
        Assert::IsTrue(v22.equal(false), L"Compare 5.3");
        Assert::IsTrue(v22 == false, L"Compare 5.4");
        bool b2 = v21;
        Assert::IsFalse(b2, L"Compare 6.1");
        Assert::IsTrue(v21.equal(b2), L"Compare 6.2");
        Assert::IsTrue(v21 == b2, L"Compare 6.3");

        Assert::IsFalse(v11 == v21, L"Compare 7.1");
        Assert::IsTrue(v11 != v21, L"Compare 7.2");

        variant v31 = v11;
        variant v32(v21);
        Assert::IsTrue(v31.vtype() == value_type::vt_bool, L"VType 31");
        Assert::IsTrue(v32.vtype() == value_type::vt_bool, L"VType 32");
        Assert::IsTrue(v31 == v11, L"Compare 8.1");
        Assert::IsTrue(v32 == v21, L"Compare 8.2");
        // operations
        variant v41 = false, v42 = true, v43 = true;
        Assert::IsTrue(!v41, L"Op 9.1");
        Assert::IsTrue(v41 || v42, L"Op 9.2");
        Assert::IsTrue(v42 && v43, L"Op 9.3");
        Assert::IsFalse(v41 && v42, L"Op 9.3");
    }

    bool TestBooleansMemoryF1(bool n1, bool n2) { return n1 && n2; }
    TEST_METHOD(TestBooleansMemory)
    {
        memchecker chk;
        {
            variant v11 = true;
            variant v12 = v11;
            variant v13 = v11 && v12;
            bool b1 = TestBooleansMemoryF1(v11, v12);
            Assert::IsTrue(v13 == b1); // Assert::AreEqual produces _CRT_BLOCK memory leaks
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }

    TEST_METHOD(TestIntegers)
    {
        variant v11 = 123, v12(-123), v13 = 0;
        Assert::IsTrue(v11.vtype() == value_type::vt_int, L"VType 11");
        Assert::IsTrue(v12.vtype() == value_type::vt_int, L"VType 12");
        Assert::IsTrue(v13.vtype() == value_type::vt_int, L"VType 13");
        Assert::AreEqual(123, (int)v11, L"Compare 1.1.1");
        Assert::AreEqual(123L, (long)v11, L"Compare 1.1.2");
        Assert::IsTrue(v11 == 123, L"Compare 1.2.1");
        Assert::IsTrue(123 == v11, L"Compare 1.2.2");
        Assert::IsTrue(v11.equal(123), L"Compare 1.2.3");
        Assert::IsTrue(v11 != 124, L"Compare 1.3.1");
        Assert::IsTrue(124 != v11, L"Compare 1.3.2");
        Assert::IsTrue(v11 > 0, L"Compare 2.1.1");
        Assert::IsTrue(124 > v11, L"Compare 2.1.2");
        Assert::IsTrue(v11 < 124, L"Compare 2.2.1");
        Assert::IsTrue(0 < v11, L"Compare 2.2.2");
        Assert::IsTrue(v11 >= 123, L"Compare 2.3.1");
        Assert::IsTrue(123 >= v11, L"Compare 2.3.2");
        Assert::IsTrue(v12 < 0, L"Compare 2.4.1");
        Assert::IsTrue(-124 < v12, L"Compare 2.4.2");
        Assert::IsTrue(v11 <= 123, L"Compare 2.5.1");
        Assert::IsTrue(123 <= v11, L"Compare 2.5.2");
        Assert::IsTrue(v11 > v12, L"Compare 3.1");
        Assert::IsTrue(v11 >= v12, L"Compare 3.2");
        Assert::IsTrue(v12 < v13, L"Compare 3.3");
        Assert::IsTrue(v12 <= v13, L"Compare 3.4");
        int n1 = v11;
        Assert::AreEqual(n1, (int)v11, L"Compare 4.1");
        variant v14 = v11;
        Assert::IsTrue(v14.vtype() == value_type::vt_int, L"Compare 4.2");
        Assert::AreEqual((int)v14, (int)v11, L"Compare 4.3");
        // opertaions
        variant v21 = 10;
        variant v22 = v21 + 10;
        Assert::AreEqual(20, (int)v22, L"Ops 5.1.1");
        variant v23 = 10 + v21;
        Assert::AreEqual(20, (int)v23, L"Ops 5.1.2");
        variant v24 = v21 + v22;
        Assert::AreEqual(30, (int)v24, L"Ops 5.1.3");
        variant v25 = v22 - 10;
        Assert::AreEqual(10, (int)v25, L"Ops 5.2.1");
        variant v26 = 10 - v22;
        Assert::AreEqual(-10, (int)v26, L"Ops 5.2.2");
        variant v27 = v21 - v22;
        Assert::AreEqual(-10, (int)v27, L"Ops 5.2.3");
    }

    int TestIntegersMemorySum1(int n1, int n2) { return n1 + n2; }
    TEST_METHOD(TestIntegersMemory)
    {
        memchecker chk;
        {
            variant v11 = 123;
            variant v12 = v11;
            variant v13 = v11 + v12;
            int n1 = TestIntegersMemorySum1(v11, v12);
            Assert::IsTrue(v13 == n1);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }
};