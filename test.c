//*******************************
// Created by Pickle on 2022/11/19.
//*******************************
#include <stdio.h>
#include <string.h>
#include "picklejson.h"
/* 检测内存泄露 */
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual),expect,actual,"%d")
#define EXPECT_EQ_DOUBLE(expect,actual) EXPECT_EQ_BASE((expect) == (actual),expect,actual,"%.17g")
#define EXPECT_EQ_STRING(expect,actual,alength) \
            EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect,actual,alength) == 0,expect,actual,"%s")
#define EXPECT_EQ_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false","%s")
#define EXPECT_EQ_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true","%s")

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do{\
        test_count++;\
        if(equality)\
            test_pass++;\
        else{\
            fprintf(stderr,"%s:%d: expect: " format " actual: " format "\n",__FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define TEST_ERROR(error, json)\
    do{\
        pickle_value v;\
        pickle_init(&v);\
        v.type=PICKLE_FALSE;\
        EXPECT_EQ_INT(error, pickle_parse(&v, json));\
        EXPECT_EQ_INT(PICKLE_NULL, pickle_get_type(&v));\
    }while(0)

#define TEST_NUMBER(expect,json)\
    do{\
        pickle_value v;\
        pickle_init(&v);\
        EXPECT_EQ_INT(PICKLE_PARSE_OK,pickle_parse(&v, json));\
        EXPECT_EQ_INT(PICKLE_NUMBER,pickle_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect,pickle_get_number(&v));\
    }while(0)

#define TEST_STRING(expect, json) \
    do{\
        pickle_value v;\
        pickle_init(&v);\
        EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v,json)); \
        EXPECT_EQ_INT(PICKLE_STRING, pickle_get_type(&v));     \
        EXPECT_EQ_STRING(expect, pickle_get_string(&v),pickle_get_string_len(&v)); \
        pickle_free(&v);\
    }while(0)

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif



static void test_parse_expect_value(){
    TEST_ERROR(PICKLE_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(PICKLE_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value(){
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE,"nul");
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE,"?");

#if 1
    /* invalid number */
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "nan");
#endif

#if 0
    /* invalid array */
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "[1,]");
    TEST_ERROR(PICKLE_PARSE_INVALID_VALUE, "[\"a\", nul]");
#endif
}

static void test_parse_root_not_singular(){
    TEST_ERROR(PICKLE_PARSE_ROOT_NOT_SINGULAR,"null x");

#if 1
    /* invalid number */
    TEST_ERROR(PICKLE_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(PICKLE_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PICKLE_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}

static void test_parse_null(){
    pickle_value v;
    pickle_init(&v);
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "null"));
    EXPECT_EQ_INT(PICKLE_NULL, pickle_get_type(&v));
}

static void test_parse_true(){
    pickle_value v;
    pickle_init(&v);
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "true"));
    EXPECT_EQ_INT(PICKLE_TRUE, pickle_get_type(&v));
}

static void test_parse_false(){
    pickle_value v;
    pickle_init(&v);
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "false"));
    EXPECT_EQ_INT(PICKLE_FALSE, pickle_get_type(&v));
}

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

#if 1
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
#endif
}

static void test_parse_number_too_big() {
#if 1
    TEST_ERROR(PICKLE_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PICKLE_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

static void test_parse_miss_comma_or_square_bracket() {
#if 1
    TEST_ERROR(PICKLE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(PICKLE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(PICKLE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(PICKLE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
#endif
}

static void test_access_null(){
    pickle_value v;
    pickle_init(&v);
    pickle_set_string(&v,"a",1);
    pickle_set_null(&v);
    EXPECT_EQ_INT(PICKLE_NULL, pickle_get_type(&v));
    pickle_free(&v);
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(PICKLE_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_string(){
    TEST_STRING("", "\"\"");
    TEST_STRING("HELLO","    \"HELLO\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_array(){
    pickle_value v;
    size_t i, j;

    pickle_init(&v);
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "[ ]"));
    EXPECT_EQ_INT(PICKLE_ARRAY, pickle_get_type(&v));
    EXPECT_EQ_SIZE_T(0, pickle_get_array_size(&v));
    pickle_free(&v);

    pickle_init(&v);
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(PICKLE_ARRAY, pickle_get_type(&v));
    EXPECT_EQ_SIZE_T(5, pickle_get_array_size(&v));
    EXPECT_EQ_INT(PICKLE_NULL,   pickle_get_type(pickle_get_array_element(&v, 0)));
    EXPECT_EQ_INT(PICKLE_FALSE,  pickle_get_type(pickle_get_array_element(&v, 1)));
    EXPECT_EQ_INT(PICKLE_TRUE,   pickle_get_type(pickle_get_array_element(&v, 2)));
    EXPECT_EQ_INT(PICKLE_NUMBER, pickle_get_type(pickle_get_array_element(&v, 3)));
    EXPECT_EQ_INT(PICKLE_STRING, pickle_get_type(pickle_get_array_element(&v, 4)));
    EXPECT_EQ_DOUBLE(123.0, pickle_get_number(pickle_get_array_element(&v, 3)));
    EXPECT_EQ_STRING("abc", pickle_get_string(pickle_get_array_element(&v, 4)), pickle_get_string_len(pickle_get_array_element(&v, 4)));
    pickle_free(&v);


    pickle_init(&v);
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(PICKLE_ARRAY, pickle_get_type(&v));
    EXPECT_EQ_SIZE_T(4, pickle_get_array_size(&v));
    for (i = 0; i < 4; i++) {
        pickle_value* a = pickle_get_array_element(&v, i);
        EXPECT_EQ_INT(PICKLE_ARRAY, pickle_get_type(a));
        EXPECT_EQ_SIZE_T(i, pickle_get_array_size(a));
        for (j = 0; j < i; j++) {
            pickle_value* e = pickle_get_array_element(a, j);
            EXPECT_EQ_INT(PICKLE_NUMBER, pickle_get_type(e));
            EXPECT_EQ_DOUBLE((double)j, pickle_get_number(e));
        }
    }
    pickle_free(&v);
}


static void test_access_boolean(){
    pickle_value v;
    pickle_init(&v);
    pickle_set_boolean(&v,1);
    EXPECT_EQ_TRUE(pickle_get_boolean(&v));
    pickle_set_boolean(&v,0);
    EXPECT_EQ_FALSE(pickle_get_boolean(&v));
    pickle_free(&v);
}

static void test_access_number(){
    pickle_value v;
    pickle_init(&v);
    pickle_set_number(&v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, pickle_get_number(&v));
}

static void test_access_string(){
    pickle_value v;
    pickle_init(&v);
    pickle_set_string(&v,"",0);
    EXPECT_EQ_STRING("", pickle_get_string(&v), pickle_get_string_len(&v));
    pickle_set_string(&v,"Hello",5);
    EXPECT_EQ_STRING("Hello", pickle_get_string(&v), pickle_get_string_len(&v));
}


static void test_parse(){
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_invalid_string_escape();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_miss_comma_or_square_bracket();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_number_too_big();
    test_parse_string();
    test_parse_array();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main(){
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n",test_pass,test_count,test_pass*100.0/test_count);
    return main_ret;
}


