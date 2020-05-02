#include "CppUnitTest.h"
#include "json.h"
#include <fstream>
#include <vector>
#include <limits>
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

    TEST_METHOD(TestDomValues)
    {
        json::dom_document doc;
        json::dom_literal* v11 = doc.create_literal(L"true");
        Assert::AreEqual<wstring>(L"true", v11->text(), L"v11 text");
        Assert::IsTrue(json::dom_value_type::vt_literal == v11->type(), L"v11 type");
        json::dom_literal* v12 = doc.create_literal(L"false");
        Assert::AreEqual<wstring>(L"false", v12->text(), L"v12 text");
        Assert::IsTrue(json::dom_value_type::vt_literal == v12->type(), L"v12 type");
        json::dom_literal* v2 = doc.create_literal(L"null");
        Assert::AreEqual<wstring>(L"null", v2->text(), L"v2 text");
        Assert::IsTrue(json::dom_value_type::vt_literal == v2->type(), L"v2 type");
        json::dom_number* v3 = doc.create_number(L"123.456");
        Assert::AreEqual<wstring>(L"123.456", v3->text(), L"v3 text");
        Assert::IsTrue(json::dom_value_type::vt_number == v3->type(), L"v3 type");
        json::dom_string* v4 = doc.create_string(L"Hello DOM");
        Assert::AreEqual<wstring>(L"Hello DOM", v4->text(), L"v4 text");
        Assert::IsTrue(json::dom_value_type::vt_string == v4->type(), L"v4 type");
        json::dom_array* v5 = doc.create_array();
        Assert::AreEqual<wstring>(L"", v5->text(), L"v5 text");
        Assert::AreEqual<int>(0, v5->size(), L"v5 size");
        Assert::IsTrue(json::dom_value_type::vt_array == v5->type(), L"v5 type");
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
        test_data.push_back(doc.create_number(L"123"));
        test_data.push_back(doc.create_number(std::to_wstring(std::numeric_limits<int64_t>::max())));
        test_data.push_back(doc.create_number(L"123.456"));
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
            Assert::IsNotNull(v, strutils::format(L"Value2 %d", i).c_str());
            Assert::IsTrue(test_data[i] == v, strutils::format(L"Value2 ptr %d", i).c_str());
            Assert::IsTrue(test_data[i]->type() == v->type(), strutils::format(L"Value2 type %d", i).c_str());
            Assert::IsTrue(test_data[i]->text() == v->text(), strutils::format(L"Value2 %d", i).c_str());
        }
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
        {
            //json::dom_document doc;
            //Assert::IsNull(doc.root(), L"Root 1");
            //doc.create_root(json::dom_value_type::vt_array);
            //Assert::IsNotNull(doc.root(), L"Root 2");
            //Assert::AreEqual<wstring>(L"", doc.root()->text(), L"Root text");
        }
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
            a1->append(doc.create_number(L"123.456"));
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
