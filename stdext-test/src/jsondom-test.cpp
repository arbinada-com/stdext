#include "jsondom-test.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include "locutils.h"
#include "strutils.h"
#include "testutils.h"


using namespace std;

namespace stdext
{
namespace jsondom_test
{

void JsonDomTest::FillTestDoc(json::dom_document& doc)
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

void JsonDomTest::CheckTestDocValue(wstring path, json::dom_value* v1)
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
        ASSERT_TRUE(json::dom_literal_type::lvt_null == dynamic_cast<json::dom_literal*>(v1)->literal_type()) << title + L"literal_type 1.1";
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
        ASSERT_TRUE(json::dom_number_type::nvt_int == dynamic_cast<json::dom_number*>(v1)->numtype()) << title + L"literal_type";
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
        EXPECT_TRUE(json::dom_literal_type::lvt_false == dynamic_cast<json::dom_literal*>(v1)->literal_type()) << title + L"literal_type";
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
        EXPECT_TRUE(json::dom_number_type::nvt_float == dynamic_cast<json::dom_number*>(v1)->numtype()) << title + L"literal_type";
        EXPECT_EQ(L"456.78", dynamic_cast<json::dom_number*>(v1)->text()) << title + L"text";
    }
    else
        FAIL() << title + L"unsupported path";
}

TEST_F(JsonDomTest, TestValueTypeNames)
{
    for (json::dom_value_type vtype = json::first_value_type();
         vtype != json::last_value_type();
         vtype = (json::dom_value_type)(static_cast<int>(vtype) + 1))
    {
        string s = json::to_string(vtype);
        ASSERT_NE(s, "unsupported") << str::wformat(L"Undefined name for value_type %d", static_cast<int>(vtype));
    }
}

TEST_F(JsonDomTest, TestDomValues_Literal)
{
    json::dom_document doc;
    json::dom_literal* v11 = doc.create_literal(L"true");
    ASSERT_EQ(L"true", v11->text()) << L"v11 text";
    ASSERT_TRUE(json::dom_value_type::vt_literal == v11->type()) << L"v11 type";
    ASSERT_TRUE(json::dom_literal_type::lvt_true == v11->literal_type()) << L"v11 literal_type";
    json::dom_literal* v12 = doc.create_literal(L"false");
    ASSERT_EQ(L"false", v12->text()) << L"v12 text";
    ASSERT_TRUE(json::dom_value_type::vt_literal == v12->type()) << L"v12 type";
    ASSERT_TRUE(json::dom_literal_type::lvt_false == v12->literal_type()) << L"v12 literal_type";
    json::dom_literal* v2 = doc.create_literal(L"null");
    ASSERT_EQ(L"null", v2->text()) << L"v2 text";
    ASSERT_TRUE(json::dom_value_type::vt_literal == v2->type()) << L"v2 type";
    ASSERT_TRUE(json::dom_literal_type::lvt_null == v2->literal_type()) << L"v2 literal_type";
}

