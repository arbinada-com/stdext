#include "stdafx.h"
#include "CppUnitTest.h"
#include "datetime.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace stdext;

namespace stdexttest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
            datetime d1("2019-10-30");
            Assert::AreEqual(2019, d1.GetYear(), L"get_year()");
		}

	};
}