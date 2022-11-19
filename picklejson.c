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

static int pickle_parse_value(pickle_context* c, pickle_value* v){
    switch (*c->json) {
        case 'n':   return pickle_parse_null(c,v);
        case '\0':  return PICKLE_PARSE_EXPECT_VALUE;
        default:    return PICKLE_PARSE_INVALID_VALUE;
    }
}

int pickle_parse(pickle_value* v, const char* json){
    pickle_context c;
    assert(v != NULL);
    c.json = json;
    v->type = PICKLE_NULL;
    //将指针移动到第一个字母
    pickle_parse_whitespace(&c);

    return pickle_parse_value(&c,v);
}

pickle_type pickle_get_type(const pickle_value* v){
    assert(v != NULL);
    return v->type;
}