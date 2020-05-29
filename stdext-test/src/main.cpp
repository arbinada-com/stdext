#include "containers-test.h"
#include "csvtools-test.h"
#include "datetime-test.h"
#include "ioutils-test.h"
#include "jsondom-test.h"
#include "jsonlexer-test.h"
#include "jsonparser-test.h"
#include "jsontools-test.h"
#include "locutils-test.h"
#include "parsers-test.h"
#include "platforms-test.h"
#include "strutils-test.h"
#include "trees-test.h"
#include "variants-test.h"

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
