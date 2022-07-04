#include <iostream>
#include <cstdlib>
#include <cstring>
#include "leptjson.h"

// main function return value; 
// 0 for NO-ERROR; 1 for ERROR-OCCURRED; 
static int main_ret = 0; 
// number of tests made; 
static int test_count = 0;
// number of tests passed; 
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        ++test_count; \
        if (equality)\
            ++test_pass; \
        else\
        {\
            fprintf(stderr, "%s:\n\tLine %d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
            main_ret = 1; \
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define TEST_INT(value, errinfo, context, type) \
    do {\
        value.setType(Lept::Type::NULLJSON); \
        EXPECT_EQ_INT(errinfo, value.parse(context)); \
        EXPECT_EQ_INT(type, value.getType()); \
    } while(0)

#define TEST_LEGAL(value, context, type) \
    TEST_INT(value, Lept::PARSE_OK, context, type)

#define TEST_ERROR(value, errinfo, context) \
    TEST_INT(value, errinfo, context, Lept::Type::NULLJSON)

static void test_parse() 
{
    Lept::Value v; 

    TEST_LEGAL(v, "null", Lept::Type::NULLJSON);
    TEST_LEGAL(v, "false", Lept::Type::FALSE);
    TEST_LEGAL(v, "true", Lept::Type::TRUE);

    TEST_ERROR(v, Lept::PARSE_EXPECT_VALUE, "");
    TEST_ERROR(v, Lept::PARSE_EXPECT_VALUE, " ");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "nul ");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "nulx");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "?");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, " ?");
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "null x");
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "nullx");

    return; 
}

int main() 
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);

    return main_ret;
}
