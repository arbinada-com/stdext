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

template<> inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<int64_t>(const int64_t& t) { RETURN_WIDE_STRING(t); }

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
    typedef test_data<Type1, Type2, TypeR> test_data_t;
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

#define DECLARE_TEST_LOCAL_VARS(Type1, Type2) variant v1 = test.v1();\
    variant v2 = test.v2(); \
    variant vresult = test.result(); \
    Type1 n1 = v1; \
    Type2 n2 = v2;
//
#define DECLARE_BIN_OP_TESTS(TypeR, op, opname) \
    variant v3 = v1 op v2; \
    variant v4 = n1 op v2; \
    variant v5 = v1 op n2; \
    Assert::AreEqual<TypeR>(test.result(), v1 op v2, make_title(test, L#opname"_1").c_str()); \
    Assert::AreEqual<TypeR>(test.result(), n1 op v2, make_title(test, L#opname"_2").c_str()); \
    Assert::AreEqual<TypeR>(test.result(), v1 op n2, make_title(test, L#opname"_3").c_str()); \
    Assert::AreEqual<TypeR>(test.result(), v3, make_title(test, L#opname"_4").c_str()); \
    Assert::AreEqual<TypeR>(test.result(), v4, make_title(test, L#opname"_5").c_str()); \
    Assert::AreEqual<TypeR>(test.result(), v5, make_title(test, L#opname"_6").c_str()); \
//
#define DECLARE_BIN_OP_FLOAT_TESTS(TypeR, op, opname) \
    variant v3 = v1 op v2; \
    variant v4 = n1 op v2; \
    variant v5 = v1 op n2; \
    Assert::IsTrue((test.result() - (v1 op v2)) < 1E-5, make_title(test, L#opname"_1").c_str()); \
    Assert::IsTrue((test.result() - (n1 op v2)) < 1E-5, make_title(test, L#opname"_2").c_str()); \
    Assert::IsTrue((test.result() - (v1 op n2)) < 1E-5, make_title(test, L#opname"_3").c_str()); \
    Assert::IsTrue((test.result() - v3) < 1E-5, make_title(test, L#opname"_4").c_str()); \
    Assert::IsTrue((test.result() - v4) < 1E-5, make_title(test, L#opname"_5").c_str()); \
    Assert::IsTrue((test.result() - v5) < 1E-5, make_title(test, L#opname"_6").c_str()); \
//

template <class Type1, class Type2, class TypeR>
class test_runner abstract
{
public:
    typedef test_data_set<Type1, Type2, TypeR> test_data_set_t;
    typedef typename test_data_set_t::test_data_t test_data_t;
public:
    test_runner(std::wstring title)
        : m_tests(test_data_set_t(title))
    { }
    test_runner(test_data_set_t& tests)
        : m_tests(tests)
    { }

    void add(const operation op, const Type1 v1, const Type2 v2, const TypeR result, std::wstring title = L"")
    {
        m_tests.add(op, v1, v2, result, title);
    }

    void run()
    {
        int count = 0;
        for (test_data_t test : m_tests)
        {
            count++;
            test_assignments(test);
            switch (test.operation())
            {
            case operation::cmp_eq:
                test_cmp_eq(test);
                break;
            case operation::cmp_neq:
                test_cmp_neq(test);
                break;
            case operation::cmp_gt:
                test_cmp_gt(test);
                break;
            case operation::cmp_ge:
                test_cmp_ge(test);
                break;
            case operation::cmp_lt:
                test_cmp_lt(test);
                break;
            case operation::cmp_le:
                test_cmp_le(test);
                break;
            case operation::add:
                test_add(test);
                break;
            case operation::subtract:
                test_subtract(test);
                break;
            case operation::multiply:
                test_multiply(test);
                break;
            case operation::divide:
                test_divide(test);
                break;
            case operation::b_and:
                test_b_and(test);
                break;
            case operation::b_or:
                test_b_or(test);
                break;
            case operation::b_not:
                test_b_not(test);
                break;
            default:
                Assert::Fail(strutils::format(
                    L"Unsupported operation %s (%d)",
                    strutils::to_wstring(variants::to_string(test.operation())).c_str(), static_cast<int>(test.operation())).c_str());
            }
        }
        Assert::IsTrue(count > 0, L"No tests performed");
    }
protected:
    std::wstring make_title(test_data_t& test, const wchar_t* msg)
    {
        return strutils::format(L"%s[%s] - %s", m_tests.title().c_str(), test.title().c_str(), msg);
    }
    virtual void test_assignments(test_data_t& test)
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::IsTrue(v1.vtype() == to_value_type(test.v1()), make_title(test, L"v1 type").c_str());
        Assert::IsTrue(v2.vtype() == to_value_type(test.v2()), make_title(test, L"v2 type").c_str());
        Assert::IsTrue(vresult.vtype() == to_value_type(test.result()), make_title(test, L"vresult type").c_str());
        Assert::AreEqual<Type1>(test.v1(), v1, make_title(test, L"v1 value").c_str());
        Assert::AreEqual<Type2>(test.v2(), v2, make_title(test, L"v2 value").c_str());
        Assert::AreEqual<TypeR>(test.result(), vresult, make_title(test, L"vresult value").c_str());
        Assert::AreEqual<Type1>(n1, v1, make_title(test, L"v1 value").c_str());
        Assert::AreEqual<Type2>(n2, v2, make_title(test, L"v2 value").c_str());
        variant v3 = std::move(v1);
        Assert::IsTrue(v3.vtype() == to_value_type(test.v1()), make_title(test, L"v3 type").c_str());
        Assert::AreEqual<Type1>(test.v1(), v3, make_title(test, L"v3 value").c_str());
    }
    virtual void test_cmp_eq(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_cmp_neq(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_cmp_gt(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_cmp_ge(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_cmp_lt(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_cmp_le(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_add(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_subtract(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_multiply(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_divide(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_b_and(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_b_or(test_data_t& test) { Assert::Fail(L"Not implemented"); }
    virtual void test_b_not(test_data_t& test) { Assert::Fail(L"Not implemented"); }

private:
    test_data_set_t m_tests;
};


template <class Type1, class Type2>
class test_runner_cmp : public test_runner<Type1, Type2, bool>
{
public:
    test_runner_cmp(std::wstring title)
        : test_runner(title)
    { }
protected:
    virtual void test_cmp_eq(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 == v2, make_title(test, L"EQ 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 == n2, make_title(test, L"EQ 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 == v2, make_title(test, L"EQ 3").c_str());
        Assert::AreEqual<bool>(test.result(), n1 == n2, make_title(test, L"EQ 4").c_str());
        Assert::AreEqual<bool>(test.result(), v1.equal(v2), make_title(test, L"EQ 5.1").c_str());
        Assert::AreEqual<bool>(test.result(), v2.equal(v1), make_title(test, L"EQ 5.2").c_str());
        Assert::AreEqual<bool>(test.result(), v1.equal(n2), make_title(test, L"EQ 5.3").c_str());
        Assert::AreEqual<bool>(test.result(), v2.equal(n1), make_title(test, L"EQ 5.4").c_str());
    }
    virtual void test_cmp_neq(test_data_t& test)
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 != v2, make_title(test, L"NEQ 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 != n2, make_title(test, L"NEQ 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 != v2, make_title(test, L"NEQ 3").c_str());
        Assert::AreEqual<bool>(test.result(), n1 != n2, make_title(test, L"NEQ 4").c_str());
    }
};

/*
 * test_runner for numeric types
 */
template <class Type1, class Type2>
class test_runner_cmp_num : public test_runner_cmp<Type1, Type2>
{
public:
    test_runner_cmp_num(std::wstring title)
        : test_runner_cmp(title)
    { }
protected:
    virtual void test_cmp_gt(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 > v2, make_title(test, L"GT 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 > n2, make_title(test, L"GT 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 > v2, make_title(test, L"GT 3").c_str());
    }
    virtual void test_cmp_ge(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 >= v2, make_title(test, L"GE 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 >= n2, make_title(test, L"GE 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 >= v2, make_title(test, L"GE 3").c_str());
    }
    virtual void test_cmp_lt(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 < v2, make_title(test, L"LT 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 < n2, make_title(test, L"LT 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 < v2, make_title(test, L"LT 3").c_str());
    }
    virtual void test_cmp_le(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 <= v2, make_title(test, L"LE 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 <= n2, make_title(test, L"LE 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 <= v2, make_title(test, L"LE 3").c_str());
    }
};

template <class Type1, class Type2, class TypeR>
class test_runner_int : public test_runner<Type1, Type2, TypeR>
{
public:
    test_runner_int(std::wstring title)
        : test_runner(title)
    { }
protected:
    virtual void test_add(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_TESTS(TypeR, +, ADD)
    }
    virtual void test_subtract(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_TESTS(TypeR, -, SUB)
    }
    virtual void test_multiply(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_TESTS(TypeR, *, MUL)
    }
    virtual void test_divide(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_TESTS(TypeR, / , DIV)
    }
};

template <class Type1, class Type2, class TypeR>
class test_runner_float : public test_runner<Type1, Type2, TypeR>
{
public:
    test_runner_float(std::wstring title)
        : test_runner(title)
    { }
protected:
    virtual void test_add(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_FLOAT_TESTS(TypeR, +, ADD)
    }
    virtual void test_subtract(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_FLOAT_TESTS(TypeR, -, SUB)
    }
    virtual void test_multiply(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_FLOAT_TESTS(TypeR, *, MUL)
    }
    virtual void test_divide(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_FLOAT_TESTS(TypeR, / , DIV)
    }
};

/*
 * test_runner_bool class
 */
class test_runner_cmp_bool : public test_runner_cmp<bool, bool>
{
public:
    test_runner_cmp_bool(std::wstring title)
        : test_runner_cmp(title)
    { }
protected:
};

class test_runner_bool : public test_runner<bool, bool, bool>
{
public:
    test_runner_bool(std::wstring title)
        : test_runner(title)
    { }
protected:
    virtual void test_b_and(test_data_t& test) override
    { 
        DECLARE_TEST_LOCAL_VARS(bool, bool)
        DECLARE_BIN_OP_TESTS(bool, &&, AND)
    }
    virtual void test_b_or(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(bool, bool)
        DECLARE_BIN_OP_TESTS(bool, ||, OR)
    }
    virtual void test_b_not(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(bool, bool)
        variant v3 = !v1;
        variant v4 = !n1;
        Assert::AreEqual<bool>(test.result(), !v1, make_title(test, L"NOT 1").c_str());
        Assert::AreEqual<bool>(test.result(), !n1, make_title(test, L"NOT 2").c_str());
        Assert::AreEqual<bool>(test.result(), v3, make_title(test, L"NOT 3").c_str());
        Assert::AreEqual<bool>(test.result(), v4, make_title(test, L"NOT 4").c_str());
    }
};

/*
 * test_runner for string types
 */
template <class Type1, class Type2>
class test_runner_cmp_string : public test_runner_cmp<Type1, Type2>
{
public:
    test_runner_cmp_string(std::wstring title)
        : test_runner_cmp(title)
    { }
protected:
    virtual void test_cmp_gt(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 > v2, make_title(test, L"GT 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 > n2, make_title(test, L"GT 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 > v2, make_title(test, L"GT 3").c_str());
    }
    virtual void test_cmp_ge(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 >= v2, make_title(test, L"GE 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 >= n2, make_title(test, L"GE 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 >= v2, make_title(test, L"GE 3").c_str());
    }
    virtual void test_cmp_lt(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 < v2, make_title(test, L"LT 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 < n2, make_title(test, L"LT 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 < v2, make_title(test, L"LT 3").c_str());
    }
    virtual void test_cmp_le(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        Assert::AreEqual<bool>(test.result(), v1 <= v2, make_title(test, L"LE 1").c_str());
        Assert::AreEqual<bool>(test.result(), v1 <= n2, make_title(test, L"LE 2").c_str());
        Assert::AreEqual<bool>(test.result(), n1 <= v2, make_title(test, L"LE 3").c_str());
    }
};

template <class Type1, class Type2, class TypeR>
class test_runner_string : public test_runner<Type1, Type2, TypeR>
{
public:
    test_runner_string(std::wstring title)
        : test_runner(title)
    { }
protected:
    virtual void test_add(test_data_t& test) override
    {
        DECLARE_TEST_LOCAL_VARS(Type1, Type2)
        DECLARE_BIN_OP_TESTS(TypeR, +, ADD)
        Assert::AreEqual<TypeR>(test.result(), v1 + v2, make_title(test, L"ADD 1").c_str());
        Assert::AreEqual<TypeR>(test.result(), v1 + n2, make_title(test, L"ADD 2").c_str());
        Assert::AreEqual<TypeR>(test.result(), n1 + v2, make_title(test, L"ADD 3").c_str());
    }
};


class testobjbase : public variants::objref
{
public:
    testobjbase(std::string value)
        : objref(), m_value(value)
    {}
    virtual std::string value() { return "Base:" + m_value; }
protected:
    std::string m_value;
};

class testobj : public testobjbase
{
public:
    testobj(std::string value)
        : testobjbase(value)
    {}
    virtual std::string value() override { return "Sub:" + m_value; }
};


/*
 * Tests class
 */
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

    TEST_METHOD(TestBool)
    {
        {
            test_runner_cmp_bool r(L"Cmp");
            r.add(operation::cmp_eq, true, true, true);
            r.add(operation::cmp_eq, false, false, true);
            r.add(operation::cmp_eq, true, false, false);
            r.add(operation::cmp_neq, true, false, true);
            r.add(operation::cmp_neq, true, true, false);
            r.add(operation::cmp_neq, false, false, false);
            r.run();
        }
        {
            test_runner_bool r(L"Ops");
            r.add(operation::b_and, true, false, false);
            r.add(operation::b_and, false, true, false);
            r.add(operation::b_and, false, false, false);
            r.add(operation::b_and, true, true, true);
            r.add(operation::b_or, true, false, true);
            r.add(operation::b_or, false, true, true);
            r.add(operation::b_or, false, false, false);
            r.add(operation::b_or, true, true, true);
            r.add(operation::b_not, true, /* not used */true, false);
            r.add(operation::b_not, false, true, true);
            r.run();
        }
    }


    template <class IntT1, class IntT2>
    void TestComparisionIntegerTypes(wstring test_set_title)
    {
        test_runner_cmp_num<IntT1, IntT2> r(test_set_title);
        r.add(operation::cmp_eq, 0, 0, true);
        r.add(operation::cmp_eq, 123, 123, true);
        r.add(operation::cmp_eq, -123, -123, true);
        r.add(operation::cmp_eq, 123, 456, false);
        r.add(operation::cmp_eq, -123, -456, false);
        r.add(operation::cmp_neq, 0, 1, true);
        r.add(operation::cmp_neq, -1, 1, true);
        r.add(operation::cmp_neq, -123, -321, true);
        r.add(operation::cmp_neq, 0, 0, false);
        r.add(operation::cmp_gt, 1, 0, true);
        r.add(operation::cmp_gt, 0, -123, true);
        r.add(operation::cmp_ge, 0, 0, true);
        r.add(operation::cmp_ge, 1, 0, true);
        r.add(operation::cmp_ge, 0, -1, true);
        r.add(operation::cmp_lt, 0, 1, true);
        r.add(operation::cmp_lt, -123, 0, true);
        r.add(operation::cmp_le, 0, 0, true);
        r.add(operation::cmp_le, 0, 1, true);
        r.add(operation::cmp_le, -1, 0, true);
        r.run();
    }

    template <class IntT1, class IntT2, class RetT>
    void TestArithmeticIntegerTypes(wstring test_set_title)
    {
        test_runner_int<IntT1, IntT2, RetT> r(test_set_title);
        r.add(operation::add, 0, 0, 0);
        r.add(operation::add, 123, 456, 579);
        r.add(operation::add, -123, 123, 0);
        r.add(operation::add, 123, -123, 0);
        r.add(operation::subtract, 0, 0, 0);
        r.add(operation::multiply, 0, 0, 0);
        r.add(operation::divide, 0, 1, 0);
        r.run();
    }

    TEST_METHOD(TestInt32)
    {
        TestComparisionIntegerTypes<int, int>(L"Int");
        {
            test_runner_cmp_num<int, int> r(L"CmpSpecific");
            r.add(operation::cmp_eq, numeric_limits<int>::max(), numeric_limits<int>::max(), true);
            r.add(operation::cmp_eq, numeric_limits<int>::min(), numeric_limits<int>::max(), false);
            r.add(operation::cmp_neq, numeric_limits<int>::min(), numeric_limits<int>::max(), true);
            r.add(operation::cmp_neq, numeric_limits<int>::max(), numeric_limits<int>::max(), false);
            r.add(operation::cmp_gt, numeric_limits<int>::max(), numeric_limits<int>::min(), true);
            r.add(operation::cmp_ge, numeric_limits<int>::max(), numeric_limits<int>::max(), true);
            r.add(operation::cmp_ge, numeric_limits<int>::max(), numeric_limits<int>::min(), true);
            r.add(operation::cmp_lt, numeric_limits<int>::min(), numeric_limits<int>::max(), true);
            r.add(operation::cmp_le, numeric_limits<int>::min(), numeric_limits<int>::min(), true);
            r.add(operation::cmp_le, numeric_limits<int>::min(), numeric_limits<int>::max(), true);
            r.run();
        }
        TestArithmeticIntegerTypes<int, int, int>(L"Ops");
        {
            test_runner_int<int, int, int> r(L"OpsSpecific");
            r.add(operation::add, numeric_limits<int>::min(), numeric_limits<int>::max(), -1);
            r.add(operation::add, numeric_limits<int>::max(), numeric_limits<int>::min(), -1);
            r.add(operation::subtract, numeric_limits<int>::max(), numeric_limits<int>::max(), 0);
            r.add(operation::subtract, numeric_limits<int>::min(), numeric_limits<int>::min(), 0);
            r.add(operation::divide, numeric_limits<int>::max(), numeric_limits<int>::max(), 1);
            r.run();
        }
    }
    TEST_METHOD(TestInt64)
    {
        TestComparisionIntegerTypes<int64_t, int64_t>(L"Cmp1");
        TestComparisionIntegerTypes<int64_t, int>(L"Cmp2");
        TestComparisionIntegerTypes<int, int64_t>(L"Cmp3");
        {
            test_runner_cmp_num<int64_t, int64_t> r(L"CmpSpecific");
            r.add(operation::cmp_eq, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::max(), true);
            r.add(operation::cmp_eq, numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), false);
            r.add(operation::cmp_neq, numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), true);
            r.add(operation::cmp_neq, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::max(), false);
            r.add(operation::cmp_gt, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::min(), true);
            r.add(operation::cmp_ge, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::max(), true);
            r.add(operation::cmp_ge, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::min(), true);
            r.add(operation::cmp_lt, numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), true);
            r.add(operation::cmp_le, numeric_limits<int64_t>::min(), numeric_limits<int64_t>::min(), true);
            r.add(operation::cmp_le, numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), true);
            r.run();
        }
        TestArithmeticIntegerTypes<int64_t, int64_t, int64_t>(L"Ops1");
        {
            test_runner_int<int64_t, int64_t, int64_t> r(L"Ops1_Specific");
            r.add(operation::add, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::min(), -1);
            r.add(operation::add, numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), -1);
            r.add(operation::subtract, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::max(), 0);
            r.add(operation::subtract, numeric_limits<int64_t>::min(), numeric_limits<int64_t>::min(), 0);
            r.add(operation::divide, numeric_limits<int64_t>::max(), numeric_limits<int64_t>::max(), 1);
            r.run();
        }
        TestArithmeticIntegerTypes<int, int64_t, int64_t>(L"Ops2");
        {
            test_runner_int<int, int64_t, int64_t> r(L"Ops2_Specific");
            r.add(operation::add, numeric_limits<int>::max(), numeric_limits<int>::max(), 4294967294L);
            r.add(operation::add, numeric_limits<int>::min(), numeric_limits<int>::min(), -4294967296L);
            r.add(operation::add, numeric_limits<int>::min(), numeric_limits<int64_t>::max(), 9223372034707292159L);
            r.run();
        }
        TestArithmeticIntegerTypes<int64_t, int, int64_t>(L"Int64_Int");
    }

    TEST_METHOD(TestDouble)
    {
        TestComparisionIntegerTypes<double, double>(L"Cmp1");
        TestComparisionIntegerTypes<double, int>(L"Cmp2");
        TestComparisionIntegerTypes<int, double>(L"Cmp3");
        TestComparisionIntegerTypes<int64_t, double>(L"Cmp4");
        TestComparisionIntegerTypes<double, int64_t>(L"Cmp5");
        {
            test_runner_cmp_num<double, double> r(L"CmpSpecific");
            r.add(operation::cmp_eq, 123.456, 123.456, true);
            r.add(operation::cmp_eq, -123.456, -123.456, true);
            r.add(operation::cmp_eq, 1.23456E3, 1.23456E3, true);
            r.add(operation::cmp_eq, numeric_limits<double>::max(), numeric_limits<double>::max(), true);
            r.add(operation::cmp_eq, numeric_limits<double>::min(), numeric_limits<double>::max(), false);
            r.add(operation::cmp_neq, 123.456, 123.457, true);
            r.add(operation::cmp_neq, numeric_limits<double>::min(), numeric_limits<double>::max(), true);
            r.add(operation::cmp_neq, numeric_limits<double>::max(), numeric_limits<double>::max(), false);
            r.add(operation::cmp_gt, 123.456, 123.4559, true);
            r.add(operation::cmp_gt, numeric_limits<double>::max(), numeric_limits<double>::min(), true);
            r.add(operation::cmp_ge, 123.456, 123.456, true);
            r.add(operation::cmp_ge, 123.4561, 123.456, true);
            r.add(operation::cmp_ge, numeric_limits<double>::max(), numeric_limits<double>::max(), true);
            r.add(operation::cmp_ge, numeric_limits<double>::max(), numeric_limits<double>::min(), true);
            r.add(operation::cmp_lt, 123.456, 123.4561, true);
            r.add(operation::cmp_lt, numeric_limits<double>::min(), numeric_limits<double>::max(), true);
            r.add(operation::cmp_le, 123.456, 123.456, true);
            r.add(operation::cmp_le, 123.456, 123.4561, true);
            r.add(operation::cmp_le, numeric_limits<double>::min(), numeric_limits<double>::min(), true);
            r.add(operation::cmp_le, numeric_limits<double>::min(), numeric_limits<double>::max(), true);
            r.run();
        }
        TestArithmeticIntegerTypes<double, double, double>(L"Ops1");
        TestArithmeticIntegerTypes<int, double, double>(L"Ops2");
        TestArithmeticIntegerTypes<int64_t, double, double>(L"Ops3");
        TestArithmeticIntegerTypes<double, int, double>(L"Ops4");
        TestArithmeticIntegerTypes<double, int64_t, double>(L"Ops5");
        {
            test_runner_float<double, double, double> r(L"OpsSpecific1");
            r.add(operation::add, 123.456, -123.456, 0.0);
            r.add(operation::add, 123.456, 0.123456E3, 246.912);
            r.add(operation::subtract, 123.456, 123.456, 0.0);
            r.add(operation::subtract, -123.456, 0.123456E3, -246.912);
            r.add(operation::multiply, 123.456, 789.012, 97408.265472);
            r.add(operation::divide, 97408.265472, 123.456, 789.012);
            r.run();
        }
        {
            test_runner_float<int, double, double> r(L"OpsSpecific2");
            r.add(operation::add, 123, -123.456, -0.456);
            r.add(operation::subtract, 123, 123.456, -0.456);
            r.add(operation::multiply, 8, 1.25, 10.0);
            r.add(operation::divide, 10, 8.0, 1.25);
            r.run();
        }
        {
            test_runner_float<int64_t, double, double> r(L"OpsSpecific3");
            r.add(operation::add, 123, -123.456, -0.456);
            r.add(operation::subtract, 123, 123.456, -0.456);
            r.add(operation::multiply, 8, 1.25, 10.0);
            r.add(operation::divide, 10, 8.0, 1.25);
            r.run();
        }
    }

    TEST_METHOD(TestString)
    {
        {
            test_runner_cmp_string<std::string, std::string> r(L"Cmp");
            r.add(operation::cmp_eq, "", "", true);
            r.add(operation::cmp_eq, "ABC abc", "ABC abc", true);
            r.add(operation::cmp_eq, "Abc", "Abcd", false);
            r.add(operation::cmp_neq, "Abc", "", true);
            r.add(operation::cmp_neq, "ABC abc", "ABC abc", false);
            r.add(operation::cmp_ge, "abc", "ABC", true);
            r.add(operation::cmp_le, "Abc", "abc", true);
            r.run();
        }
        {
            test_runner_string<std::string, std::string, std::string> r(L"Ops");
            r.add(operation::add, "", "", "");
            r.run();
        }
    }

    TEST_METHOD(TestWString)
    {
        {
            test_runner_cmp_string<std::wstring, std::wstring> r(L"Cmp");
            r.add(operation::cmp_eq, L"", L"", true);
            r.add(operation::cmp_eq, L"ABC abc", L"ABC abc", true);
            r.add(operation::cmp_eq, L"Abc", L"Abcd", false);
            r.add(operation::cmp_neq, L"Abc", L"", true);
            r.add(operation::cmp_neq, L"ABC abc", L"ABC abc", false);
            r.add(operation::cmp_ge, L"abc", L"ABC", true);
            r.add(operation::cmp_le, L"Abc", L"abc", true);
            r.run();
        }
    }


    TEST_METHOD(TestObjref)
    {
        variant v1 = new testobjbase("Obj1");
        Assert::IsTrue(v1.vtype() == value_type::vt_object, L"Type 1");
        variant v2 = v1;
        Assert::IsTrue(v2.vtype() == value_type::vt_object, L"Type 2");
        Assert::AreEqual<std::string>("Base:Obj1", v1.to_object<testobjbase>()->value(), L"Value 1");
        Assert::AreEqual<std::string>("Base:Obj1", v2.to_object<testobjbase>()->value(), L"Value 2");
        variant v3 = new testobj("Obj2");
        Assert::IsTrue(v3.vtype() == value_type::vt_object, L"Type 3");
        Assert::AreEqual<std::string>("Sub:Obj2", v3.to_object<testobj>()->value(), L"Value 3.1");
        Assert::AreEqual<std::string>("Sub:Obj2", v3.to_object<testobjbase>()->value(), L"Value 3.2");
        variant v4 = std::move(v3);
        Assert::IsTrue(v4.vtype() == value_type::vt_object, L"Type 4");
        Assert::AreEqual<std::string>("Sub:Obj2", v4.to_object<testobj>()->value(), L"Value 4.1");
        Assert::AreEqual<std::string>("Sub:Obj2", v4.to_object<testobjbase>()->value(), L"Value 4.2");
    }

    /* 
     * Memory checks
     */
    TEST_METHOD(TestBoolMemory)
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

    template <class IntT>
    void TestMemoryIntType()
    {
        auto sum1 = [=](variant n1, variant n2)->variant { return n1 + n2; };
        memchecker chk;
        {
            variant v11 = static_cast<IntT>(123);
            Assert::IsTrue(v11.vtype() == variants::to_value_type(static_cast<IntT>(0)), L"Int type");
            variant v12 = v11;
            variant v13 = v11 + v12;
            IntT n1 = sum1(v11, v12);
            Assert::IsTrue(v13 == n1);
            variant v14 = std::move(v13);
            Assert::IsTrue(v14 == n1);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }
    TEST_METHOD(TestInt32Memory)
    {
        TestMemoryIntType<int>();
    }
    TEST_METHOD(TestInt64Memory)
    {
        TestMemoryIntType<int64_t>();
    }
    TEST_METHOD(TestDoubleMemory)
    {
        TestMemoryIntType<double>();
    }

    TEST_METHOD(TestStringMemory)
    {
        auto func1 = [](variant n1, variant n2)->variant { return n1 + n2; };
        memchecker chk;
        {
            variant v11 = "ABC def";
            variant v12 = v11;
            variant v13 = v11 + v12;
            string b1 = func1(v11, v12);
            Assert::IsTrue(v13 == b1);
            variant v14 = std::move(v13);
            Assert::IsTrue(v14 == b1);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }

    TEST_METHOD(TestObjrefMemory)
    {
        auto func1 = [](variant n1, variant n2)->variant { return n1.to_object<testobj>()->value() + n2.to_object<testobj>()->value(); };
        memchecker chk;
        {
            variant v11 = new testobj("Obj1");
            variant v12 = v11;
            string b1 = func1(v11, v12);
            Assert::IsTrue("Sub:Obj1Sub:Obj1" == b1);
            variant v13 = new testobj("Obj1");
            variant v14 = std::move(v13);
            Assert::IsTrue(v14.vtype() == value_type::vt_object);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }

};