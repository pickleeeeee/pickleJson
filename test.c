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

static void test_parse_null(){
    pickle_value v;
    v.type = PICKLE_FALSE;
    EXPECT_EQ_INT(PICKLE_PARSE_OK, pickle_parse(&v, "null"));
    EXPECT_EQ_INT(PICKLE_NULL, pickle_get_type(&v));
}

static void test_parse_expect_value(){
    pickle_value v;
    v.type = PICKLE_FALSE;

}
int main(){
    test_parse_null();
    return main_ret;
}


