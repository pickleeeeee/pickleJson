//*******************************
// Created by Pickle on 2022/11/19.
//*******************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define TEST_ERROR(error, json)\
    do{\
        pickle_value v;\
        v.type = PICKLE_FALSE;\
        EXPECT_EQ_INT(error, pickle_parse(&v, json));\
        EXPECT_EQ_INT(PICKLE_NULL, pickle_get_type(&v));\
    }while(0)


static void test_parse_null(){
    pickle_value v;
    v.type = PICKLE_FALSE;
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
}

static void test_parse_root_not_singular(){
    TEST_ERROR(PICKLE_PARSE_ROOT_NOT_SINGULAR,"null x");
}

static void test_parse_true(){
    pickle_value v;
    v.type = PICKLE_FALSE;
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "true"));
    EXPECT_EQ_INT(PICKLE_TRUE, pickle_get_type(&v));
}

static void test_parse_false(){
    pickle_value v;
    v.type = PICKLE_FALSE;
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "false"));
    EXPECT_EQ_INT(PICKLE_FALSE, pickle_get_type(&v));
}

static void test_parse(){
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_true();
    test_parse_false();
}

int main(){
    test_parse();
    printf("%d/%d (%3.2f%%) passed]\n",test_pass,test_count,test_pass*100.0/test_count);
    return main_ret;
}


