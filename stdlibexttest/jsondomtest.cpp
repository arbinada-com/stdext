﻿#include "CppUnitTest.h"
#include "json.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include "locutils.h"
#include "strutils.h"
#include "memchecker.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

TEST_CLASS(JsonDomTest)
{
public:
    TEST_METHOD(TestValueTypeNames)
    {
        for (json::dom_value_type vtype = json::first_value_type; vtype != json::last_value_type; vtype = (json::dom_value_type)(static_cast<int>(vtype) + 1))
        {
            string s = json::to_string(vtype);
            Assert::IsFalse(s == "unsupported",
                strutils::format(L"Undefined name for value_type %d", static_cast<int>(vtype)).c_str());
        }
    }

    TEST_METHOD(TestDomValues_Literal)
    {
        json::dom_document doc;
        json::dom_literal* v11 = doc.create_literal(L"true");
        Assert::AreEqual<wstring>(L"true", v11->text(), L"v11 text");
        Assert::IsTrue(json::dom_value_type::vt_literal == v11->type(), L"v11 type");
        Assert::IsTrue(json::dom_literal_value_type::lvt_true == v11->subtype(), L"v11 subtype");
        json::dom_literal* v12 = doc.create_literal(L"false");
        Assert::AreEqual<wstring>(L"false", v12->text(), L"v12 text");
        Assert::IsTrue(json::dom_value_type::vt_literal == v12->type(), L"v12 type");
        Assert::IsTrue(json::dom_literal_value_type::lvt_false == v12->subtype(), L"v12 subtype");
        json::dom_literal* v2 = doc.create_literal(L"null");
        Assert::AreEqual<wstring>(L"null", v2->text(), L"v2 text");
        Assert::IsTrue(json::dom_value_type::vt_literal == v2->type(), L"v2 type");
        Assert::IsTrue(json::dom_literal_value_type::lvt_null == v2->subtype(), L"v2 subtype");
    }

    TEST_METHOD(TestDomValues_Number)
    {
        auto test_number = [](json::dom_number* value, const wstring text, const json::dom_number_value_type subtype, wstring title)
        {
            title += L": ";
            Assert::AreEqual<wstring>(text, value->text(), (title + L"text").c_str());
            Assert::IsTrue(json::dom_value_type::vt_number == value->type(), (title + L"type").c_str());
            Assert::IsTrue(subtype == value->subtype(), (title + L"subtype").c_str());
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

    TEST_METHOD(TestDomValues_Other)
    {
        json::dom_document doc;
        //
        json::dom_string* v4 = doc.create_string(L"Hello DOM");
        Assert::AreEqual<wstring>(L"Hello DOM", v4->text(), L"v4 text");
        Assert::IsTrue(json::dom_value_type::vt_string == v4->type(), L"v4 type");
        //
        json::dom_array* v5 = doc.create_array();
        Assert::AreEqual<wstring>(L"", v5->text(), L"v5 text");
        Assert::AreEqual<int>(0, v5->size(), L"v5 size");
        Assert::IsTrue(json::dom_value_type::vt_array == v5->type(), L"v5 type");
        //
        json::dom_object* v6 = doc.create_object();
        Assert::AreEqual<wstring>(L"", v6->text(), L"v6 text");
        Assert::AreEqual<int>(0, v6->members()->size(), L"v6 values size");
        Assert::IsTrue(json::dom_value_type::vt_object == v6->type(), L"v6 type");
    }

    TEST_METHOD(TestDomMembers)
    {
        auto make_name = [](const int index)->wstring { return L"Value " + std::to_wstring(index); };
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
        Assert::AreEqual(test_data.size(), o1->members()->size(), L"Size 1");
        for (size_t i = 0; i < test_data.size(); i++)
        {
            json::dom_object_member::name_t name = make_name(i + 1);
            Assert::AreEqual(name, o1->members()->at(i)->name(), strutils::format(L"Name %d", i).c_str());
            Assert::IsTrue(test_data[i] == (*o1)[i]->value(), strutils::format(L"Value ptr %d", i).c_str());
            Assert::IsTrue(test_data[i]->type() == (*o1)[i]->value()->type(), strutils::format(L"Value type %d", i).c_str());
            Assert::IsTrue(test_data[i]->text() == (*o1)[i]->value()->text(), strutils::format(L"Value %d", i).c_str());
            json::dom_value* v = o1->find(name);
            Assert::IsTrue(o1->contains_member(name), strutils::format(L"Value2 name %d", i).c_str());
            Assert::IsNotNull(v, strutils::format(L"Value2 %d", i).c_str());
            Assert::IsTrue(test_data[i] == v, strutils::format(L"Value2 ptr %d", i).c_str());
            Assert::IsTrue(test_data[i]->type() == v->type(), strutils::format(L"Value2 type %d", i).c_str());
            Assert::IsTrue(test_data[i]->text() == v->text(), strutils::format(L"Value2 %d", i).c_str());
        }
    }

    void CheckTestDocValue(wstring path, json::dom_value* v1)
    {
        wstring title = path + L": ";
        if (path == L"0")
        {
            Assert::IsTrue(nullptr != dynamic_cast<json::dom_array*>(v1), (title + L"cast 1").c_str());
            json::indexer_intf* idx = dynamic_cast<json::indexer_intf*>(v1);
            Assert::IsTrue(idx != nullptr, (title + L"cast 2").c_str());
            Assert::AreEqual<int>(3, idx->value_count(), (title + L"count").c_str());
        }
        else if (path == L"0.0")
        {
            Assert::IsTrue(json::dom_value_type::vt_string == v1->type(), (title + L"type").c_str());
            Assert::AreEqual<wstring>(L"Hello", dynamic_cast<json::dom_string*>(v1)->text(), (title + L"text").c_str());
        }
        else if (path == L"0.1")
        {
            Assert::IsTrue(json::dom_value_type::vt_literal == v1->type(), (title + L"type 1.1").c_str());
            Assert::IsTrue(json::dom_literal_value_type::lvt_null == dynamic_cast<json::dom_literal*>(v1)->subtype(), (title + L"subtype 1.1").c_str());
            Assert::IsTrue(nullptr == dynamic_cast<json::dom_object*>(v1), (title + L"cast 1").c_str());
            Assert::IsTrue(nullptr == dynamic_cast<json::indexer_intf*>(v1), (title + L"cast 2").c_str());
        }
        else if (path == L"0.2")
        {
            Assert::IsTrue(nullptr != dynamic_cast<json::dom_object*>(v1), (title + L"cast 1").c_str());
            json::indexer_intf* idx = dynamic_cast<json::indexer_intf*>(v1);
            Assert::IsTrue(idx != nullptr, (title + L"cast 2").c_str());
            Assert::AreEqual<int>(5, idx->value_count(), (title + L"count").c_str());
        }
        else if (path == L"0.2.0")
        {
            Assert::IsTrue(json::dom_value_type::vt_string == v1->type(), (title + L"type").c_str());
            Assert::AreEqual<wstring>(L"World", dynamic_cast<json::dom_string*>(v1)->text(), (title + L"text").c_str());
            Assert::IsTrue(v1->member() != nullptr, (title + L"member").c_str());
            Assert::AreEqual<wstring>(L"Str 1", v1->member()->name(), (title + L"member name").c_str());
        }
        else if (path == L"0.2.1")
        {
            Assert::IsTrue(json::dom_value_type::vt_number == v1->type(), (title + L"type").c_str());
            Assert::IsTrue(json::dom_number_value_type::nvt_int == dynamic_cast<json::dom_number*>(v1)->subtype(), (title + L"subtype").c_str());
            Assert::AreEqual<wstring>(L"123", dynamic_cast<json::dom_number*>(v1)->text(), (title + L"text").c_str());
            Assert::IsTrue(v1->member() != nullptr, (title + L"member").c_str());
            Assert::AreEqual<wstring>(L"Num 1", v1->member()->name(), (title + L"member name").c_str());
        }
        else if (path == L"0.2.2")
        {
            Assert::IsTrue(nullptr != dynamic_cast<json::dom_array*>(v1), (title + L"cast 1").c_str());
            json::indexer_intf* idx = dynamic_cast<json::indexer_intf*>(v1);
            Assert::IsTrue(idx != nullptr, (title + L"cast 2").c_str());
            Assert::AreEqual<int>(0, idx->value_count(), (title + L"count").c_str());
            Assert::IsTrue(v1->member() != nullptr, (title + L"member").c_str());
            Assert::AreEqual<wstring>(L"Arr 1", v1->member()->name(), (title + L"member name").c_str());
        }
        else if (path == L"0.2.3")
        {
            Assert::IsTrue(json::dom_value_type::vt_literal == v1->type(), (title + L"type").c_str());
            Assert::IsTrue(json::dom_literal_value_type::lvt_false == dynamic_cast<json::dom_literal*>(v1)->subtype(), (title + L"subtype").c_str());
            Assert::AreEqual<wstring>(L"false", dynamic_cast<json::dom_literal*>(v1)->text(), (title + L"text").c_str());
            Assert::IsTrue(v1->member() != nullptr, (title + L"member").c_str());
            Assert::AreEqual<wstring>(L"Literal 1", v1->member()->name(), (title + L"member name").c_str());
        }
        else if (path == L"0.2.4")
        {
            Assert::IsTrue(nullptr != dynamic_cast<json::dom_array*>(v1), (title + L"cast 1").c_str());
            json::indexer_intf* idx = dynamic_cast<json::indexer_intf*>(v1);
            Assert::IsTrue(idx != nullptr, (title + L"cast 2").c_str());
            Assert::AreEqual<int>(1, idx->value_count(), (title + L"count").c_str());
            Assert::IsTrue(v1->member() != nullptr, (title + L"member").c_str());
            Assert::AreEqual<wstring>(L"Arr 2", v1->member()->name(), (title + L"member name").c_str());
        }
        else if (path == L"0.2.4.0")
        {
            Assert::IsTrue(json::dom_value_type::vt_number == v1->type(), (title + L"type").c_str());
            Assert::IsTrue(json::dom_number_value_type::nvt_float == dynamic_cast<json::dom_number*>(v1)->subtype(), (title + L"subtype").c_str());
            Assert::AreEqual<wstring>(L"456.78", dynamic_cast<json::dom_number*>(v1)->text(), (title + L"text").c_str());
        }
        else
            Assert::Fail((title + L"unsupported path").c_str());
    }

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

    TEST_METHOD(TestDomIndexers)
    {
        json::dom_document doc;
        FillTestDoc(doc);
        //
        CheckTestDocValue(L"0", doc.root());
        json::indexer_intf* idx1 = dynamic_cast<json::indexer_intf*>(doc.root());
        json::dom_value* v1 = idx1->get_value(0);
        CheckTestDocValue(L"0.0", v1);
        v1 = idx1->get_value(1);
        CheckTestDocValue(L"0.1", v1);
        v1 = idx1->get_value(2);
        CheckTestDocValue(L"0.2", v1);
        //
        json::indexer_intf* idx2 = dynamic_cast<json::indexer_intf*>(v1);
        v1 = idx2->get_value(0);
        CheckTestDocValue(L"0.2.0", v1);
        v1 = idx2->get_value(1);
        CheckTestDocValue(L"0.2.1", v1);
        v1 = idx2->get_value(2);
        CheckTestDocValue(L"0.2.2", v1);
        json::indexer_intf* idx3 = dynamic_cast<json::indexer_intf*>(v1);
        v1 = idx2->get_value(3);
        CheckTestDocValue(L"0.2.3", v1);
        v1 = idx2->get_value(4);
        CheckTestDocValue(L"0.2.4", v1);
        json::indexer_intf* idx4 = dynamic_cast<json::indexer_intf*>(v1);
        v1 = idx4->get_value(0);
        CheckTestDocValue(L"0.2.4.0", v1);
    }

    TEST_METHOD(TestDomDocument)
    {
        {
            json::dom_document doc;
            Assert::IsNull(doc.root(), L"Root 1");
            doc.root(doc.create_string(L"Hello world!"));
            Assert::IsNotNull(doc.root(), L"Root 2");
            Assert::AreEqual<wstring>(L"Hello world!", doc.root()->text(), L"Root text");
        }
    }

    TEST_METHOD(TestDomDocumentIterator)
    {
        json::dom_document doc;
        json::dom_document::iterator it = doc.begin();
        Assert::IsTrue(it.value() == nullptr, L"Curent 1.1");
        Assert::IsTrue(*it == nullptr, L"Curent 1.2");
        Assert::IsTrue(it == doc.end(), L"Curent 1.3");
        Assert::AreEqual<int>(0, it.level(), L"it.level 1");
        FillTestDoc(doc);
        it = doc.begin();
        Assert::IsTrue(it.value() == doc.root(), L"Curent 2.1");
        Assert::IsTrue(*it == doc.root(), L"Curent 2.2");
        Assert::IsTrue(*it == it.value(), L"Curent 2.2");
        Assert::IsTrue(it != doc.end(), L"Curent 2.4");
        Assert::AreEqual<int>(1, it.level(), L"it.level 2");
        //
        json::dom_document::iterator::path_t path = { 0 };
        Assert::IsTrue(path == it.path(), L"path 0");
        Assert::IsFalse(it.has_prev_sibling(), L"has_prev_sibling 0");
        CheckTestDocValue(L"0", *it);
        it++;
        path = { 0, 0 };
        Assert::IsTrue(path == it.path(), L"path 0.0");
        Assert::IsFalse(it.has_prev_sibling(), L"has_prev_sibling 0.0");
        CheckTestDocValue(L"0.0", *it);
        it++;
        path = { 0, 1 };
        Assert::IsTrue(path == it.path(), L"path 0.1");
        Assert::IsTrue(it.has_prev_sibling(), L"has_prev_sibling 0.1");
        CheckTestDocValue(L"0.1", *it);
        it++;
        path = { 0, 2 };
        Assert::IsTrue(path == it.path(), L"path 0.2");
        CheckTestDocValue(L"0.2", *it);
        it++;
        path = { 0, 2, 0 };
        Assert::IsTrue(path == it.path(), L"path 0.2.0");
        CheckTestDocValue(L"0.2.0", *it);
        it++;
        path = { 0, 2, 1 };
        Assert::IsTrue(path == it.path(), L"path 0.2.1");
        CheckTestDocValue(L"0.2.1", *it);
        it++;
        path = { 0, 2, 2 };
        Assert::IsTrue(path == it.path(), L"path 0.2.2");
        CheckTestDocValue(L"0.2.2", *it);
        it++;
        path = { 0, 2, 3 };
        Assert::IsTrue(path == it.path(), L"path 0.2.3");
        CheckTestDocValue(L"0.2.3", *it);
        it++;
        path = { 0, 2, 4 };
        Assert::IsTrue(path == it.path(), L"path 0.2.4");
        Assert::IsTrue(it.has_prev_sibling(), L"has_prev_sibling 0.2.4");
        CheckTestDocValue(L"0.2.4", *it);
        it++;
        path = { 0, 2, 4, 0 };
        Assert::IsTrue(path == it.path(), L"path 0.2.4.0");
        Assert::IsFalse(it.has_prev_sibling(), L"has_prev_sibling 0.2.4.0");
        CheckTestDocValue(L"0.2.4.0", *it);
        it++;
        Assert::IsTrue(it == doc.end(), L"End 1");
        Assert::AreEqual<int>(0, it.level(), L"End 2");
        //
        int count = 0;
        for (const auto value : doc) // const auto -> const json::dom_value*
            count++;
        Assert::AreEqual(10, count, L"for const auto");
        count = 0;
        for (auto value : doc) // auto -> json::dom_value*
            count++;
        Assert::AreEqual(10, count, L"for auto");
        count = 0;
        for (json::dom_value* value : doc)
            count++;
        Assert::AreEqual(10, count, L"for json::dom_value*");
    }

    TEST_METHOD(TestDomDocumentComparison)
    {
        json::dom_document doc1;
        json::dom_document doc2;
        Assert::IsTrue(json::equal(doc1, doc2), L"Cmp 1");
        FillTestDoc(doc1);
        Assert::IsFalse(json::equal(doc1, doc2), L"Cmp 2");
        FillTestDoc(doc2);
        Assert::IsTrue(json::equal(doc1, doc2), L"Cmp 3");
        doc1.clear();
        Assert::IsFalse(json::equal(doc1, doc2), L"Cmp 4");
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
        Assert::IsFalse(json::equal(doc1, doc2), L"Cmp 5");
        //
        doc1.clear();
        doc2.clear();
        doc1.root(doc1.create_string(L"Str1"));
        doc2.root(doc2.create_string(L"Str1"));
        Assert::IsTrue(json::equal(doc1, doc2), L"Cmp 6");
    }

    TEST_METHOD(TestDomDocumentMemory)
    {
        memchecker chk;
        {
            json::dom_document doc;
            doc.root(doc.create_string(L"Hello world!"));
            Assert::IsNotNull(doc.root());
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
            Assert::IsTrue(((json::dom_array*)doc.root())->size() > 0);
        }
        chk.checkpoint();
        Assert::IsFalse(chk.has_leaks(), chk.wreport().c_str());
    }


};

