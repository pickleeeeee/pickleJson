//*******************************
// Created by Pickle on 2022/11/19.
//*******************************
#include "picklejson.h"
#include <assert.h>
#include <stdlib.h>

#define EXPECT(c, ch)    do { assert(*c->json==(ch)); c->json++; } while(0)

typedef struct{
    const char* json;
}pickle_context;

/**
 * 删除字符串之前的空白
 * @param c
 */
static void pickle_parse_whitespace(pickle_context* c){
    const char* p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int pickle_parse_null(pickle_context* c, pickle_value* v){
    EXPECT(c,'n');
    if(c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return PICKLE_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = PICKLE_NULL;
    return PICKLE_PARSE_OK;
}

static int pickle_parse_true(pickle_context* c, pickle_value* v){
    EXPECT(c,'t');
    if(c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return PICKLE_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = PICKLE_TRUE;
    return PICKLE_PARSE_OK;
}

static int pickle_parse_false(pickle_context* c,pickle_value* v){
    EXPECT(c,'f');
    if(c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return PICKLE_PARSE_INVALID_VALUE;
    c->json +=4;
    v->type = PICKLE_FALSE;
    return PICKLE_PARSE_OK;
}

static int pickle_parse_value(pickle_context* c, pickle_value* v){
    switch (*c->json) {
        case 'n':   return pickle_parse_null(c,v);
        case 't':   return pickle_parse_true(c,v);
        case 'f':   return pickle_parse_false(c,v);
        case '\0':  return PICKLE_PARSE_EXPECT_VALUE;
        default:    return PICKLE_PARSE_INVALID_VALUE;
    }
}


int pickle_parse(pickle_value* v, const char* json){
    pickle_context c;
    int ret = 0;
    assert(v != NULL);
    c.json = json;
    //如果解析失败v将会返回NULL值，函数中先将v设置成了NULL值
    v->type = PICKLE_NULL;
    //移除字符前的空白
    pickle_parse_whitespace(&c);
    //只有再成功解析后才分析是不是又多个root
    if((ret = pickle_parse_value(&c,v)) == PICKLE_PARSE_OK){
        pickle_parse_whitespace(&c);
        if(c.json[0]!='\0')
            ret = PICKLE_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

pickle_type pickle_get_type(const pickle_value* v){
    assert(v != NULL);
    return v->type;
}