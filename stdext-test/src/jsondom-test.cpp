#include <gtest/gtest.h>
#include "json.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include "locutils.h"
#include "strutils.h"
#include "testutils.h"


using namespace std;
using namespace stdext;
using namespace testutils;

namespace jsondom_test
{

class JsonDomTest : public testing::Test
{
protected:
    void FillTestDoc(json::dom_document& doc)
    {
        json::dom_array* a1 = doc.create_array();
        doc.root(a1);
        a1->append(doc.create_string(L"Hello"));
        a1->append(doc.create_literal(L"null"));
        json::dom_object* o1 = doc.create_object();
        a1->append(o1);
        o1->append_member(L"Str 1", doc.create_string(L"World"));
        o1->append_member(L"Num 1", doc.create_number(123));
        json::dom_array* a2 = doc.create_array();
        o1->append_member(L"Arr 1", a2);
        o1->append_member(L"Literal 1", doc.create_literal(L"false"));
        json::dom_array* a3 = doc.create_array();
        o1->append_member(L"Arr 2", a3);
        a3->append(doc.create_number(456.78));
    }

    void CheckTestDocValue(wstring path, json::dom_value* v1)
    {
        wstring title = path + L": ";
        if (path == L"0")
        {
            ASSERT_TRUE(nullptr != dynamic_cast<json::dom_array*>(v1)) << title + L"cast 1";
            json::container_intf* cont = dynamic_cast<json::container_intf*>(v1);
            ASSERT_TRUE(cont != nullptr) << title + L"cast 2";
            EXPECT_EQ(3u, cont->count()) << title + L"count";
        }
        else if (path == L"0.0")
        {
            ASSERT_TRUE(json::dom_value_type::vt_string == v1->type()) << title + L"type";
            ASSERT_EQ(L"Hello", dynamic_cast<json::dom_string*>(v1)->text()) << title + L"text";
        }
        else if (path == L"0.1")
        {
            ASSERT_TRUE(json::dom_value_type::vt_literal == v1->type()) << title + L"type 1.1";
            ASSERT_TRUE(json::dom_literal_value_type::lvt_null == dynamic_cast<json::dom_literal*>(v1)->subtype()) << title + L"subtype 1.1";
            ASSERT_TRUE(nullptr == dynamic_cast<json::dom_object*>(v1)) << title + L"cast 1";
            ASSERT_TRUE(nullptr == dynamic_cast<json::container_intf*>(v1)) << title + L"cast 2";
        }
        else if (path == L"0.2")
        {
            ASSERT_TRUE(nullptr != dynamic_cast<json::dom_object*>(v1)) << title + L"cast 1";
            json::container_intf* cont = dynamic_cast<json::container_intf*>(v1);
            ASSERT_TRUE(cont != nullptr) << title + L"cast 2";
            EXPECT_EQ(5u, cont->count()) << title + L"count";
        }
        else if (path == L"0.2.0")
        {
            ASSERT_TRUE(json::dom_value_type::vt_string == v1->type()) << title + L"type";
            ASSERT_EQ(L"World", dynamic_cast<json::dom_string*>(v1)->text()) << title + L"text";
            ASSERT_TRUE(v1->member() != nullptr) << title + L"member";
            ASSERT_EQ(L"Str 1", v1->member()->name()) << title + L"member name";
        }
        else if (path == L"0.2.1")
        {
            ASSERT_TRUE(json::dom_value_type::vt_number == v1->type()) << title + L"type";
            ASSERT_TRUE(json::dom_number_value_type::nvt_int == dynamic_cast<json::dom_number*>(v1)->subtype()) << title + L"subtype";
            ASSERT_EQ(L"123", dynamic_cast<json::dom_number*>(v1)->text()) << title + L"text";
            ASSERT_TRUE(v1->member() != nullptr) << title + L"member";
            ASSERT_EQ(L"Num 1", v1->member()->name()) << title + L"member name";
        }
        else if (path == L"0.2.2")
        {
            ASSERT_TRUE(nullptr != dynamic_cast<json::dom_array*>(v1)) << title + L"cast 1";
            json::container_intf* cont = dynamic_cast<json::container_intf*>(v1);
            ASSERT_TRUE(cont != nullptr) << title + L"cast 2";
            EXPECT_EQ(0u, cont->count()) << title + L"count";
            ASSERT_TRUE(v1->member() != nullptr) << title + L"member";
            EXPECT_EQ(L"Arr 1", v1->member()->name()) << title + L"member name";
        }
        else if (path == L"0.2.3")
        {
            EXPECT_TRUE(json::dom_value_type::vt_literal == v1->type()) << title + L"type";
            EXPECT_TRUE(json::dom_literal_value_type::lvt_false == dynamic_cast<json::dom_literal*>(v1)->subtype()) << title + L"subtype";
            EXPECT_EQ(L"false", dynamic_cast<json::dom_literal*>(v1)->text()) << title + L"text";
            ASSERT_TRUE(v1->member() != nullptr) << title + L"member";
            EXPECT_EQ(L"Literal 1", v1->member()->name()) << title + L"member name";
        }
        else if (path == L"0.2.4")
        {
            ASSERT_TRUE(nullptr != dynamic_cast<json::dom_array*>(v1)) << title + L"cast 1";
            json::container_intf* cont = dynamic_cast<json::container_intf*>(v1);
            ASSERT_TRUE(cont != nullptr) << title + L"cast 2";
            EXPECT_EQ(1u, cont->count()) << title + L"count";
            ASSERT_TRUE(v1->member() != nullptr) << title + L"member";
            EXPECT_EQ(L"Arr 2", v1->member()->name()) << title + L"member name";
        }
        else if (path == L"0.2.4.0")
        {
            EXPECT_TRUE(json::dom_value_type::vt_number == v1->type()) << title + L"type";
            EXPECT_TRUE(json::dom_number_value_type::nvt_float == dynamic_cast<json::dom_number*>(v1)->subtype()) << title + L"subtype";
            EXPECT_EQ(L"456.78", dynamic_cast<json::dom_number*>(v1)->text()) << title + L"text";
        }
        else
            FAIL() << title + L"unsupported path";
    }

};

TEST_F(JsonDomTest, TestValueTypeNames)
{
    for (json::dom_value_type vtype = json::first_value_type();
         vtype != json::last_value_type();
         vtype = (json::dom_value_type)(static_cast<int>(vtype) + 1))
    {
        string s = json::to_string(vtype);
        ASSERT_NE(s, "unsupported") << strutils::wformat(L"Undefined name for value_type %d", static_cast<int>(vtype));
    }
}

TEST_F(JsonDomTest, TestDomValues_Literal)
{
    json::dom_document doc;
    json::dom_literal* v11 = doc.create_literal(L"true");
    ASSERT_EQ(L"true", v11->text()) << L"v11 text";
    ASSERT_TRUE(json::dom_value_type::vt_literal == v11->type()) << L"v11 type";
    ASSERT_TRUE(json::dom_literal_value_type::lvt_true == v11->subtype()) << L"v11 subtype";
    json::dom_literal* v12 = doc.create_literal(L"false");
    ASSERT_EQ(L"false", v12->text()) << L"v12 text";
    ASSERT_TRUE(json::dom_value_type::vt_literal == v12->type()) << L"v12 type";
    ASSERT_TRUE(json::dom_literal_value_type::lvt_false == v12->subtype()) << L"v12 subtype";
    json::dom_literal* v2 = doc.create_literal(L"null");
    ASSERT_EQ(L"null", v2->text()) << L"v2 text";
    ASSERT_TRUE(json::dom_value_type::vt_literal == v2->type()) << L"v2 type";
    ASSERT_TRUE(json::dom_literal_value_type::lvt_null == v2->subtype()) << L"v2 subtype";
}

TEST_F(JsonDomTest, TestDomValues_Number)
{
    auto test_number = [](json::dom_number* value, const wstring text, const json::dom_number_value_type subtype, wstring title)
    {
        title += L": ";
        ASSERT_EQ(text, value->text()) << title + L"text";
        ASSERT_TRUE(json::dom_value_type::vt_number == value->type()) << title + L"type";
        ASSERT_TRUE(subtype == value->subtype()) << title + L"subtype";
        delete value;
    };
    json::dom_document doc;
    test_number(doc.create_number(L"123456", json::dom_number_value_type::nvt_int),
                L"123456", json::dom_number_value_type::nvt_int, L"Int 1");
    test_number(doc.create_number(123456),
                L"123456", json::dom_number_value_type::nvt_int, L"Int 2");
    test_number(doc.create_number(std::numeric_limits<int64_t>::max()),
                L"9223372036854775807", json::dom_number_value_type::nvt_int, L"Int 3");
    test_number(doc.create_number(L"123.456", json::dom_number_value_type::nvt_float),
                L"123.456", json::dom_number_value_type::nvt_float, L"Decimal 1");
    test_number(doc.create_number(123.456),
                L"123.456", json::dom_number_value_type::nvt_float, L"Float 1.1");
    {
        locutils::locale_guard loc(LC_NUMERIC, "ru_RU");
        test_number(doc.create_number(123.456), // 123.456 -> "123,456" on ru_RU
                    L"123.456", json::dom_number_value_type::nvt_float, L"Float 1.2");
    }
    test_number(doc.create_number(123.456),
                L"123.456", json::dom_number_value_type::nvt_float, L"Float 1.3");
    test_number(doc.create_number(123.456e10),
                L"1.23456e+12", json::dom_number_value_type::nvt_float, L"Float 2.1");
    test_number(doc.create_number(0.123456e-10),
                L"1.23456e-11", json::dom_number_value_type::nvt_float, L"Float 2.2");
    test_number(doc.create_number(1.23456e8),
                L"1.23456e+08", json::dom_number_value_type::nvt_float, L"Float 2.3");
    test_number(doc.create_number(1.23456e-8),
                L"1.23456e-08", json::dom_number_value_type::nvt_float, L"Float 2.4");
}

TEST_F(JsonDomTest, TestDomValues_Other)
{
    json::dom_document doc;
    //
    json::dom_string* v4 = doc.create_string(L"Hello DOM");
    EXPECT_EQ(L"Hello DOM", v4->text()) << L"v4 text";
    EXPECT_TRUE(json::dom_value_type::vt_string == v4->type()) << L"v4 type";
    //
    json::dom_array* v5 = doc.create_array();
    EXPECT_EQ(L"", v5->text()) << L"v5 text";
    EXPECT_EQ(0u, v5->size()) << L"v5 size";
    EXPECT_TRUE(json::dom_value_type::vt_array == v5->type()) << L"v5 type";
    //
    json::dom_object* v6 = doc.create_object();
    EXPECT_EQ(L"", v6->text()) << L"v6 text";
    EXPECT_EQ(0u, v6->members()->size()) << L"v6 values size";
    EXPECT_TRUE(json::dom_value_type::vt_object == v6->type()) << L"v6 type";
}

TEST_F(JsonDomTest, TestDomMembers)
{
    auto make_name = [](const size_t index)->wstring { return L"Value " + std::to_wstring(index); };
    json::dom_document doc;
    std::vector<json::dom_value*> test_data;
    test_data.push_back(doc.create_literal(L"true"));
    test_data.push_back(doc.create_literal(L"false"));
    test_data.push_back(doc.create_number(123));
    test_data.push_back(doc.create_number(std::numeric_limits<int64_t>::max()));
    test_data.push_back(doc.create_number(L"123.456", json::dom_number_value_type::nvt_float));
    test_data.push_back(doc.create_string(L"String"));

    unique_ptr<json::dom_object> o1(doc.create_object());
    int i = 1;
    for (json::dom_value* v : test_data)
    {
        o1->members()->append(make_name(i), v);
        i++;
    }
    ASSERT_EQ(test_data.size(), o1->members()->size()) << L"Size 1";
    for (size_t i = 0; i < test_data.size(); i++)
    {
        json::dom_object_member::name_t name = make_name(i + 1);
        ASSERT_EQ(name, o1->members()->at(i)->name()) << strutils::wformat(L"Name %d", i);
        ASSERT_TRUE(test_data[i] == (*o1)[i]->value()) << strutils::wformat(L"Value ptr %d", i);
        ASSERT_TRUE(test_data[i]->type() == (*o1)[i]->value()->type()) << strutils::wformat(L"Value type %d", i);
        ASSERT_TRUE(test_data[i]->text() == (*o1)[i]->value()->text()) << strutils::wformat(L"Value %d", i);
        json::dom_value* v = o1->find(name);
        ASSERT_TRUE(o1->contains_member(name)) << strutils::wformat(L"Value2 name %d", i);
        ASSERT_FALSE(v == nullptr) << strutils::wformat(L"Value2 %d", i);
        ASSERT_EQ(test_data[i], v) << strutils::wformat(L"Value2 ptr %d", i);
        ASSERT_EQ(test_data[i]->type(), v->type()) << strutils::wformat(L"Value2 type %d", i);
        ASSERT_EQ(test_data[i]->text(), v->text()) << strutils::wformat(L"Value2 %d", i);
    }
}

TEST_F(JsonDomTest, TestDomContainers)
{
    json::dom_document doc;
    FillTestDoc(doc);
    //
    CheckTestDocValue(L"0", doc.root());
    json::container_intf* cont1 = dynamic_cast<json::container_intf*>(doc.root());
    ASSERT_NE(cont1, nullptr);
    json::dom_value* v1 = cont1->get_value(0);
    CheckTestDocValue(L"0.0", v1);
    v1 = cont1->get_value(1);
    CheckTestDocValue(L"0.1", v1);
    v1 = cont1->get_value(2);
    CheckTestDocValue(L"0.2", v1);
    //
    json::container_intf* cont2 = dynamic_cast<json::container_intf*>(v1);
    ASSERT_NE(cont2, nullptr);
    v1 = cont2->get_value(0);
    CheckTestDocValue(L"0.2.0", v1);
    v1 = cont2->get_value(1);
    CheckTestDocValue(L"0.2.1", v1);
    v1 = cont2->get_value(2);
    CheckTestDocValue(L"0.2.2", v1);
    json::container_intf* cont3 = dynamic_cast<json::container_intf*>(v1);
    ASSERT_NE(cont3, nullptr);
    EXPECT_EQ(cont3->count(), 0u);
    v1 = cont2->get_value(3);
    CheckTestDocValue(L"0.2.3", v1);
    v1 = cont2->get_value(4);
    CheckTestDocValue(L"0.2.4", v1);
    json::container_intf* cont4 = dynamic_cast<json::container_intf*>(v1);
    ASSERT_NE(cont4, nullptr);
    v1 = cont4->get_value(0);
    CheckTestDocValue(L"0.2.4.0", v1);
}

TEST_F(JsonDomTest, TestDomDocument)
{
    {
        json::dom_document doc;
        ASSERT_EQ(doc.root(), nullptr) << L"Root 1";
        doc.root(doc.create_string(L"Hello world!"));
        ASSERT_NE(doc.root(), nullptr) << L"Root 2";
        EXPECT_EQ(L"Hello world!", doc.root()->text()) << L"Root text";
    }
}

TEST_F(JsonDomTest, TestDomDocumentIterator)
{
    json::dom_document doc;
    json::dom_document::iterator it = doc.begin();
    EXPECT_TRUE(it.value() == nullptr) << L"Curent 1.1";
    EXPECT_TRUE(*it == nullptr) << L"Curent 1.2";
    EXPECT_TRUE(it == doc.end()) << L"Curent 1.3";
    EXPECT_EQ(0u, it.level()) << L"it.level 1";
    FillTestDoc(doc);
    it = doc.begin();
    ASSERT_TRUE(it.value() == doc.root()) << L"Curent 2.1";
    EXPECT_TRUE(*it == doc.root()) << L"Curent 2.2";
    EXPECT_TRUE(*it == it.value()) << L"Curent 2.2";
    EXPECT_TRUE(it != doc.end()) << L"Curent 2.4";
    EXPECT_EQ(1u, it.level()) << L"it.level 2";
    //
    json::dom_document::iterator::path_t path = { 0 };
    EXPECT_TRUE(path == it.path()) << L"path 0";
    EXPECT_FALSE(it.has_prev_sibling()) << L"has_prev_sibling 0";
    CheckTestDocValue(L"0", *it);
    it++;
    path = { 0, 0 };
    EXPECT_TRUE(path == it.path()) << L"path 0.0";
    EXPECT_FALSE(it.has_prev_sibling()) << L"has_prev_sibling 0.0";
    CheckTestDocValue(L"0.0", *it);
    it++;
    path = { 0, 1 };
    EXPECT_TRUE(path == it.path()) << L"path 0.1";
    EXPECT_TRUE(it.has_prev_sibling()) << L"has_prev_sibling 0.1";
    CheckTestDocValue(L"0.1", *it);
    it++;
    path = { 0, 2 };
    EXPECT_TRUE(path == it.path()) << L"path 0.2";
    CheckTestDocValue(L"0.2", *it);
    it++;
    path = { 0, 2, 0 };
    EXPECT_TRUE(path == it.path()) << L"path 0.2.0";
    CheckTestDocValue(L"0.2.0", *it);
    it++;
    path = { 0, 2, 1 };
    EXPECT_TRUE(path == it.path()) << L"path 0.2.1";
    CheckTestDocValue(L"0.2.1", *it);
    it++;
    path = { 0, 2, 2 };
    EXPECT_TRUE(path == it.path()) << L"path 0.2.2";
    CheckTestDocValue(L"0.2.2", *it);
    it++;
    path = { 0, 2, 3 };
    EXPECT_TRUE(path == it.path()) << L"path 0.2.3";
    CheckTestDocValue(L"0.2.3", *it);
    it++;
    path = { 0, 2, 4 };
    EXPECT_TRUE(path == it.path()) << L"path 0.2.4";
    EXPECT_TRUE(it.has_prev_sibling()) << L"has_prev_sibling 0.2.4";
    CheckTestDocValue(L"0.2.4", *it);
    it++;
    path = { 0, 2, 4, 0 };
    EXPECT_TRUE(path == it.path()) << L"path 0.2.4.0";
    EXPECT_FALSE(it.has_prev_sibling()) << L"has_prev_sibling 0.2.4.0";
    CheckTestDocValue(L"0.2.4.0", *it);
    it++;
    EXPECT_TRUE(it == doc.end()) << L"End 1";
    EXPECT_EQ(0u, it.level()) << L"End 2";
    //
    int count = 0;
    for (const auto& value : doc) // const auto -> const json::dom_value*
    {
        EXPECT_EQ(value->document(), &doc);
        count++;
    }
    EXPECT_EQ(10, count) << L"for const auto";
    count = 0;
    for (auto value : doc) // auto -> json::dom_value*
    {
        EXPECT_EQ(value->document(), &doc);
        count++;
    }
    EXPECT_EQ(10, count) << L"for auto";
    count = 0;
    for (json::dom_value* value : doc)
    {
        EXPECT_EQ(value->document(), &doc);
        count++;
    }
    EXPECT_EQ(10, count) << L"for json::dom_value*";
}

TEST_F(JsonDomTest, TestDomDocumentComparison)
{
    json::dom_document doc1;
    json::dom_document doc2;
    ASSERT_TRUE(json::equal(doc1, doc2)) << L"Cmp 1";
    FillTestDoc(doc1);
    ASSERT_FALSE(json::equal(doc1, doc2)) << L"Cmp 2";
    FillTestDoc(doc2);
    ASSERT_TRUE(json::equal(doc1, doc2)) << L"Cmp 3";
    doc1.clear();
    ASSERT_FALSE(json::equal(doc1, doc2)) << L"Cmp 4";
    //
    doc1.clear();
    doc2.clear();
    json::dom_array* a1 = doc1.create_array();
    doc1.root(a1);
    a1->append(doc1.create_array());
    a1->append(doc1.create_array());
    a1->append(doc1.create_array());
    a1 = doc2.create_array();
    doc2.root(a1);
    json::dom_array* a11 = doc2.create_array();
    a1->append(a11);
    json::dom_array* a111 = doc2.create_array();
    a11->append(a111);
    json::dom_array* a1111 = doc2.create_array();
    a111->append(a1111);
    ASSERT_FALSE(json::equal(doc1, doc2)) << L"Cmp 5";
    //
    doc1.clear();
    doc2.clear();
    doc1.root(doc1.create_string(L"Str1"));
    doc2.root(doc2.create_string(L"Str1"));
    ASSERT_TRUE(json::equal(doc1, doc2)) << L"Cmp 6";
}

TEST_F(JsonDomTest, TestDomDocumentMemory)
{
    memchecker chk;
    {
        json::dom_document doc;
        doc.root(doc.create_string(L"Hello world!"));
        ASSERT_NE(doc.root(), nullptr);
        json::dom_array* a1 = doc.create_array();
        doc.root(a1);
        a1->append(doc.create_literal(L"null"));
        a1->append(doc.create_number(L"123.456", json::dom_number_value_type::nvt_float));
        a1->append(doc.create_string(L"Hello"));
        json::dom_object* o1 = doc.create_object();
        a1->append(o1);
        o1->members()->append(L"Name1", doc.create_string(L"Value1"));
        o1->members()->append(L"Name2", doc.create_literal(L"true"));
        json::dom_object* o2 = doc.create_object();
        o1->members()->append(L"Obj2", o2);
        o2->append_member(L"Obj2.Name1", doc.create_string(L"Value11"));
        ASSERT_TRUE(((json::dom_array*)doc.root())->size() > 0);
    }
    chk.checkpoint();
    ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
}

}
