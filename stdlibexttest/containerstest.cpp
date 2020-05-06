#include "CppUnitTest.h"
#include "ptr_vector.h"
#include "strutils.h"
#include "memchecker.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

class TestData
{
public:
    TestData()
        : m_int_value(0), m_str_value(L"")
    { }
    TestData(const int int_value, const wstring str_value)
        : m_int_value(int_value), m_str_value(str_value)
    { }
    TestData(const TestData& src) { *this = src; }
    const TestData& operator = (const TestData& src)
    {
        m_int_value = src.m_int_value;
        m_str_value = src.m_str_value;
        return *this;
    }
    bool operator == (const TestData& rhs) const
    {
        return m_int_value == rhs.m_int_value && m_str_value == rhs.m_str_value;
    }
    std::wstring str_value() const { return m_str_value; }
    int int_value() const { return m_int_value; };
private:
    int     m_int_value;
    wstring m_str_value;
};

typedef ptr_vector<TestData> TestDataPtrVector;

TEST_CLASS(PtrVectorTest)
{
private:
    void FillData(TestDataPtrVector& v)
    {
        v.clear();
        v.push_back(new TestData(1, L"Val 1"));
        v.push_back(new TestData(2, L"Val 2"));
        v.push_back(new TestData(3, L"Val 3"));
    }
public:
    TEST_METHOD(TestInsertEraseClear)
    {
        TestDataPtrVector v;
        FillData(v);
        Assert::AreEqual<int>(3, v.size(), L"init");
        int i = 1;
        for (TestDataPtrVector::iterator it = v.begin(); it != v.end(); it++)
        {
            Assert::AreEqual<int>(i, (*it)->int_value(), L"for iterator 1");
            Assert::AreEqual<wstring>(L"Val " + std::to_wstring(i), (*it)->str_value(), L"for iterator 2");
            i++;
        }
        i = 1;
        for (TestData* data : v)
        {
            Assert::AreEqual<int>(i, data->int_value(), L"foreach 1");
            Assert::AreEqual<wstring>(L"Val " + std::to_wstring(i), data->str_value(), L"foreach 2");
            i++;
        }
        TestDataPtrVector::iterator it = v.begin();
        Assert::AreEqual<int>(1, (*it)->int_value(), L"begin");
        it = v.erase(v.begin());
        Assert::AreEqual<int>(2, v.size(), L"erase 1");
        Assert::AreEqual<int>(2, (*it)->int_value(), L"erase 2");
        it++;
        Assert::AreEqual<int>(3, (*it)->int_value(), L"it++");
        v.clear();
        Assert::AreEqual<int>(0, v.size(), L"clear");
    }

    TEST_METHOD(TestExtract)
    {
        unique_ptr<TestData> t;
        {
            TestDataPtrVector v;
            FillData(v);
            TestDataPtrVector::size_type size = v.size();
            Assert::IsTrue(v.size() > 0, L"size 1");
            TestDataPtrVector::iterator it = v.begin();
            it++;
            t.reset(v.extract(it));
            size--;
            Assert::IsTrue(v.size() == size, L"size 2");
        }
        Assert::AreEqual<int>(2, t->int_value(), L"extracted value");
    }

    TEST_METHOD(TestExtractMemory)
    {
        memchecker chk;
        {
            TestDataPtrVector v;
            FillData(v);
            TestDataPtrVector::iterator it = v.begin();
            it++;
            TestData* t = v.extract(it);
            delete t;
            Assert::IsTrue(v.size() > 0);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }

    TEST_METHOD(TestFind)
    {
        TestDataPtrVector v1;
        FillData(v1);
        Assert::IsTrue(v1.size() > 0, L"init size");
        std::vector<TestData*> v2;
        for (TestData* value : v1)
            v2.push_back(value);
        Assert::AreEqual<int>(v1.size(), v2.size(), L"Size");
        for (TestData* value : v2)
        {
            Assert::IsTrue(v1.find(value) != v1.end(), L"Not found");
        }
    }

    TEST_METHOD(TestOperators)
    {
        TestDataPtrVector v1, v2, v3;
        FillData(v1);
        FillData(v2);   
        Assert::IsTrue(v1 == v2, L"Equal 1");
        Assert::IsTrue(*v1[0] == *v2[0], L"Equal 2");
        v3 = std::move(v2);
        Assert::AreEqual<int>(v1.size(), v3.size(), L"move 1");
        Assert::AreEqual<int>(0, v2.size(), L"move 2");
        Assert::IsTrue(v1 == v3, L"Equal 3.1");
        Assert::IsFalse(v1 == v2, L"Equal 3.2");
    }
    
    TEST_METHOD(TestMemory)
    {
        {
            memchecker chk;
            {
                ptr_vector<TestData> v;
                FillData(v);
            }
            chk.checkpoint();
            Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
        }
        {
            memchecker chk;
            {
                ptr_vector<TestData> v;
                FillData(v);
                v.erase(v.begin());
            }
            chk.checkpoint();
            Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
        }
        {
            memchecker chk;
            {
                ptr_vector<TestData> v;
                FillData(v);
                v.clear();
            }
            chk.checkpoint();
            Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
        }
    }
};