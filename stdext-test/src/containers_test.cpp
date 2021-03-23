#include <gtest/gtest.h>
#include "ptr_vector.h"
#include "strutils.h"
#include "testutils.h"

using namespace std;

namespace stdext
{
namespace containers_test
{

class test_data
{
public:
    test_data()
        : m_int_value(0), m_str_value(L"")
    { }
    test_data(const int int_value, const wstring str_value)
        : m_int_value(int_value), m_str_value(str_value)
    { }
    test_data(const test_data&) = default;
    test_data& operator = (const test_data&) = default;
    test_data(test_data&&) = default;
    test_data& operator = (test_data&&) = default;
    bool operator == (const test_data& rhs) const
    {
        return m_int_value == rhs.m_int_value && m_str_value == rhs.m_str_value;
    }
    std::wstring str_value() const { return m_str_value; }
    int int_value() const { return m_int_value; }
private:
    int     m_int_value;
    wstring m_str_value;
};

typedef ptr_vector<test_data> TestDataPtrVector;

class PtrVectorTest : public testing::Test
{
protected:
    void FillData(TestDataPtrVector& v)
    {
        v.clear();
        v.push_back(new test_data(1, L"Val 1"));
        v.push_back(new test_data(2, L"Val 2"));
        v.push_back(new test_data(3, L"Val 3"));
    }
};

TEST_F(PtrVectorTest, TestInsertEraseClear)
{
    TestDataPtrVector v;
    FillData(v);
    ASSERT_EQ(3u, v.size()) << L"init";
    int i = 1;
    for (TestDataPtrVector::iterator it = v.begin(); it != v.end(); it++)
    {
        ASSERT_EQ(i, (*it)->int_value()) << L"for iterator 1";
        ASSERT_EQ(L"Val " + std::to_wstring(i), (*it)->str_value()) << L"for iterator 2";
        i++;
    }
    i = 1;
    for (const test_data* data : v)
    {
        ASSERT_EQ(i, data->int_value()) << L"foreach 1";
        ASSERT_EQ(L"Val " + std::to_wstring(i), data->str_value()) << L"foreach 2";
        i++;
    }
    TestDataPtrVector::iterator it = v.begin();
    ASSERT_EQ(1, (*it)->int_value()) << L"begin";
    it = v.erase(v.begin());
    ASSERT_EQ(2u, v.size()) << L"erase 1";
    ASSERT_EQ(2, (*it)->int_value()) << L"erase 2";
    it++;
    ASSERT_EQ(3, (*it)->int_value()) << L"it++";
    v.clear();
    ASSERT_EQ(0u, v.size()) << L"clear";
}

TEST_F(PtrVectorTest, TestExtract)
{
    unique_ptr<test_data> t;
    {
        TestDataPtrVector v;
        FillData(v);
        TestDataPtrVector::size_type size = v.size();
        ASSERT_TRUE(v.size() > 0) << L"size 1";
        TestDataPtrVector::iterator it = v.begin();
        it++;
        t.reset(v.extract(it));
        size--;
        ASSERT_TRUE(v.size() == size) << L"size 2";
    }
    ASSERT_EQ(2, t->int_value()) << L"extracted value";
}

TEST_F(PtrVectorTest, TestExtractMemory)
{
    testutils::memchecker chk;
    {
        TestDataPtrVector v;
        FillData(v);
        TestDataPtrVector::iterator it = v.begin();
        it++;
        test_data* t = v.extract(it);
        delete t;
        ASSERT_TRUE(v.size() > 0);
    }
    chk.checkpoint();
    ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
}

TEST_F(PtrVectorTest, TestFind)
{
    TestDataPtrVector v1;
    FillData(v1);
    ASSERT_TRUE(v1.size() > 0) << L"init size";
    std::vector<test_data*> v2;
    for (test_data* value : v1)
        v2.push_back(value);
    ASSERT_EQ(v1.size(), v2.size()) << L"Size";
    for (test_data* value : v2)
    {
        ASSERT_TRUE(v1.find(value) != v1.end()) << L"Not found";
    }
}

TEST_F(PtrVectorTest, TestOperators)
{
    TestDataPtrVector v1, v2, v3;
    FillData(v1);
    FillData(v2);
    ASSERT_TRUE(v1.size() > 0 && v2.size() > 0) << "Size";
    EXPECT_TRUE(v1 == v2) << "Equal 1";
    EXPECT_TRUE(*v1[0] == *v2[0]) << "Equal 2";
    v3 = std::move(v2);
    EXPECT_EQ(v1.size(), v3.size()) << "move 1";
    EXPECT_EQ(0u, v2.size()) << "move 2";
    EXPECT_TRUE(v1 == v3) << "Equal 3.1";
    EXPECT_FALSE(v1 == v2) << "Equal 3.2";
}

TEST_F(PtrVectorTest, TestBackFront)
{
    TestDataPtrVector v1;
    test_data* d1 = new test_data(1, L"test front/back 1");
    v1.push_back(d1);
    EXPECT_EQ(d1, v1->front());
    EXPECT_EQ(d1, v1->back());
    EXPECT_EQ(d1->int_value(), v1->front()->int_value());
    EXPECT_EQ(d1->str_value(), v1->front()->str_value());
    test_data* d2 = new test_data(2, L"test front/back 2");
    v1.push_back(d2);
    EXPECT_EQ(d1, v1->front());
    EXPECT_EQ(d2, v1->back());
    EXPECT_EQ(d2->int_value(), v1->back()->int_value());
    EXPECT_EQ(d2->str_value(), v1->back()->str_value());
}

TEST_F(PtrVectorTest, TestMemory)
{
    {
        testutils::memchecker chk;
        {
            ptr_vector<test_data> v;
            FillData(v);
        }
        chk.checkpoint();
        ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
    }
    {
        testutils::memchecker chk;
        {
            ptr_vector<test_data> v;
            FillData(v);
            v.erase(v.begin());
        }
        chk.checkpoint();
        ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
    }
    {
        testutils::memchecker chk;
        {
            ptr_vector<test_data> v;
            FillData(v);
            v.clear();
        }
        chk.checkpoint();
        ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
    }
}

}
}
