#include "CppUnitTest.h"
#include "variants.h"
#include "strutils.h"
#include "memchecker.h"
#include <vector>
#include <limits>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;
using namespace variants;

template <class Type1, class Type2, class TypeR>
class test_data
{
public:
    test_data(const operation op, const Type1 v1, const Type2 v2, const TypeR result, const std::wstring title)
        : m_operation(op), m_v1(v1), m_v2(v2), m_result(result), m_title(title)
    { }
    variants::operation operation() const noexcept { return m_operation; }
    Type1 v1() const noexcept { return m_v1; }
    Type2 v2() const noexcept { return m_v2; }
    TypeR result() const noexcept { return m_result; }
    std::wstring title() const noexcept { return m_title; }
private:
    variants::operation m_operation;
    Type1 m_v1;
    Type2 m_v2;
    TypeR m_result;
    std::wstring m_title;
};

template <class Type1, class Type2, class TypeR>
class test_data_set : public std::vector<test_data<Type1, Type2, TypeR> >
{ 
public:
    typedef typename test_data<Type1, Type2, TypeR> test_data_t;
public:
    test_data_set(std::wstring title)
        : m_title(title)
    {}
    std::wstring title() const noexcept { return m_title; }
    void add(const operation op, const Type1 v1, const Type2 v2, const TypeR result, std::wstring title = L"")
    {
        if (title.empty())
            title = L"#" + std::to_wstring(this->size() + 1);
        push_back(test_data_t(op, v1, v2, result, title));
    }
private:
    std::wstring m_title;
};


