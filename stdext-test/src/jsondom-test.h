#pragma once
#include <gtest/gtest.h>
#include <string>
#include "json.h"

namespace jsondom_test
{

class JsonDomTest : public testing::Test
{
public:
    static void FillTestDoc(stdext::json::dom_document& doc);
protected:
    void CheckTestDocValue(std::wstring path, stdext::json::dom_value* v1);

};

}