TEST_F(JsonDomTest, TestDomValues_Number)
{
    auto test_number = [](json::dom_number* value, const wstring text, const json::dom_number_type literal_type, wstring title)
    {
        title += L": ";
        EXPECT_EQ(text, value->text()) << title + L"text";
        EXPECT_TRUE(json::dom_value_type::vt_number == value->type()) << title + L"type";
        EXPECT_TRUE(literal_type == value->numtype()) << title + L"literal_type";
        delete value;
    };
    json::dom_document doc;
    test_number(doc.create_number(L"123456", json::dom_number_type::nvt_int),
                L"123456", json::dom_number_type::nvt_int, L"Int 1");
    test_number(doc.create_number(123456),
                L"123456", json::dom_number_type::nvt_int, L"Int 2");
    test_number(doc.create_number(std::numeric_limits<int64_t>::max()),
                L"9223372036854775807", json::dom_number_type::nvt_int, L"Int 3");
    test_number(doc.create_number(L"123.456", json::dom_number_type::nvt_float),
                L"123.456", json::dom_number_type::nvt_float, L"Decimal 1");
    test_number(doc.create_number(123.456),
                L"123.456", json::dom_number_type::nvt_float, L"Float 1.1");
    {
        locutils::locale_guard loc(LC_NUMERIC, "ru_RU");
        test_number(doc.create_number(123.456), // 123.456 -> "123,456" on ru_RU
                    L"123.456", json::dom_number_type::nvt_float, L"Float 1.2");
    }
    test_number(doc.create_number(123.456),
                L"123.456", json::dom_number_type::nvt_float, L"Float 1.3");
    test_number(doc.create_number(123456.0),
                L"123456.0", json::dom_number_type::nvt_float, L"Float 1.4");
    test_number(doc.create_number(0.123456),
                L"0.123456", json::dom_number_type::nvt_float, L"Float 1.5");
    test_number(doc.create_number(0.0),
                L"0.0", json::dom_number_type::nvt_float, L"Float 1.6");
    test_number(doc.create_number(123.456e10),
                L"1.23456e+12", json::dom_number_type::nvt_float, L"Float 2.1");
    test_number(doc.create_number(0.123456e-10),
                L"1.23456e-11", json::dom_number_type::nvt_float, L"Float 2.2");
    test_number(doc.create_number(1.23456e8),
                L"1.23456e+08", json::dom_number_type::nvt_float, L"Float 2.3");
    test_number(doc.create_number(1.23456e-8),
                L"1.23456e-08", json::dom_number_type::nvt_float, L"Float 2.4");
    test_number(doc.create_number(1.0e-8),
                L"1e-08", json::dom_number_type::nvt_float, L"Float 2.5");
    test_number(doc.create_number(1.0e-80),
                L"1e-80", json::dom_number_type::nvt_float, L"Float 2.6");
    test_number(doc.create_number(1.0e+80),
                L"1e+80", json::dom_number_type::nvt_float, L"Float 2.7");
    test_number(doc.create_number(123456e+00),
                L"123456.0", json::dom_number_type::nvt_float, L"Float 2.8");
    test_number(doc.create_number(0e+80),
                L"0.0", json::dom_number_type::nvt_float, L"Float 2.9");
}

TEST_F(JsonDomTest, TestDomValues_Number_Random)
{
    testutils::rnd_helper rnd;
    json::dom_document doc;
    for (int i = 0; i < 10000; i++)
    {
        json::dom_number *v1, *v2;
        json::dom_number_type numtype;
        if (rnd.random_bool())
        {
            numtype = json::dom_number_type::nvt_int;
            int value = rnd.random_range(numeric_limits<int>::min(), numeric_limits<int>::max());
            v1 = doc.create_number(value);
            v2 = new json::dom_number(&doc, value);
        }
        else
        {
            numtype = json::dom_number_type::nvt_float;
            double value = std::pow(10.0f, rnd.random_range(-20, 20)) * (rnd.random_float() - 0.5);
            wstring s = json::dom_number::to_text(value);
            v1 = doc.create_number(value);
            v2 = new json::dom_number(&doc, value);
            EXPECT_EQ(v1->text(), s);
            EXPECT_EQ(v2->text(), s);
            bool is_float = s.find(L'.') != s.npos || s.find(L'e') != s.npos || s.find(L'E') != s.npos;
            ASSERT_TRUE(is_float) << s << str::wformat(L", value: %#g", value);
        }
        ASSERT_EQ(v1->text(), v2->text());
        EXPECT_EQ(v1->numtype(), numtype) << L"value 1: " + v1->to_wstring();
        EXPECT_EQ(v2->numtype(), numtype) << L"value 2: " + v2->to_wstring();
        delete v1;
        delete v2;
    }
}

TEST_F(JsonDomTest, TestDomValues_String)
{
    json::dom_document doc;
    {
        json::dom_string* v = doc.create_string(L"Hello DOM");
        doc.root(v);
        EXPECT_EQ(L"Hello DOM", v->text()) << L"text 1";
        EXPECT_TRUE(json::dom_value_type::vt_string == v->type()) << L"type 1";
    }
    {
        doc.clear();
        json::dom_string* v = doc.create_string(L"a\\u005Cb");
        doc.root(v);
        EXPECT_NE(L"a\\u005Cb", v->text()) << L"text 2.1";
        EXPECT_EQ(L"a\\b", v->text()) << L"text 2.2";
    }
}