TEST_CLASS(VariantsTest)
{
public:
    TEST_METHOD(TestOperationNames)
    {
        for (operation op = first_operation; op != last_operation; op = (operation)(static_cast<int>(op) + 1))
        {
            string s = variants::to_string(op);
            Assert::IsFalse(s == "unsupported",
                strutils::format(L"Undefined name for operation %d", static_cast<int>(op)).c_str());
        }
    }
    TEST_METHOD(TestVTypeNames)
    {
        for (value_type vtype = first_value_type; vtype != last_value_type; vtype = (value_type)(static_cast<int>(vtype) + 1))
        {
            string s = variants::to_string(vtype);
            Assert::IsFalse(s == "unsupported",
                strutils::format(L"Undefined name for value_type %d", static_cast<int>(vtype)).c_str());
        }
    }

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

    TEST_METHOD(TestBooleansMemory)
    {
        auto func1 = [](variant n1, variant n2)->variant { return n1 && n2; };
        memchecker chk;
        {
            variant v11 = true;
            variant v12 = v11;
            variant v13 = v11 && v12;
            bool b1 = func1(v11, v12);
            Assert::IsTrue(v13 == b1); // Assert::AreEqual produces _CRT_BLOCK memory leaks
            variant v14 = std::move(v13);
            Assert::IsTrue(v14 == b1);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }

    template <class Type1, class Type2, class TypeR>
    void RunTestsSet(test_data_set<Type1, Type2, TypeR> tests)
    {
        for (test_data<Type1, Type2, TypeR> test : tests)
        {
            auto make_title = [&](const wchar_t* msg) -> wstring
            {
                wstring ws = strutils::format(L"%s[%s] - %s", tests.title().c_str(), test.title().c_str(), msg);
                return ws;
            };
            variant v1 = test.v1();
            variant v2 = test.v2();
            variant vresult = test.result();
            Assert::IsTrue(v1.vtype() == to_value_type(test.v1()), L"v1 type");
            Assert::IsTrue(v2.vtype() == to_value_type(test.v2()), L"v2 type");
            Assert::IsTrue(vresult.vtype() == to_value_type(test.result()), L"vresult type");
            Assert::AreEqual<Type1>(test.v1(), v1, L"v1 value");
            Assert::AreEqual<Type2>(test.v2(), v2, L"v2 value");
            Assert::AreEqual<TypeR>(test.result(), vresult, L"vresult value");
            Type1 n1 = v1;
            Type2 n2 = v2;
            Assert::AreEqual<Type1>(n1, v1, L"v1 value");
            Assert::AreEqual<Type2>(n2, v2, L"v2 value");
            switch (test.operation())
            {
            case operation::cmp_eq:
                Assert::AreEqual<TypeR>(test.result(), v1 == v2, make_title(L"EQ 1").c_str());
                Assert::AreEqual<TypeR>(test.result(), v1 == n2, make_title(L"EQ 2").c_str());
                Assert::AreEqual<TypeR>(test.result(), n1 == v2, make_title(L"EQ 3").c_str());
                break;
            case operation::cmp_neq:
                Assert::AreEqual<TypeR>(test.result(), v1 != v2, make_title(L"NEQ 1").c_str());
                Assert::AreEqual<TypeR>(test.result(), v1 != n2, make_title(L"NEQ 2").c_str());
                Assert::AreEqual<TypeR>(test.result(), n1 != v2, make_title(L"NEQ 3").c_str());
                break;
            case operation::cmp_gt:
                Assert::AreEqual<TypeR>(test.result(), v1 > v2, make_title(L"GT 1").c_str());
                Assert::AreEqual<TypeR>(test.result(), v1 > n2, make_title(L"GT 2").c_str());
                Assert::AreEqual<TypeR>(test.result(), n1 > v2, make_title(L"GT 3").c_str());
                break;
            case operation::cmp_ge:
                Assert::AreEqual<TypeR>(test.result(), v1 >= v2, make_title(L"GE 1").c_str());
                Assert::AreEqual<TypeR>(test.result(), v1 >= n2, make_title(L"GE 2").c_str());
                Assert::AreEqual<TypeR>(test.result(), n1 >= v2, make_title(L"GE 3").c_str());
                break;
            case operation::cmp_lt:
                Assert::AreEqual<TypeR>(test.result(), v1 < v2, make_title(L"LT 1").c_str());
                Assert::AreEqual<TypeR>(test.result(), v1 < n2, make_title(L"LT 2").c_str());
                Assert::AreEqual<TypeR>(test.result(), n1 < v2, make_title(L"LT 3").c_str());
                break;
            case operation::cmp_le:
                Assert::AreEqual<TypeR>(test.result(), v1 <= v2, make_title(L"LE 1").c_str());
                Assert::AreEqual<TypeR>(test.result(), v1 <= n2, make_title(L"LE 2").c_str());
                Assert::AreEqual<TypeR>(test.result(), n1 <= v2, make_title(L"LE 3").c_str());
                break;
            case operation::add:
            {
                variant v3 = v1 + v2;
                variant v4 = n1 + v2;
                variant v5 = v1 + n2;
                Assert::AreEqual<TypeR>(test.result(), v1 + v2, make_title(L"ADD 1").c_str());
                Assert::AreEqual<TypeR>(test.result(), n1 + v2, make_title(L"ADD 2").c_str());
                Assert::AreEqual<TypeR>(test.result(), v1 + n2, make_title(L"ADD 3").c_str());
                Assert::AreEqual<TypeR>(test.result(), v3, make_title(L"ADD 4").c_str());
                Assert::AreEqual<TypeR>(test.result(), v4, make_title(L"ADD 5").c_str());
                Assert::AreEqual<TypeR>(test.result(), v5, make_title(L"ADD 6").c_str());
                break;
            }
            default:
                Assert::Fail(strutils::format(
                    L"Unsupported operation %s (%d)", 
                    strutils::to_wstring(variants::to_string(test.operation())).c_str(), static_cast<int>(test.operation())).c_str() );
            }
        }
    }

    TEST_METHOD(TestIntegerComparision)
    {
        typedef typename test_data<int, int, bool> test_data_t;
        test_data_set<int, int, bool> tests(L"IntCmp");
        tests.add(operation::cmp_eq, 0, 0, true);
        tests.add(operation::cmp_eq, 123, 123, true);
        tests.add(operation::cmp_eq, -123, -123, true);
        tests.add(operation::cmp_eq, numeric_limits<int>::max(), numeric_limits<int>::max(), true);
        tests.add(operation::cmp_eq, 123, 456, false);
        tests.add(operation::cmp_eq, -123, -456, false);
        tests.add(operation::cmp_eq, numeric_limits<int>::min(), numeric_limits<int>::max(), false);
        tests.add(operation::cmp_neq, 0, 1, true);
        tests.add(operation::cmp_neq, -1, 1, true);
        tests.add(operation::cmp_neq, -123, -321, true);
        tests.add(operation::cmp_neq, numeric_limits<int>::min(), numeric_limits<int>::max(), true);
        tests.add(operation::cmp_neq, 0, 0, false);
        tests.add(operation::cmp_neq, numeric_limits<int>::max(), numeric_limits<int>::max(), false);
        tests.add(operation::cmp_gt, 1, 0, true);
        tests.add(operation::cmp_gt, 0, -123, true);
        tests.add(operation::cmp_gt, numeric_limits<int>::max(), numeric_limits<int>::min(), true);
        RunTestsSet<int, int, bool>(tests);
    }

    TEST_METHOD(TestIntegerArithmetic)
    {
        typedef typename test_data<int, int, int> test_data_t;
        test_data_set<int, int, int> tests(L"IntMath");
        tests.add(operation::add, 123, 456, 579);
        tests.add(operation::add, numeric_limits<int>::min(), numeric_limits<int>::max(), -1);
        tests.add(operation::add, numeric_limits<int>::max(), numeric_limits<int>::min(), -1);
        RunTestsSet<int, int, int>(tests);
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

    TEST_METHOD(TestIntegersMemory)
    {
        auto sum1 = [=](variant n1, variant n2)->variant { return n1 + n2; };
        memchecker chk;
        {
            variant v11 = 123;
            variant v12 = v11;
            variant v13 = v11 + v12;
            int n1 = sum1(v11, v12);
            Assert::IsTrue(v13 == n1);
            variant v14 = std::move(v13);
            Assert::IsTrue(v14 == n1);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }
};