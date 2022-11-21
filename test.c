//*******************************
// Created by Pickle on 2022/11/19.
//*******************************
#include <stdio.h>
#include "picklejson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

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

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual),expect,actual,"%d")
#define EXPECT_EQ_DOUBLE(expect,actual) EXPECT_EQ_BASE((expect) == (actual),expect,actual,"%.17g")

//TODO 2022.11.22.0:20

#define EXPECT_EQ_STRING(expect,actual,alength) \
            EXPECT_EQ_BASE(sizeof(expect) - 1) == alength &&
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

#define TEST_STRING(expect, json, len)\
    do{\
        pickle_value v;\
        pickle_init(&v);\
        EXPECT_EQ_INT(PICKLE_PARSE_OK,pickle_parse(&v, json));\
        EXPECT_EQ_INT((len),pickle_get_len(&v));\
        EXPECT_EQ_INT(PICKLE_STRING,pickle_get_type(&v));\
        EXPECT_EQ_INT(expect,pickle_get_string(&v));\
    }while(0)


static void test_parse_null(){
    pickle_value v;
    pickle_init(&v);
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "null"));
    EXPECT_EQ_INT(PICKLE_NULL, pickle_get_type(&v));
}

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

static void test_parse_number_too_big() {
#if 1
    TEST_ERROR(PICKLE_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PICKLE_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
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

#define TEST_STRING(expect, json) \
    do{                           \
        pickle_value v;           \
        pickle_init(&v);            \
        EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v,json)); \
        EXPECT_EQ_INT(PICKLE_STRING, pickle_get_type(&v));     \
        EXPECT_EQ_STRING();\
    }while(0)

static void test_parse_string(){

}




static void test_parse(){
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_number_too_big();
}

int main(){
    test_parse();
    printf("%d/%d (%3.2f%%) passed]\n",test_pass,test_count,test_pass*100.0/test_count);
    return main_ret;
}