TEST_F(JsonDomTest, TestDomValues_Other)
{
    json::dom_document doc;
    //
    json::dom_array* v5 = doc.create_array();
    EXPECT_EQ(L"", v5->text()) << L"array text";
    EXPECT_EQ(0u, v5->size()) << L"array size";
    EXPECT_TRUE(json::dom_value_type::vt_array == v5->type()) << L"array type";
    //
    json::dom_object* v6 = doc.create_object();
    EXPECT_EQ(L"", v6->text()) << L"object text";
    EXPECT_EQ(0u, v6->members()->size()) << L"object values size";
    EXPECT_TRUE(json::dom_value_type::vt_object == v6->type()) << L"object type";
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
    test_data.push_back(doc.create_number(L"123.456", json::dom_number_type::nvt_float));
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
        ASSERT_EQ(name, o1->members()->at(i)->name()) << str::wformat(L"Name %d", i);
        ASSERT_TRUE(test_data[i] == (*o1)[i]->value()) << str::wformat(L"Value ptr %d", i);
        ASSERT_TRUE(test_data[i]->type() == (*o1)[i]->value()->type()) << str::wformat(L"Value type %d", i);
        ASSERT_TRUE(test_data[i]->text() == (*o1)[i]->value()->text()) << str::wformat(L"Value %d", i);
        json::dom_object_member* member = o1->find(name);
        ASSERT_TRUE(o1->contains_member(name)) << str::wformat(L"Member2 name %d", i);
        ASSERT_FALSE(member == nullptr) << str::wformat(L"Member2 %d", i);
        ASSERT_FALSE(member->value() == nullptr) << str::wformat(L"MemberValue2 %d", i);
        EXPECT_EQ(test_data[i], member->value()) << str::wformat(L"MemberValue2 ptr %d", i);
        EXPECT_EQ(test_data[i]->type(), member->value()->type()) << str::wformat(L"MemberValue2 type %d", i);
        EXPECT_EQ(test_data[i]->text(), member->value()->text()) << str::wformat(L"MemberValue2 %d", i);
        json::dom_value* v2 = o1->find_value(name);
        ASSERT_FALSE(v2 == nullptr) << str::wformat(L"Value2 %d", i);
        EXPECT_EQ(v2, member->value()) << str::wformat(L"Value2 ptr %d", i);
    }
}

TEST_F(JsonDomTest, TestDomMemberName)
{
    json::dom_document doc;
    json::dom_object* o = doc.create_object();
    doc.root(o);
    wstring name1     = L"ABC déjà строка",
            expected1 = L"ABC déjà строка",
            name2     = L"a\\u005Cb",
            expected2 = L"a\\b";
    json::dom_number* v1 = doc.create_number(123);
    o->append_member(name1, v1);
    json::dom_number* v2 = doc.create_number(456);
    o->append_member(name2, v2);
    EXPECT_EQ(v1->member()->name(), expected1);
    EXPECT_EQ(v2->member()->name(), expected2);
    auto test_member_name = [&](wstring name, int num, wstring title)
    {
        EXPECT_TRUE(o->members()->contains_name(name)) << title;
        EXPECT_TRUE(o->members()->get(name)) << title;
        ASSERT_TRUE(o->members()->find(name) != nullptr) << title;
        ASSERT_TRUE(o->find(name) != nullptr) << title;
        ASSERT_TRUE(o->find_value(name) != nullptr) << title;
        EXPECT_EQ(std::to_wstring(num), o->find_value(name)->text()) << title;
    };
    test_member_name(name1, 123, L"name1");
    test_member_name(expected1, 123, L"expected1");
    test_member_name(name2, 456, L"name2");
    test_member_name(expected2, 456, L"expected2");
}

TEST_F(JsonDomTest, TestDomMemberNotFoundException)
{
    json::dom_document doc;
    json::dom_object* o = doc.create_object();
    doc.root(o);
    wstring name1 = L"Value 1";
    o->append_member(name1, doc.create_number(123));
    EXPECT_TRUE(o->find(name1) != nullptr);
    wstring name2 = L"_" + name1;
    EXPECT_TRUE(o->find(name2) == nullptr);
    EXPECT_THROW(o->members()->get(name2), json::dom_member_not_found);
    EXPECT_THROW((*(o->members()))[name2], json::dom_member_not_found);
    json::dom_object_members* members = o->members();
    EXPECT_THROW((*members)[name2], json::dom_member_not_found);
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
    testutils::memchecker chk;
    {
        json::dom_document doc;
        doc.root(doc.create_string(L"Hello world!"));
        ASSERT_NE(doc.root(), nullptr);
        json::dom_array* a1 = doc.create_array();
        doc.root(a1);
        a1->append(doc.create_literal(L"null"));
        a1->append(doc.create_number(L"123.456", json::dom_number_type::nvt_float));
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
}
