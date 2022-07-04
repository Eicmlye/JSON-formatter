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
#define EXPECT_EQ_STRING(expect, actual) \
    EXPECT_EQ_BASE(!strcmp(expect, actual), expect, actual, "%s")

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

#define TEST_LEGAL_NUMBER(value, context, expectNum) \
    do {\
        TEST_LEGAL(value, context, Lept::Type::NUMBER); \
        EXPECT_EQ_DOUBLE(expectNum, value.getNum()); \
    } while (0)
#define TEST_LEGAL_STRING(value, context, expectStr) \
    do {\
        TEST_LEGAL(value, context, Lept::Type::STRING); \
        char* stdstr = new char[(*value.getStr()).size() + 1]; \
        strcpy(stdstr, (*value.getStr()).c_str()); \
        EXPECT_EQ_STRING(expectStr, stdstr); \
    } while (0)

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
    /* test string */
#if 1
    TEST_LEGAL_STRING(v, "\"\"", "");
    TEST_LEGAL_STRING(v, "\"Hello\"", "Hello");
    TEST_LEGAL_STRING(v, "\"Hello\\nWorld\"", "Hello\nWorld");
    TEST_LEGAL_STRING(v, "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"", "\" \\ / \b \f \n \r \t");
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
    /* test string */
#if 1
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "\"null\"Hello");
    TEST_ERROR(v, Lept::PARSE_ROOT_NOT_SINGULAR, "\"12\\n3\"\"extra\"");
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

    return; 
}
static void testMissingQuotationMark(void)
{
    Lept::Value v;

    TEST_ERROR(v, Lept::PARSE_MISSING_QUOTATION_MARK, "\"");
    TEST_ERROR(v, Lept::PARSE_MISSING_QUOTATION_MARK, "\"abc");

    return; 
}
static void testInvalidStringEscape(void)
{
    Lept::Value v;

    TEST_ERROR(v, Lept::PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(v, Lept::PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(v, Lept::PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(v, Lept::PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");

    return; 
}
static void testInvalidStringChar(void)
{
    Lept::Value v;

    TEST_ERROR(v, Lept::PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(v, Lept::PARSE_INVALID_STRING_CHAR, "\"\x1F\"");

    return; 
}

int main(void) 
{
    /* test parse result */
    testLegal();

    testExpectValue();
    testInvalidValue();
    testRootNotSingular();
    testNumberOverflow();
    testMissingQuotationMark(); 
    testInvalidStringEscape(); 
    testInvalidStringChar(); 

    /* test access methods */


    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);

    return main_ret;
}
