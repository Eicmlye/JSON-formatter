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

#define EXPECT_EQ_DOUBLE(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lf")

#define TEST_INT(value, errinfo, context, type) \
    do {\
        value.setType(Lept::Type::NULLJSON); \
        EXPECT_EQ_INT(errinfo, value.parse(context)); \
        EXPECT_EQ_INT(type, value.getType()); \
    } while(0)

#define TEST_LEGAL(value, context, type) \
    TEST_INT(value, Lept::PARSE_OK, context, type)

#define TEST_LEGAL_NUMBER(value, context, expectNum) \
    do {\
        value.setType(Lept::Type::NULLJSON); \
        EXPECT_EQ_INT(Lept::PARSE_OK, value.parse(context)); \
        EXPECT_EQ_INT(Lept::Type::NUMBER, value.getType()); \
        EXPECT_EQ_DOUBLE(expectNum, value.getNum()); \
    } while (0)

#define TEST_ERROR(value, errinfo, context) \
    TEST_INT(value, errinfo, context, Lept::Type::NULLJSON)

static void testLegal(void)
{
    Lept::Value v;

    TEST_LEGAL(v, "null", Lept::Type::NULLJSON);
    TEST_LEGAL(v, "false", Lept::Type::FALSE);
    TEST_LEGAL(v, "true", Lept::Type::TRUE);

    /* test number */
#if 1
    TEST_LEGAL_NUMBER(v, "0", 0.0);
    TEST_LEGAL_NUMBER(v, "-0", 0.0);
    TEST_LEGAL_NUMBER(v, "0.0", 0.0);
    TEST_LEGAL_NUMBER(v, "-0.0", 0.0);
    TEST_LEGAL_NUMBER(v, "1", 1.0);
    TEST_LEGAL_NUMBER(v, "-1", -1.0);
    TEST_LEGAL_NUMBER(v, "1.0", 1.0);
    TEST_LEGAL_NUMBER(v, "1.5", 1.5);
    TEST_LEGAL_NUMBER(v, "-1.5", -1.5);
    TEST_LEGAL_NUMBER(v, "3.1416", 3.1416);
    TEST_LEGAL_NUMBER(v, "0e10", 0.0);
    TEST_LEGAL_NUMBER(v, "0E10", 0.0);
    TEST_LEGAL_NUMBER(v, "1e10", 1e10);
    TEST_LEGAL_NUMBER(v, "1E10", 1E10);
    TEST_LEGAL_NUMBER(v, "1e012", 1e12);
    TEST_LEGAL_NUMBER(v, "1e+10", 1e+10);
    TEST_LEGAL_NUMBER(v, "1e-10", 1e-10);
    TEST_LEGAL_NUMBER(v, "-1e10", -1e10);
    TEST_LEGAL_NUMBER(v, "-1e+10", -1e+10);
    TEST_LEGAL_NUMBER(v, "-1e-10", -1e-10);
    TEST_LEGAL_NUMBER(v, "1.234e10", 1.234e10);
    TEST_LEGAL_NUMBER(v, "1.234e+10", 1.234e+10);
    TEST_LEGAL_NUMBER(v, "1.234e-10", 1.234e-10);
    TEST_LEGAL_NUMBER(v, "-1.234e-10", -1.234e-10);
    TEST_LEGAL_NUMBER(v, "1e-10000", 0.0); /* must underflow */
#endif

    return; 
}
static void testExpectValue(void)
{
    Lept::Value v; 

    TEST_ERROR(v, Lept::PARSE_EXPECT_VALUE, "");
    TEST_ERROR(v, Lept::PARSE_EXPECT_VALUE, " ");

    return; 
}
static void testInvalidValue(void)
{
    Lept::Value v; 

    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "nul ");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "nulx");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "?");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, " ?");

    /* test number */
#if 1
    /* should not start with plus sign */
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "+1");
    /* at least one digit before dot */
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, ".123");
    /* at least one digit after dot */
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "1.");
    /* special format number */
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "Nan");
    TEST_ERROR(v, Lept::PARSE_INVALID_VALUE, "nan");
#endif

    return; 
}
static void testRootNotSingular(void)
{
    Lept::Value v; 

    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "null x");
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "nullx");

    /* test number */
#if 1
    /* integer part should not start with 0 */
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "0123");
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "0.1x23");
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif

    return; 
}
static void testNumberOverflow(void)
{
    Lept::Value v; 

#if 1
    TEST_ERROR(v, Lept::PARSE_NUMBER_OVERFLOW, "1e309");
    TEST_ERROR(v, Lept::PARSE_NUMBER_OVERFLOW, "-1e309");
#endif
}
static void testError(void)
{
    testExpectValue();
    testInvalidValue();
    testRootNotSingular();
    testNumberOverflow(); 

    return; 
}

static void test_parse() 
{
    testLegal();
    testError(); 

    return; 
}

int main() 
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);

    return main_ret;
}
